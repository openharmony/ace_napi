/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ark_native_engine.h"

#include "ark_native_deferred.h"
#include "ark_native_reference.h"

#include "native_engine/native_property.h"

#include "native_value/ark_native_array.h"
#include "native_value/ark_native_array_buffer.h"
#include "native_value/ark_native_boolean.h"
#include "native_value/ark_native_data_view.h"
#include "native_value/ark_native_external.h"
#include "native_value/ark_native_function.h"
#include "native_value/ark_native_number.h"
#include "native_value/ark_native_object.h"
#include "native_value/ark_native_string.h"
#include "native_value/ark_native_typed_array.h"
#include "native_value/ark_native_date.h"

#include "securec.h"
#include "utils/log.h"

using panda::BooleanRef;
using panda::ObjectRef;
using panda::StringRef;
using panda::FunctionRef;
using panda::PrimitiveRef;
using panda::JSValueRef;
using panda::ArrayBufferRef;
using panda::TypedArrayRef;
using panda::PromiseCapabilityRef;
using panda::NativePointerRef;
using panda::SymbolRef;
using panda::IntegerRef;
using panda::DateRef;
static constexpr auto PANDA_MAIN_FUNCTION = "_GLOBAL::func_main_0";

ArkNativeEngine::ArkNativeEngine(EcmaVM* vm, void* jsEngine) : NativeEngine(jsEngine), vm_(vm), topScope_(vm)
{
    Local<StringRef> requireName = StringRef::NewFromUtf8(vm, "requireNapi");
    Local<StringRef> requireInternalName = StringRef::NewFromUtf8(vm, "requireInternal");
    void* requireData = static_cast<void*>(this);

    Local<FunctionRef> requireNapi =
        FunctionRef::New(
            vm,
            [](EcmaVM *ecmaVm, Local<JSValueRef> thisObj,
               const Local<JSValueRef> argv[],  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
               int32_t length, void *data) -> Local<JSValueRef> {
                panda::EscapeLocalScope scope(ecmaVm);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                ArkNativeEngine* engine = static_cast<ArkNativeEngine*>(data);
                Local<StringRef> moduleName(argv[0]);
                bool isAppModule = false;
                int32_t lengthMax = 2;
                if (length == lengthMax) {
                    Local<BooleanRef> ret(argv[1]);
                    isAppModule = ret->Value();
                }
                NativeModule* module =
                    moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, isAppModule, false, true);
                Global<JSValueRef> exports(ecmaVm, JSValueRef::Undefined(ecmaVm));
                if (module != nullptr) {
                    auto it = engine->loadedModules_.find(module);
                    if (it != engine->loadedModules_.end()) {
                        return scope.Escape(it->second.ToLocal(ecmaVm));
                    }

                    if (module->jsCode != nullptr) {
                        HILOG_INFO("load js code");
                        NativeValue* script = engine->CreateString(module->jsCode, module->jsCodeLen);
                        char fileName[NAPI_PATH_MAX] = { 0 };
                        const char* name = module->name;
                        if (sprintf_s(fileName, sizeof(fileName), "lib%s.z.so/%s.js", name, name) == -1) {
                            HILOG_ERROR("sprintf_s file name failed");
                            return scope.Escape(exports.ToLocal(ecmaVm));
                        }
                        HILOG_DEBUG("load js code from %{public}s", fileName);
                        NativeValue* exportObject = engine->LoadModule(script, fileName);
                        if (exportObject == nullptr) {
                            HILOG_ERROR("load module failed");
                            return scope.Escape(exports.ToLocal(ecmaVm));
                        } else {
                            exports = *exportObject;
                            engine->loadedModules_[module] = Global<JSValueRef>(ecmaVm, exports.ToLocal(ecmaVm));
                        }
                    } else if (module->registerCallback != nullptr) {
                        NativeValue* exportObject = engine->CreateObject();
                        module->registerCallback(engine, exportObject);
                        exports = *exportObject;
                        engine->loadedModules_[module] = Global<JSValueRef>(ecmaVm, exports.ToLocal(ecmaVm));
                    } else {
                        HILOG_ERROR("init module failed");
                        return scope.Escape(exports.ToLocal(ecmaVm));
                    }
                }
                return scope.Escape(exports.ToLocal(ecmaVm));
            },
            requireData);

    Local<FunctionRef> requireInternal =
        FunctionRef::New(
            vm,
            [](EcmaVM *ecmaVm, Local<JSValueRef> thisObj,
               const Local<JSValueRef> argv[],  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
               int32_t length, void *data) -> Local<JSValueRef> {
                panda::EscapeLocalScope scope(ecmaVm);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                ArkNativeEngine* engine = static_cast<ArkNativeEngine*>(data);
                Local<StringRef> moduleName(argv[0]);
                NativeModule* module = moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, false);
                Global<JSValueRef> exports(ecmaVm, JSValueRef::Undefined(ecmaVm));
                if (module != nullptr) {
                    auto it = engine->loadedModules_.find(module);
                    if (it != engine->loadedModules_.end()) {
                        return scope.Escape(it->second.ToLocal(ecmaVm));
                    }

                    NativeValue* exportObject = engine->CreateObject();
                    if (exportObject != nullptr) {
                        module->registerCallback(engine, exportObject);
                        exports = *exportObject;
                        engine->loadedModules_[module] = Global<JSValueRef>(ecmaVm, exports.ToLocal(ecmaVm));
                    } else {
                        HILOG_ERROR("exportObject is nullptr");
                        return scope.Escape(exports.ToLocal(ecmaVm));
                    }
                }
                return scope.Escape(exports.ToLocal(ecmaVm));
            },
            requireData);

    Local<ObjectRef> global = panda::JSNApi::GetGlobalObject(vm);
    global->Set(vm, requireName, requireNapi);
    global->Set(vm, requireInternalName, requireInternal);
    // need to call init of base class.
    Init();
}

ArkNativeEngine::~ArkNativeEngine()
{
    // need to call deinit before base class.
    Deinit();

    // Free cached objects
    for (auto&& [module, exportObj] : loadedModules_) {
        exportObj.FreeGlobalHandleAddr();
    }
    // Free callbackRef
    if (promiseRejectCallbackRef_ != nullptr) {
        delete promiseRejectCallbackRef_;
    }
    if (checkCallbackRef_ != nullptr) {
        delete checkCallbackRef_;
    }
}

panda::Global<panda::ObjectRef> ArkNativeEngine::LoadModuleByName(
    const std::string& moduleName, bool isAppModule, const std::string& param,
    const std::string& instanceName, void* instance)
{
    Global<ObjectRef> exports(vm_, JSValueRef::Undefined(vm_));
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module = moduleManager->LoadNativeModule(moduleName.c_str(), nullptr, isAppModule);
    if (module != nullptr) {
        NativeValue* exportObject = new ArkNativeObject(this);
        ArkNativeObject* exportObj = reinterpret_cast<ArkNativeObject*>(exportObject);

        NativePropertyDescriptor paramProperty, instanceProperty;

        NativeValue* paramValue = new ArkNativeString(this, param.c_str(), param.size());
        paramProperty.utf8name = "param";
        paramProperty.value = paramValue;

        auto instanceValue = new ArkNativeObject(this);
        instanceValue->SetNativePointer(instance, nullptr, nullptr);
        instanceProperty.utf8name = instanceName.c_str();
        instanceProperty.value = instanceValue;

        exportObj->DefineProperty(paramProperty);
        exportObj->DefineProperty(instanceProperty);

        module->registerCallback(this, exportObject);
        exports = *exportObject;
    }
    return exports;
}

void ArkNativeEngine::Loop(LoopMode mode, bool needSync)
{
    LocalScope scope(vm_);
    NativeEngine::Loop(mode, needSync);
    panda::JSNApi::ExecutePendingJob(vm_);
}

NativeValue* ArkNativeEngine::GetGlobal()
{
    Local<ObjectRef> value = panda::JSNApi::GetGlobalObject(vm_);
    return ArkValueToNativeValue(this, value);
}

NativeValue* ArkNativeEngine::CreateNull()
{
    Local<PrimitiveRef> value = JSValueRef::Null(vm_);
    return new ArkNativeValue(this, value);
}

NativeValue* ArkNativeEngine::CreateUndefined()
{
    Local<PrimitiveRef> value = JSValueRef::Undefined(vm_);
    return new ArkNativeValue(this, value);
}

NativeValue* ArkNativeEngine::CreateBoolean(bool value)
{
    return new ArkNativeBoolean(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(int32_t value)
{
    return new ArkNativeNumber(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(uint32_t value)
{
    return new ArkNativeNumber(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(int64_t value)
{
    return new ArkNativeNumber(this, value);
}

NativeValue* ArkNativeEngine::CreateNumber(double value)
{
    return new ArkNativeNumber(this, value);
}

NativeValue* ArkNativeEngine::CreateBigInt(int64_t value)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::CreateBigInt(uint64_t value)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::CreateString(const char* value, size_t length)
{
    return new ArkNativeString(this, value, length);
}

NativeValue* ArkNativeEngine::CreateString16(const char16_t* value, size_t length)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::CreateSymbol(NativeValue* value)
{
    LocalScope scope(vm_);
    Global<StringRef> str = *value;
    Local<SymbolRef> symbol = SymbolRef::New(vm_, str.ToLocal(vm_));
    return new ArkNativeValue(this, symbol);
}

NativeValue* ArkNativeEngine::CreateExternal(void* value, NativeFinalize callback, void* hint)
{
    return new ArkNativeExternal(this, value, callback, hint);
}

NativeValue* ArkNativeEngine::CreateObject()
{
    return new ArkNativeObject(this);
}

NativeValue* ArkNativeEngine::CreateFunction(const char* name, size_t length, NativeCallback cb, void* value)
{
    return new ArkNativeFunction(this, name, length, cb, value);
}

NativeValue* ArkNativeEngine::CreateArray(size_t length)
{
    return new ArkNativeArray(this, length);
}

NativeValue* ArkNativeEngine::CreateArrayBuffer(void** value, size_t length)
{
    return new ArkNativeArrayBuffer(this, (uint8_t**)value, length);
}

NativeValue* ArkNativeEngine::CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint)
{
    return new ArkNativeArrayBuffer(this, (uint8_t*)value, length, cb, hint);
}

NativeValue* ArkNativeEngine::CreateTypedArray(NativeTypedArrayType type,
                                               NativeValue* value,
                                               size_t length,
                                               size_t offset)
{
    LocalScope scope(vm_);
    Global<ArrayBufferRef> globalBuffer = *value;
    Local<ArrayBufferRef> buffer = globalBuffer.ToLocal(vm_);
    Local<TypedArrayRef> typedArray(JSValueRef::Undefined(vm_));

    switch (type) {
        case NATIVE_INT8_ARRAY:
            typedArray = panda::Int8ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT8_ARRAY:
            typedArray = panda::Uint8ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT8_CLAMPED_ARRAY:
            typedArray = panda::Uint8ClampedArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_INT16_ARRAY:
            typedArray = panda::Int16ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT16_ARRAY:
            typedArray = panda::Uint16ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_INT32_ARRAY:
            typedArray = panda::Int32ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_UINT32_ARRAY:
            typedArray = panda::Uint32ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_FLOAT32_ARRAY:
            typedArray = panda::Float32ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_FLOAT64_ARRAY:
            typedArray = panda::Float64ArrayRef::New(vm_, buffer, offset, length);
            break;
        case NATIVE_BIGINT64_ARRAY:
            // not support yet
            return nullptr;
        case NATIVE_BIGUINT64_ARRAY:
            // not support yet
            return nullptr;
        default:
            return nullptr;
    }
    return new ArkNativeTypedArray(this, typedArray);
}

NativeValue* ArkNativeEngine::CreateDataView(NativeValue* value, size_t length, size_t offset)
{
    return new ArkNativeDataView(this, value, length, offset);
}

NativeValue* ArkNativeEngine::CreatePromise(NativeDeferred** deferred)
{
    LocalScope scope(vm_);
    Local<PromiseCapabilityRef> capability = PromiseCapabilityRef::New(vm_);
    *deferred = new ArkNativeDeferred(this, capability);

    return new ArkNativeValue(this, capability->GetPromise(vm_));
}

NativeValue* ArkNativeEngine::CreateError(NativeValue* code, NativeValue* message)
{
    LocalScope scope(vm_);
    Local<JSValueRef> errorVal = panda::Exception::Error(vm_, *message);
    if (code != nullptr) {
        Local<StringRef> codeKey = StringRef::NewFromUtf8(vm_, "code");
        Local<ObjectRef> errorObj(errorVal);
        errorObj->Set(vm_, codeKey, *code);
    }
    return ArkValueToNativeValue(this, errorVal);
}

NativeValue* ArkNativeEngine::CallFunction(NativeValue* thisVar,
                                           NativeValue* function,
                                           NativeValue* const* argv,
                                           size_t argc)
{
    if (function == nullptr) {
        return nullptr;
    }
    LocalScope scope(vm_);
    Global<JSValueRef> thisObj = (thisVar != nullptr) ? *thisVar : Global<JSValueRef>(vm_, JSValueRef::Undefined(vm_));
    Global<FunctionRef> funcObj = *function;

    std::vector<Local<JSValueRef>> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            Global<JSValueRef> arg = *argv[i];
            args.emplace_back(arg.ToLocal(vm_));
        } else {
            args.emplace_back(JSValueRef::Undefined(vm_));
        }
    }

    Local<JSValueRef> value = funcObj->Call(vm_, thisObj.ToLocal(vm_), args.data(), argc);
    Local<ObjectRef> excep = panda::JSNApi::GetUncaughtException(vm_);
    if (!excep.IsNull()) {
        Local<StringRef> exceptionMsg = excep->ToString(vm_);
        exceptionStr_ = exceptionMsg->ToString();
    }

    return ArkValueToNativeValue(this, value);
}

NativeValue* ArkNativeEngine::RunScript(NativeValue* script)
{
    // not support yet
    return nullptr;
}

void ArkNativeEngine::SetPackagePath(const std::string& packagePath)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager) {
        moduleManager->SetAppLibPath(packagePath.c_str());
    }
}

NativeValue* ArkNativeEngine::DefineClass(const char* name,
                                          NativeCallback callback,
                                          void* data,
                                          const NativePropertyDescriptor* properties,
                                          size_t length)
{
    LocalScope scope(vm_);
    auto classConstructor = new ArkNativeFunction(this, name, callback, data);
    auto classPrototype = classConstructor->GetFunctionPrototype();

    for (size_t i = 0; i < length; i++) {
        if (properties[i].attributes & NATIVE_STATIC) {
            classConstructor->DefineProperty(properties[i]);
        } else {
            if (classPrototype == nullptr) {
                HILOG_ERROR("ArkNativeEngine::Class's prototype is null");
                continue;
            }
            static_cast<ArkNativeObject*>(classPrototype)->DefineProperty(properties[i]);
        }
    }

    return classConstructor;
}

NativeValue* ArkNativeEngine::CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc)
{
    if (constructor == nullptr) {
        return nullptr;
    }
    LocalScope scope(vm_);
    Global<FunctionRef> value = *constructor;

    std::vector<Local<JSValueRef>> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        if (argv[i] != nullptr) {
            Global<JSValueRef> arg = *argv[i];
            args.emplace_back(arg.ToLocal(vm_));
        } else {
            args.emplace_back(JSValueRef::Undefined(vm_));
        }
    }

    Local<JSValueRef> instance = value->Constructor(vm_, args.data(), argc);

    return ArkValueToNativeValue(this, instance);
}

NativeReference* ArkNativeEngine::CreateReference(NativeValue* value, uint32_t initialRefcount,
    NativeFinalize callback, void* data, void* hint)
{
    return new ArkNativeReference(this, value, initialRefcount);
}

bool ArkNativeEngine::Throw(NativeValue* error)
{
    LocalScope scope(vm_);
    Global<JSValueRef> errorVal = *error;
    panda::JSNApi::ThrowException(vm_, errorVal.ToLocal(vm_));
    lastException_ = error;
    return true;
}

bool ArkNativeEngine::Throw(NativeErrorType type, const char* code, const char* message)
{
    LocalScope scope(vm_);
    Local<JSValueRef> error(JSValueRef::Undefined(vm_));
    switch (type) {
        case NATIVE_COMMON_ERROR:
            error = panda::Exception::Error(vm_, StringRef::NewFromUtf8(vm_, message));
            break;
        case NATIVE_TYPE_ERROR:
            error = panda::Exception::TypeError(vm_, StringRef::NewFromUtf8(vm_, message));
            break;
        case NATIVE_RANGE_ERROR:
            error = panda::Exception::RangeError(vm_, StringRef::NewFromUtf8(vm_, message));
            break;
        default:
            return false;
    }
    if (code != nullptr) {
        Local<JSValueRef> codeKey = StringRef::NewFromUtf8(vm_, "code");
        Local<JSValueRef> codeValue = StringRef::NewFromUtf8(vm_, code);
        Local<ObjectRef> errorObj(error);
        errorObj->Set(vm_, codeKey, codeValue);
    }

    panda::JSNApi::ThrowException(vm_, error);
    lastException_ = ArkValueToNativeValue(this, error);
    return true;
}

void* ArkNativeEngine::CreateRuntime()
{
    panda::RuntimeOption option;
    option.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
    const int64_t poolSize = 0x1000000;
    option.SetGcPoolSize(poolSize);
    option.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
    if (vm == nullptr) {
        return nullptr;
    }

    ArkNativeEngine* arkEngine = new ArkNativeEngine(vm, this);
    return reinterpret_cast<void*>(arkEngine);
}

NativeValue* ArkNativeEngine::Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer)
{
    const panda::ecmascript::EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(context)->GetEcmaVm();
    Local<JSValueRef> arkValue = *value;
    Local<JSValueRef> arkTransfer = *transfer;
    void* result = panda::JSNApi::SerializeValue(vm, arkValue, arkTransfer);
    return reinterpret_cast<NativeValue*>(result);
}

NativeValue* ArkNativeEngine::Deserialize(NativeEngine* context, NativeValue* recorder)
{
    const panda::ecmascript::EcmaVM* vm = reinterpret_cast<ArkNativeEngine*>(context)->GetEcmaVm();
    Local<JSValueRef> result = panda::JSNApi::DeserializeValue(vm, recorder);
    return ArkValueToNativeValue(this, result);
}

ExceptionInfo* ArkNativeEngine::GetExceptionForWorker() const
{
    if (exceptionStr_.empty()) {
        HILOG_ERROR("worker:: exception is null");
        return nullptr;
    }

    ExceptionInfo* exceptionInfo = new ExceptionInfo();
    int msgLength = exceptionStr_.length();
    char* exceptionMessage = new char[msgLength + 1] { 0 };
    if (memcpy_s(exceptionMessage, msgLength + 1, exceptionStr_.c_str(), msgLength) != EOK) {
        HILOG_ERROR("worker:: memcpy_s error");
        delete exceptionInfo;
        delete[] exceptionMessage;
        return nullptr;
    }
    exceptionInfo->message_ = exceptionMessage;
    // need add colno, lineno when ark exception support
    return exceptionInfo;
}

void ArkNativeEngine::DeleteSerializationData(NativeValue* value) const
{
    void* data = reinterpret_cast<void*>(value);
    panda::JSNApi::DeleteSerializationData(data);
}

void ArkNativeEngine::StartCpuProfiler()
{
    panda::JSNApi::StartCpuProfiler(vm_);
}

void ArkNativeEngine::StopCpuProfiler()
{
    panda::JSNApi::StopCpuProfiler();
}

NativeValue* ArkNativeEngine::RunBufferScript(std::vector<uint8_t>& buffer)
{
    Local<StringRef> entryPoint = StringRef::NewFromUtf8(vm_, PANDA_MAIN_FUNCTION);
    panda::JSExecutionScope executionScope(vm_);
    bool ret = panda::JSNApi::Execute(vm_, buffer.data(), buffer.size(), entryPoint);

    Local<ObjectRef> excep = panda::JSNApi::GetUncaughtException(vm_);
    if (!excep.IsNull()) {
        Local<StringRef> exceptionMsg = excep->ToString(vm_);
        exceptionStr_ = exceptionMsg->ToString();
    }

    if (!ret) {
        return nullptr;
    }
    return CreateUndefined();
}

NativeValue* ArkNativeEngine::LoadModule(NativeValue* str, const std::string& fileName)
{
    HILOG_DEBUG("ArkNativeEngine::LoadModule start");
    if (str == nullptr || fileName.empty()) {
        HILOG_ERROR("fileName is nullptr or source code is nullptr");
        return nullptr;
    }

    Local<StringRef> source = *str;
    int32_t len = source->Length();
    char* buffer = new char[len];
    source->WriteUtf8(buffer, len);
    HILOG_DEBUG("Ark::LoadModule buffer = %{public}s", buffer);

    bool res = JSNApi::ExecuteModuleFromBuffer(vm_, buffer, len, fileName);
    if (!res) {
        HILOG_ERROR("Execute module failed");
        delete[] buffer;
        return nullptr;
    }

    Local<ObjectRef> exportObj = JSNApi::GetExportObject(vm_, fileName, "default");
    if (exportObj->IsNull()) {
        HILOG_ERROR("Get export object failed");
        delete[] buffer;
        return nullptr;
    }

    delete[] buffer;
    HILOG_DEBUG("ArkNativeEngine::LoadModule end");
    return ArkValueToNativeValue(this, exportObj);
}

NativeValue* ArkNativeEngine::ArkValueToNativeValue(ArkNativeEngine* engine, Local<JSValueRef> value)
{
    NativeValue* result = nullptr;
    if (value->IsNull() || value->IsUndefined() || value->IsSymbol() || value->IsPromise()) {
        result = new ArkNativeValue(engine, value);
    } else if (value->IsNumber()) {
        result = new ArkNativeNumber(engine, value);
    } else if (value->IsString()) {
        result = new ArkNativeString(engine, value);
    } else if (value->IsArray(engine->GetEcmaVm())) {
        result = new ArkNativeArray(engine, value);
    } else if (value->IsFunction()) {
        result = new ArkNativeFunction(engine, value);
    } else if (value->IsArrayBuffer()) {
        result = new ArkNativeArrayBuffer(engine, value);
    } else if (value->IsDataView()) {
        result = new ArkNativeDataView(engine, value);
    } else if (value->IsTypedArray()) {
        result = new ArkNativeTypedArray(engine, value);
    } else if (value->IsNativePointer()) {
        result = new ArkNativeExternal(engine, value);
    } else if (value->IsDate()) {
        result = new ArkNativeDate(engine, value);
    } else if (value->IsObject()) {
        result = new ArkNativeObject(engine, value);
    } else if (value->IsBoolean()) {
        result = new ArkNativeBoolean(engine, value);
    }
    return result;
}

NativeValue* ArkNativeEngine::ValueToNativeValue(JSValueWrapper& value)
{
    Global<JSValueRef> arkValue = value;
    return ArkValueToNativeValue(this, arkValue.ToLocal(vm_));
}

bool ArkNativeEngine::ExecuteJsBin(const std::string& fileName)
{
    panda::JSExecutionScope executionScope(vm_);
    LocalScope scope(vm_);
    Local<StringRef> file = StringRef::NewFromUtf8(vm_, fileName.c_str());
    Local<StringRef> entryPoint = StringRef::NewFromUtf8(vm_, PANDA_MAIN_FUNCTION);
    bool ret = JSNApi::Execute(vm_, file, entryPoint);
    return ret;
}

NativeValue* ArkNativeEngine::CreateBuffer(void** value, size_t length)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::CreateBufferCopy(void** value, size_t length, const void* data)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::CreateDate(double value)
{
    return ArkValueToNativeValue(this, DateRef::New(vm_, value));
}

NativeValue* ArkNativeEngine::CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words)
{
    return nullptr;
}

bool ArkNativeEngine::TriggerFatalException(NativeValue* error)
{
    return true;
}

bool ArkNativeEngine::AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue)
{
    return true;
}

void ArkNativeEngine::SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef)
{
    if (rejectCallbackRef == nullptr || checkCallbackRef == nullptr) {
        HILOG_ERROR("rejectCallbackRef or checkCallbackRef is nullptr");
        return;
    }
    promiseRejectCallbackRef_ = rejectCallbackRef;
    checkCallbackRef_ = checkCallbackRef;
    JSNApi::SetHostPromiseRejectionTracker(vm_, reinterpret_cast<void*>(PromiseRejectCallback),
                                           reinterpret_cast<void*>(this));
}

// values = {type, promise, reason}
void ArkNativeEngine::PromiseRejectCallback(void* info)
{
    panda::PromiseRejectInfo* promiseRejectInfo = reinterpret_cast<panda::PromiseRejectInfo*>(info);
    ArkNativeEngine* env = reinterpret_cast<ArkNativeEngine*>(promiseRejectInfo->GetData());
    Local<JSValueRef> promise = promiseRejectInfo->GetPromise();
    Local<JSValueRef> reason = promiseRejectInfo->GetReason();
    panda::PromiseRejectInfo::PROMISE_REJECTION_EVENT operation = promiseRejectInfo->GetOperation();

    if (env == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return;
    }

    if (env->promiseRejectCallbackRef_ == nullptr || env->checkCallbackRef_ == nullptr) {
        HILOG_ERROR("promiseRejectCallbackRef or checkCallbackRef is nullptr");
        return;
    }

    const panda::ecmascript::EcmaVM* vm = env->GetEcmaVm();
    Local<JSValueRef> type(IntegerRef::New(vm, static_cast<int32_t>(operation)));

    Local<JSValueRef> args[] = {type, promise, reason};
    Global<FunctionRef> promiseRejectCallback = *(env->promiseRejectCallbackRef_->Get());
    if (!promiseRejectCallback.IsEmpty()) {
        Global<JSValueRef> thisObj = Global<JSValueRef>(vm, JSValueRef::Undefined(vm));
        promiseRejectCallback->Call(vm, thisObj.ToLocal(vm), args, 3); // 3 args size
    }

    if (operation == panda::PromiseRejectInfo::PROMISE_REJECTION_EVENT::REJECT) {
        Global<JSValueRef> checkCallback = *(env->checkCallbackRef_->Get());
        if (!checkCallback.IsEmpty()) {
            JSNApi::SetHostEnqueueJob(vm, checkCallback.ToLocal(vm));
        }
    }
}