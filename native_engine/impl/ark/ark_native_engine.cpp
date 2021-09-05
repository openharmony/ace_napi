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

#include "utils/log.h"

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
ArkNativeEngine::ArkNativeEngine(EcmaVM* vm, void* jsEngine) : NativeEngine(jsEngine), vm_(vm), topScope_(vm)
{
    Local<StringRef> requireName = StringRef::NewFromUtf8(vm, "requireNapi");
    Local<StringRef> requireInternalName = StringRef::NewFromUtf8(vm, "requireInternal");
    void* requireData = static_cast<void*>(this);

    Local<FunctionRef> requireNapi =
        FunctionRef::New(
            vm,
            [](EcmaVM *ecmaVm, Local<JSValueRef> thisObj, const Local<JSValueRef> argv[],  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
               int32_t length, void *data) -> Local<JSValueRef> {
                panda::EscapeLocalScope scope(ecmaVm);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                ArkNativeEngine* engine = static_cast<ArkNativeEngine*>(data);
                Local<StringRef> moduleName(argv[0]);
                NativeModule* module = moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, false);
                Global<ObjectRef> exports;
                if (module != nullptr) {
                    if (module->jsCode != nullptr) {
                        HILOG_INFO("load js code");
                        NativeValue* script = engine->CreateString(module->jsCode, strlen(module->jsCode));
                        NativeValue* exportObject = engine->LoadModule(script, "testjsnapi.js");
                        if (exportObject == nullptr) {
                            HILOG_ERROR("load module failed");
                        } else {
                            exports = *exportObject;
                        }
                    } else if (module->registerCallback != nullptr) {
                        NativeValue* exportObject = new ArkNativeObject(engine);
                        module->registerCallback(engine, exportObject);
                        exports = *exportObject;
                    } else {
                        HILOG_ERROR("init module failed");
                    }
                }
                return scope.Escape(exports.ToLocal(ecmaVm));
            },
            requireData);

    Local<FunctionRef> requireInternal =
        FunctionRef::New(
            vm,
            [](EcmaVM *ecmaVm, Local<JSValueRef> thisObj, const Local<JSValueRef> argv[],  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
               int32_t length, void *data) -> Local<JSValueRef> {
                panda::EscapeLocalScope scope(ecmaVm);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                ArkNativeEngine* engine = static_cast<ArkNativeEngine*>(data);
                Local<StringRef> moduleName(argv[0]);
                NativeModule* module = moduleManager->LoadNativeModule(moduleName->ToString().c_str(), nullptr, false);
                Global<ObjectRef> exports;
                if (module != nullptr) {
                    NativeValue* exportObject = new ArkNativeObject(engine);
                    module->registerCallback(engine, exportObject);
                    exports = *exportObject;
                }
                return scope.Escape(exports.ToLocal(ecmaVm));
            },
            requireData);

    Local<ObjectRef> global = panda::JSNApi::GetGlobalObject(vm);
    global->Set(vm, requireName, requireNapi);
    global->Set(vm, requireInternalName, requireInternal);
}

ArkNativeEngine::~ArkNativeEngine() {}

void ArkNativeEngine::Loop(LoopMode mode, bool needSync)
{
    LocalScope scope(vm_);
    NativeEngine::Loop(mode);
    //panda::JSNApi::ExecutePendingJob(vm_);
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

NativeValue* ArkNativeEngine::CreateString(const char* value, size_t length)
{
    return new ArkNativeString(this, value, length);
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
    Local<TypedArrayRef> typedArray;

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
    Local<JSValueRef>* args = nullptr;

    if (argc > 0) {
        args = new Local<JSValueRef>[argc];
        for (size_t i = 0; i < argc; i++) {
            if (argv[i] != nullptr) {
                Global<JSValueRef> arg = *argv[i];
                args[i] = arg.ToLocal(vm_);
            } else {
                args[i] = JSValueRef::Undefined(vm_);
            }
        }
    }

    Local<JSValueRef> value = funcObj->Call(vm_, thisObj.ToLocal(vm_), args, argc);

    if (args != nullptr) {
        delete []args;
    }

    return ArkValueToNativeValue(this, value);
}

NativeValue* ArkNativeEngine::RunScript(NativeValue* script)
{
    // not support yet
    return nullptr;
}

NativeValue* ArkNativeEngine::DefineClass(const char* name,
                                         NativeCallback callback,
                                         void* data,
                                         const NativePropertyDescriptor* properties,
                                         size_t length)
{
    LocalScope scope(vm_);
    auto classConstructor = new ArkNativeFunction(this, name, callback, data);

    for (size_t i = 0; i < length; i++) {
        if (properties[i].attributes & NATIVE_STATIC) {
            classConstructor->DefineProperty(properties[i]);
        } else {
            auto classPrototype = classConstructor->GetFunctionPrototype();
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
    Local<JSValueRef>* args = nullptr;
    if (argc > 0) {
        args = new Local<JSValueRef>[argc];
        for (size_t i = 0; i < argc; i++) {
            Global<JSValueRef> arg = *argv[i];
            args[i] = arg.ToLocal(vm_);
        }
    }
    Local<JSValueRef> instance = value->Constructor(vm_, args, argc);

    if (args != nullptr) {
        delete[] args;
    }
    return ArkValueToNativeValue(this, instance);
}

NativeReference* ArkNativeEngine::CreateReference(NativeValue* value, uint32_t initialRefcount)
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
    Local<JSValueRef> error;
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
    return nullptr;
}

NativeValue* ArkNativeEngine::Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer)
{
    return nullptr;
}

NativeValue* ArkNativeEngine::Deserialize(NativeEngine* context, NativeValue* recorder)
{
    return nullptr;
}

void ArkNativeEngine::DeleteSerializationData(NativeValue* value) const
{
}

ExceptionInfo* ArkNativeEngine::GetExceptionForWorker() const
{
    return nullptr;
}

NativeValue* ArkNativeEngine::LoadModule(NativeValue* str, const std::string& fileName)
{
    return nullptr;
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
    } else if (value->IsNativeObject() || value->IsNativePointer()) {
        result = new ArkNativeExternal(engine, value);
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
