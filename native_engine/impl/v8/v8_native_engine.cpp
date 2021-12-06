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

#include "v8_native_engine.h"

#include <js_native_api.h>

#include "native_engine/native_property.h"
#include "native_value/v8_native_array.h"
#include "native_value/v8_native_array_buffer.h"
#include "native_value/v8_native_boolean.h"
#include "native_value/v8_native_data_view.h"
#include "native_value/v8_native_external.h"
#include "native_value/v8_native_function.h"
#include "native_value/v8_native_number.h"
#include "native_value/v8_native_object.h"
#include "native_value/v8_native_string.h"
#include "native_value/v8_native_typed_array.h"
#include "securec.h"
#include "utils/log.h"
#include "v8_native_deferred.h"
#include "v8_native_reference.h"


static thread_local V8NativeEngine* g_env = nullptr;

V8NativeEngine::V8NativeEngine(v8::Platform* platform, v8::Isolate* isolate,
    v8::Persistent<v8::Context>& context, void* jsEngine)
    : NativeEngine(jsEngine),
      platform_(platform),
      isolate_(isolate),
      context_(isolate, context),
      isolateScope_(isolate),
      handleScope_(isolate_),
      contextScope_(context.Get(isolate_)),
      tryCatch_(isolate_)
{
    g_env = this;
    v8::Local<v8::String> requireNapiName = v8::String::NewFromUtf8(isolate_, "requireNapi").ToLocalChecked();
    v8::Local<v8::String> requireInternalName = v8::String::NewFromUtf8(isolate_, "requireInternal").ToLocalChecked();

    v8::Local<v8::Value> requireData = v8::External::New(isolate_, (void*)this).As<v8::Value>();

    v8::Local<v8::Function> requireNapi =
        v8::Function::New(
            context_.Get(isolate_),
            [](const v8::FunctionCallbackInfo<v8::Value>& info) {
                v8::Isolate::Scope isolateScope(info.GetIsolate());
                v8::HandleScope handleScope(info.GetIsolate());

                V8NativeEngine* engine = (V8NativeEngine*)info.Data().As<v8::External>()->Value();
                if (engine == nullptr) {
                    return;
                }
                v8::String::Utf8Value moduleName(info.GetIsolate(), info[0]);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                if (moduleManager == nullptr) {
                    return;
                }

                bool isAppModule = false;
                if (info.Length() == 2) {
                    isAppModule = info[1]->ToBoolean(info.GetIsolate())->Value();
                }
                NativeModule* module = moduleManager->LoadNativeModule(*moduleName, nullptr, isAppModule);

                if (module == nullptr) {
                    return;
                }

                if (module->jsCode != nullptr) {
                    HILOG_INFO("load js code");
                    NativeValue* script = engine->CreateString(module->jsCode, strlen(module->jsCode));
                    NativeValue* exportObject = engine->LoadModule(script, "testjsnapi.js");
                    if (exportObject == nullptr) {
                        HILOG_ERROR("load module failed");
                        return;
                    }
                    v8::Local<v8::Object> exports = *exportObject;
                    info.GetReturnValue().Set(exports);
                    HILOG_ERROR("load module succ");
                } else if (module->registerCallback != nullptr) {
                    HILOG_INFO("load napi module");
                    NativeValue* exportObject = new V8NativeObject(engine);
                    if (exportObject == nullptr) {
                        return;
                    }
                    module->registerCallback(engine, exportObject);
                    v8::Local<v8::Object> exports = *exportObject;
                    info.GetReturnValue().Set(exports);
                }
            },
            requireData, 1)
            .ToLocalChecked();

    v8::Local<v8::Function> requireInternal =
        v8::Function::New(
            context_.Get(isolate_),
            [](const v8::FunctionCallbackInfo<v8::Value>& info) {
                v8::Isolate::Scope isolateScope(info.GetIsolate());
                v8::HandleScope handleScope(info.GetIsolate());

                V8NativeEngine* engine = (V8NativeEngine*)info.Data().As<v8::External>()->Value();
                if (engine == nullptr) {
                    return;
                }
                v8::String::Utf8Value moduleName(info.GetIsolate(), info[0]);
                NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
                if (moduleManager == nullptr) {
                    return;
                }
                NativeModule* module = moduleManager->LoadNativeModule(*moduleName, nullptr, false, true);
                if (module == nullptr) {
                    return;
                }
                NativeValue* exportObject = new V8NativeObject(engine);
                if (exportObject == nullptr) {
                    return;
                }
                module->registerCallback(engine, exportObject);
                v8::Local<v8::Object> exports = *exportObject;
                info.GetReturnValue().Set(exports);
            },
            requireData, 1)
            .ToLocalChecked();

    v8::Local<v8::Object> global = context_.Get(isolate_)->Global();

    global->Set(context_.Get(isolate_), requireNapiName, requireNapi).FromJust();
    global->Set(context_.Get(isolate_), requireInternalName, requireInternal).FromJust();
    // need to call init of base class.
    Init();
}

V8NativeEngine::~V8NativeEngine()
{
    if (promiseRejectCallbackRef_ != nullptr) {
        delete promiseRejectCallbackRef_;
    }
    if (checkCallbackRef_ != nullptr) {
        delete checkCallbackRef_;
    }
    // need to call deinit before base class.
    Deinit();
}

v8::Local<v8::Object> V8NativeEngine::GetModuleFromName(
    const std::string& moduleName, bool isAppModule, const std::string& id, const std::string& param,
    const std::string& instanceName, void** instance)
{
    v8::Isolate* isolate = this->GetContext()->GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> exports;
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module = moduleManager->LoadNativeModule(moduleName.c_str(), nullptr, isAppModule);
    if (module != nullptr) {
        NativeValue* idValue = new V8NativeString(this, id.c_str(), id.size());
        NativeValue* paramValue = new V8NativeString(this, param.c_str(), param.size());
        NativeValue* exportObject = new V8NativeObject(this);

        NativePropertyDescriptor idProperty, paramProperty;
        idProperty.utf8name = "id";
        idProperty.value = idValue;
        paramProperty.utf8name = "param";
        paramProperty.value = paramValue;
        V8NativeObject* exportObj = reinterpret_cast<V8NativeObject*>(exportObject);
        exportObj->DefineProperty(idProperty);
        exportObj->DefineProperty(paramProperty);
        module->registerCallback(this, exportObject);

        napi_value nExport = reinterpret_cast<napi_value>(exportObject);
        napi_value exportInstance = nullptr;
        napi_status status = napi_get_named_property(
            reinterpret_cast<napi_env>(this), nExport, instanceName.c_str(), &exportInstance);
        if (status != napi_ok) {
            HILOG_ERROR("GetModuleFromName napi_get_named_property status != napi_ok");
        }

        status = napi_unwrap(reinterpret_cast<napi_env>(this), exportInstance, reinterpret_cast<void**>(instance));
        if (status != napi_ok) {
            HILOG_ERROR("GetModuleFromName napi_unwrap status != napi_ok");
        }

        exports = *exportObject;
    }
    return exports;
}

v8::Local<v8::Object> V8NativeEngine::LoadModuleByName(
    const std::string& moduleName, bool isAppModule, const std::string& param,
    const std::string& instanceName, void* instance)
{
    v8::Isolate* isolate = this->GetContext()->GetIsolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Object> exports;
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule* module = moduleManager->LoadNativeModule(moduleName.c_str(), nullptr, isAppModule);
    if (module != nullptr) {
        NativeValue* exportObject = new V8NativeObject(this);
        V8NativeObject* exportObj = reinterpret_cast<V8NativeObject*>(exportObject);

        NativePropertyDescriptor paramProperty, instanceProperty;

        NativeValue* paramValue = new V8NativeString(this, param.c_str(), param.size());
        paramProperty.utf8name = "param";
        paramProperty.value = paramValue;

        V8NativeObject* instanceValue = new V8NativeObject(this);
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

v8::Isolate* V8NativeEngine::GetIsolate()
{
    return isolate_;
}

v8::Local<v8::Context> V8NativeEngine::GetContext()
{
    return *reinterpret_cast<v8::Local<v8::Context>*>(const_cast<v8::Global<v8::Context>*>(&context_));
}

void V8NativeEngine::Loop(LoopMode mode, bool needSync)
{
    NativeEngine::Loop(mode, needSync);
    v8::platform::PumpMessageLoop(platform_, isolate_);
}

NativeValue* V8NativeEngine::GetGlobal()
{
    v8::Local<v8::Object> value = context_.Get(isolate_)->Global();
    return V8ValueToNativeValue(this, value);
}

NativeValue* V8NativeEngine::CreateNull()
{
    v8::Local<v8::Primitive> value = v8::Null(isolate_);
    return new V8NativeValue(this, value);
}

NativeValue* V8NativeEngine::CreateUndefined()
{
    v8::Local<v8::Primitive> value = v8::Undefined(isolate_);
    return new V8NativeValue(this, value);
}

NativeValue* V8NativeEngine::CreateBoolean(bool value)
{
    return new V8NativeBoolean(this, value);
}

NativeValue* V8NativeEngine::CreateNumber(int32_t value)
{
    return new V8NativeNumber(this, value);
}

NativeValue* V8NativeEngine::CreateNumber(uint32_t value)
{
    return new V8NativeNumber(this, value);
}

NativeValue* V8NativeEngine::CreateNumber(int64_t value)
{
    return new V8NativeNumber(this, value);
}

NativeValue* V8NativeEngine::CreateNumber(double value)
{
    return new V8NativeNumber(this, value);
}

NativeValue* V8NativeEngine::CreateString(const char* value, size_t length)
{
    return new V8NativeString(this, value, length);
}

NativeValue* V8NativeEngine::CreateSymbol(NativeValue* value)
{
    return new V8NativeValue(this, v8::Symbol::New(isolate_, *value));
}

NativeValue* V8NativeEngine::CreateExternal(void* value, NativeFinalize callback, void* hint)
{
    return new V8NativeExternal(this, value, callback, hint);
}

NativeValue* V8NativeEngine::CreateObject()
{
    return new V8NativeObject(this);
}

NativeValue* V8NativeEngine::CreateFunction(const char* name, size_t length, NativeCallback cb, void* value)
{
    return new V8NativeFunction(this, name, length, cb, value);
}

NativeValue* V8NativeEngine::CreateArray(size_t length)
{
    return new V8NativeArray(this, length);
}

NativeValue* V8NativeEngine::CreateArrayBuffer(void** value, size_t length)
{
    return new V8NativeArrayBuffer(this, (uint8_t**)value, length);
}

NativeValue* V8NativeEngine::CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint)
{
    return new V8NativeArrayBuffer(this, (uint8_t*)value, length, cb, hint);
}

NativeValue* V8NativeEngine::CreateTypedArray(NativeTypedArrayType type,
                                              NativeValue* value,
                                              size_t length,
                                              size_t offset)
{
    v8::Local<v8::ArrayBuffer> buffer = *value;
    v8::Local<v8::TypedArray> typedArray;

    switch (type) {
        case NATIVE_INT8_ARRAY:
            typedArray = v8::Int8Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT8_ARRAY:
            typedArray = v8::Uint8Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT8_CLAMPED_ARRAY:
            typedArray = v8::Uint8ClampedArray::New(buffer, offset, length);
            break;
        case NATIVE_INT16_ARRAY:
            typedArray = v8::Int16Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT16_ARRAY:
            typedArray = v8::Uint16Array::New(buffer, offset, length);
            break;
        case NATIVE_INT32_ARRAY:
            typedArray = v8::Int32Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT32_ARRAY:
            typedArray = v8::Uint32Array::New(buffer, offset, length);
            break;
        case NATIVE_FLOAT32_ARRAY:
            typedArray = v8::Float32Array::New(buffer, offset, length);
            break;
        case NATIVE_FLOAT64_ARRAY:
            typedArray = v8::Float64Array::New(buffer, offset, length);
            break;
        case NATIVE_BIGINT64_ARRAY:
            typedArray = v8::BigInt64Array::New(buffer, offset, length);
            break;
        case NATIVE_BIGUINT64_ARRAY:
            typedArray = v8::BigUint64Array::New(buffer, offset, length);
            break;
        default:
            return nullptr;
    }
    return new V8NativeTypedArray(this, typedArray);
}

struct CompleteWrapData {
    NativeAsyncExecuteCallback execute_ = nullptr;
    NativeAsyncCompleteCallback complete_ = nullptr;
    void* data_ = nullptr;
    v8::Isolate* isolate_ = nullptr;
};

void V8NativeEngine::ExecuteWrap(NativeEngine* engine, void* data)
{
    CompleteWrapData* wrapData = (CompleteWrapData*)data;
    wrapData->execute_(engine, wrapData->data_);
}

void V8NativeEngine::CompleteWrap(NativeEngine* engine, int status, void* data)
{
    CompleteWrapData* wrapData = (CompleteWrapData*)data;
    v8::Isolate::Scope isolateScope(wrapData->isolate_);
    v8::HandleScope handleScope(wrapData->isolate_);
    wrapData->complete_(engine, status, wrapData->data_);
    delete wrapData;
}

NativeAsyncWork* V8NativeEngine::CreateAsyncWork(NativeValue* asyncResource, NativeValue* asyncResourceName,
    NativeAsyncExecuteCallback execute, NativeAsyncCompleteCallback complete, void* data)
{
    CompleteWrapData* wrapData = new CompleteWrapData();
    if (wrapData == nullptr) {
        HILOG_ERROR("create wrap data failed");
        return nullptr;
    }
    wrapData->execute_ = execute;
    wrapData->complete_ = complete;
    wrapData->data_ = data;
    wrapData->isolate_ = GetIsolate();

    return NativeEngine::CreateAsyncWork(asyncResource, asyncResourceName, ExecuteWrap, CompleteWrap, (void*)wrapData);
}

NativeValue* V8NativeEngine::CreateDataView(NativeValue* value, size_t length, size_t offset)
{
    return new V8NativeDataView(this, value, length, offset);
}

NativeValue* V8NativeEngine::CreatePromise(NativeDeferred** deferred)
{
    auto v8Resolver = v8::Promise::Resolver::New(context_.Get(isolate_)).ToLocalChecked();

    *deferred = new V8NativeDeferred(this, v8Resolver);

    return new V8NativeValue(this, v8Resolver->GetPromise());
}

NativeValue* V8NativeEngine::CreateError(NativeValue* code, NativeValue* message)
{
    v8::Local<v8::Value> errorObj = v8::Exception::Error(*message);
    if (code) {
        v8::Local<v8::Value> codeKey = v8::String::NewFromUtf8(isolate_, "code").ToLocalChecked();
        errorObj.As<v8::Object>()->Set(context_.Get(isolate_), codeKey, *code).FromJust();
    }
    return V8ValueToNativeValue(this, errorObj);
}

NativeValue* V8NativeEngine::CallFunction(NativeValue* thisVar,
                                          NativeValue* function,
                                          NativeValue* const* argv,
                                          size_t argc)
{
    if (function == nullptr) {
        return nullptr;
    }
    v8::Local<v8::Value> v8recv = (thisVar != nullptr) ? *thisVar : v8::Undefined(isolate_);
    v8::Local<v8::Function> v8func = *function;
    v8::Local<v8::Value>* args = nullptr;
    v8::Local<v8::Context> context = context_.Get(isolate_);
    if (argc > 0) {
        args = new v8::Local<v8::Value>[argc];
        for (size_t i = 0; i < argc && args != nullptr; i++) {
            if (argv[i] != nullptr) {
                args[i] = *argv[i];
            } else {
                args[i] = v8::Undefined(isolate_);
            }
        }
    }
    v8::MaybeLocal<v8::Value> maybeValue = v8func->Call(context, v8recv, argc, args);
    if (args != nullptr) {
        delete []args;
    }
    v8::Local<v8::Value> result;
    if (!maybeValue.ToLocal(&result)) {
        return nullptr;
    }
    return V8ValueToNativeValue(this, result);
}

NativeValue* V8NativeEngine::RunScript(NativeValue* script)
{
    v8::Local<v8::Value> v8Script = *script;
    auto maybeScript = v8::Script::Compile(context_.Get(isolate_), v8Script.As<v8::String>());
    auto localScript = maybeScript.ToLocalChecked();
    auto scriptResult = localScript->Run(context_.Get(isolate_));

    v8::Local<v8::Value> result;
    if (!scriptResult.ToLocal(&result)) {
        return nullptr;
    }

    return V8ValueToNativeValue(this, result);
}

NativeValue* V8NativeEngine::RunBufferScript(std::vector<uint8_t>& buffer)
{
    NativeValue* script = CreateString(reinterpret_cast<char*>(buffer.data()), buffer.size());
    return RunScript(script);
}

namespace {
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* path)
{
    std::ifstream file(path);
    if (file.fail()) {
        file.close();
        return v8::MaybeLocal<v8::String>();
    }

    std::string fileContent;
    fileContent.clear();
    file.seekg(0, std::ios::end);
    fileContent.reserve(static_cast<std::string::size_type>(file.tellg()));
    file.seekg(0, std::ios::beg);
    fileContent.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    file.close();

    v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(isolate, fileContent.c_str(),
                                                                v8::NewStringType::kNormal, fileContent.size());
    return result;
}

v8::MaybeLocal<v8::Module> ModuleResolveCallback(v8::Local<v8::Context> context,
                                                 v8::Local<v8::String> specifier,
                                                 v8::Local<v8::Module> referrer)
{
    v8::Isolate* isolate = context->GetIsolate();
    int len = specifier->Length();
    char *buffer = new char[len + 1];
    specifier->WriteUtf8(isolate, buffer, len, nullptr,
                         v8::String::REPLACE_INVALID_UTF8 | v8::String::NO_NULL_TERMINATION);
    auto maybeSourceCode = ReadFile(isolate, buffer);
    v8::Local<v8::String> sourceCode;
    if (!maybeSourceCode.ToLocal(&sourceCode)) {
        v8::ScriptOrigin origin(v8::String::NewFromUtf8(isolate, "moduleloader.js").ToLocalChecked(),
                                v8::Local<v8::Integer>(), v8::Local<v8::Integer>(), v8::Local<v8::Boolean>(),
                                v8::Local<v8::Integer>(), v8::Local<v8::Value>(), v8::Local<v8::Boolean>(),
                                v8::Local<v8::Boolean>(), True(isolate));
        v8::ScriptCompiler::Source source(specifier, origin);
        delete[] buffer;
        return v8::ScriptCompiler::CompileModule(isolate, &source).ToLocalChecked();
    }

    v8::ScriptOrigin origin(specifier, v8::Local<v8::Integer>(), v8::Local<v8::Integer>(), v8::Local<v8::Boolean>(),
                            v8::Local<v8::Integer>(), v8::Local<v8::Value>(), v8::Local<v8::Boolean>(),
                            v8::Local<v8::Boolean>(), True(isolate));
    v8::ScriptCompiler::Source source(sourceCode, origin);
    auto result = v8::ScriptCompiler::CompileModule(isolate, &source).ToLocalChecked();
    delete[] buffer;
    return result;
}
}

NativeValue* V8NativeEngine::LoadModule(NativeValue* str, const std::string& fileName)
{
    v8::Local<v8::Value> value = *str;
    auto source = value.As<v8::String>();
    if (source.IsEmpty() || fileName.empty()) {
        isolate_->ThrowException(
            v8::String::NewFromUtf8(isolate_, "Invalid input parameter", v8::NewStringType::kNormal).ToLocalChecked());
        return nullptr;
    }

    v8::ScriptOrigin origin(v8::String::NewFromUtf8(isolate_, fileName.c_str()).ToLocalChecked(),
                            v8::Local<v8::Integer>(), v8::Local<v8::Integer>(), v8::Local<v8::Boolean>(),
                            v8::Local<v8::Integer>(), v8::Local<v8::Value>(), v8::Local<v8::Boolean>(),
                            v8::Local<v8::Boolean>(), True(isolate_));
    v8::ScriptCompiler::Source moduleSource(source, origin);
    v8::Local<v8::Module> module = v8::ScriptCompiler::CompileModule(isolate_, &moduleSource).ToLocalChecked();

    auto context = context_.Get(isolate_);
    if (!module->InstantiateModule(context, ModuleResolveCallback).FromJust()) {
        return nullptr;
    }
    auto maybeEvaluate = module->Evaluate(context);
    v8::Local<v8::Value> evaluate;
    if (!maybeEvaluate.ToLocal(&evaluate)) {
        return nullptr;
    }

    v8::Local<v8::Value> moduleNameSpace = module->GetModuleNamespace();
    v8::Local<v8::Object> nameSpaceObject = moduleNameSpace->ToObject(context).ToLocalChecked();
    auto exportObj = nameSpaceObject->Get(context, v8::String::NewFromUtf8(isolate_, "default").ToLocalChecked());
    v8::Local<v8::Value> result;
    if (!exportObj.ToLocal(&result)) {
        return nullptr;
    }

    // can use return V8ValueToNativeValue(this, result) ?
    return new V8NativeObject(this, result);
}

NativeValue* V8NativeEngine::DefineClass(const char* name,
                                         NativeCallback callback,
                                         void* data,
                                         const NativePropertyDescriptor* properties,
                                         size_t length)
{
    auto classConstructor = new V8NativeFunction(this, name, 0, callback, data);
    if (classConstructor == nullptr) {
        return nullptr;
    }

    auto classPrototype = new V8NativeObject(this);
    if (classPrototype == nullptr) {
        delete classConstructor;
        return nullptr;
    }

    classConstructor->SetProperty("prototype", classPrototype);

    for (size_t i = 0; i < length; i++) {
        if (properties[i].attributes & NATIVE_STATIC) {
            classConstructor->DefineProperty(properties[i]);
        } else {
            classPrototype->DefineProperty(properties[i]);
        }
    }

    return classConstructor;
}

NativeValue* V8NativeEngine::CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc)
{
    v8::Local<v8::Object> value = *constructor;
    v8::Local<v8::Value>* args = new v8::Local<v8::Value>[argc];
    for (size_t i = 0; i < argc && args != nullptr; i++) {
        args[i] = *argv[i];
    }

    v8::TryCatch tryCatch(isolate_);
    v8::MaybeLocal<v8::Value> maybeInstance = value->CallAsConstructor(context_.Get(isolate_), argc, args);
    delete[] args;

    v8::Local<v8::Value> result;
    if (maybeInstance.IsEmpty()) {
        result = v8::Undefined(isolate_);
    } else {
        result = maybeInstance.ToLocalChecked();
    }

    return V8ValueToNativeValue(this, result);
}

NativeReference* V8NativeEngine::CreateReference(NativeValue* value, uint32_t initialRefcount)
{
    return new V8NativeReference(this, value, initialRefcount, false);
}

bool V8NativeEngine::Throw(NativeValue* error)
{
    isolate_->ThrowException(*error);
    lastException_ = error;
    return true;
}

bool V8NativeEngine::Throw(NativeErrorType type, const char* code, const char* message)
{
    v8::Local<v8::Value> error;

    switch (type) {
        case NATIVE_COMMON_ERROR:
            error = v8::Exception::Error(v8::String::NewFromUtf8(isolate_, message).ToLocalChecked());
            break;
        case NATIVE_TYPE_ERROR:
            error = v8::Exception::TypeError(v8::String::NewFromUtf8(isolate_, message).ToLocalChecked());
            break;
        case NATIVE_RANGE_ERROR:
            error = v8::Exception::RangeError(v8::String::NewFromUtf8(isolate_, message).ToLocalChecked());
            break;
        default:
            return false;
    }
    if (code) {
        v8::Local<v8::Value> codeKey = v8::String::NewFromUtf8(isolate_, "code").ToLocalChecked();
        v8::Local<v8::Value> codeValue = v8::String::NewFromUtf8(isolate_, code).ToLocalChecked();
        error.As<v8::Object>()->Set(context_.Get(isolate_), codeKey, codeValue).FromJust();
    }

    isolate_->ThrowException(error);
    lastException_ = V8ValueToNativeValue(this, error);
    return true;
}

NativeValue* V8NativeEngine::V8ValueToNativeValue(V8NativeEngine* engine, v8::Local<v8::Value> value)
{
    NativeValue* result = nullptr;
    if (value->IsNull() || value->IsUndefined() || value->IsSymbol() || value->IsPromise()) {
        result = new V8NativeValue(engine, value);
    } else if (value->IsNumber()) {
        result = new V8NativeNumber(engine, value);
    } else if (value->IsString()) {
        result = new V8NativeString(engine, value);
    } else if (value->IsArray()) {
        result = new V8NativeArray(engine, value);
    } else if (value->IsFunction()) {
        result = new V8NativeFunction(engine, value);
    } else if (value->IsArrayBuffer()) {
        result = new V8NativeArrayBuffer(engine, value);
    } else if (value->IsDataView()) {
        result = new V8NativeDataView(engine, value);
    } else if (value->IsTypedArray()) {
        result = new V8NativeTypedArray(engine, value);
    } else if (value->IsExternal()) {
        result = new V8NativeExternal(engine, value);
    } else if (value->IsObject()) {
        result = new V8NativeObject(engine, value);
    } else if (value->IsBoolean()) {
        result = new V8NativeBoolean(engine, value);
    }
    return result;
}

void* V8NativeEngine::CreateRuntime()
{
    v8::Isolate::CreateParams createParams;
    createParams.array_buffer_allocator = isolate_->GetArrayBufferAllocator();
    v8::Isolate* isolate = v8::Isolate::New(createParams);
    v8::HandleScope handleScope(isolate);
    v8::Isolate::Scope isolateScope(isolate);
    v8::Local<v8::Context> context = v8::Context::New(isolate);
    v8::Persistent<v8::Context> persistContext;
    persistContext.Reset(isolate, context);
    if (context.IsEmpty()) {
        return nullptr;
    }

    V8NativeEngine* v8Engine = new V8NativeEngine(platform_, isolate, persistContext, jsEngine_);
    v8Engine->MarkAutoDispose();
    auto cleanEnv = [isolate]() {
        if (isolate != nullptr) {
            HILOG_INFO("worker:: cleanEnv is called");
            isolate->Dispose();
        }
    };
    v8Engine->SetCleanEnv(cleanEnv);
    return reinterpret_cast<void*>(v8Engine);
}

class Serializer {
public:
    explicit Serializer(v8::Isolate* isolate) : isolate_(isolate), v8Serializer_(isolate, nullptr) {}
    ~Serializer() = default;

    bool SerializeValue(v8::Local<v8::Value> value, v8::Local<v8::Value> transfer)
    {
        v8::Local<v8::Context> context = isolate_->GetCurrentContext();
        bool ok = false;
        DCHECK(!data_);
        data_.reset(new SerializationData);

        // check transfer list is right
        if (!CheckTransferReliability(transfer)) {
            return false;
        }

        // serial value
        v8Serializer_.WriteHeader();
        if (!v8Serializer_.WriteValue(context, value).To(&ok)) {
            data_.reset();
            return false;
        }

        // releasing Data Control Rights
        if (!DetachTransfer()) {
            data_.reset();
            return false;
        }

        std::pair<uint8_t*, size_t> pair = v8Serializer_.Release();
        data_->data_.reset(pair.first);
        data_->size_ = pair.second;
        return true;
    }

    std::unique_ptr<SerializationData> Release()
    {
        return std::move(data_);
    }

private:
    bool CheckTransferReliability(v8::Local<v8::Value> transfer)
    {
        if (transfer->IsUndefined()) {
            return true;
        }
        if (!transfer->IsArray()) {
            std::string msg = "Transfer list must be an Array or undefined";
            isolate_->ThrowException(
                v8::String::NewFromUtf8(isolate_, msg.c_str(), v8::NewStringType::kNormal).ToLocalChecked());
            return false;
        }

        v8::Local<v8::Array> transferArray = v8::Local<v8::Array>::Cast(transfer);
        uint32_t length = transferArray->Length();
        uint32_t arrayBufferIdx = 0;
        v8::Local<v8::Context> context = isolate_->GetCurrentContext();
        for (uint32_t i = 0; i < length; ++i) {
            v8::Local<v8::Value> element;
            if (transferArray->Get(context, i).ToLocal(&element)) {
                if (!element->IsArrayBuffer()) {
                    std::string msg = "Transfer array elements must be an ArrayBuffer";
                    isolate_->ThrowException(
                        v8::String::NewFromUtf8(isolate_, msg.c_str(), v8::NewStringType::kNormal).ToLocalChecked());
                    return false;
                }

                v8::Local<v8::ArrayBuffer> arrayBuffer = v8::Local<v8::ArrayBuffer>::Cast(element);
                auto iter = std::find(visitedTransfer_.begin(), visitedTransfer_.end(), arrayBuffer);
                if (iter != visitedTransfer_.end()) {
                    std::string msg = "ArrayBuffer occurs in the transfer array more than once";
                    isolate_->ThrowException(
                        v8::String::NewFromUtf8(isolate_, msg.c_str(), v8::NewStringType::kNormal).ToLocalChecked());
                    return false;
                }

                v8Serializer_.TransferArrayBuffer(arrayBufferIdx++, arrayBuffer);
                visitedTransfer_.emplace_back(isolate_, arrayBuffer);
            } else {
                return false;
            }
        }
        return true;
    }

    bool DetachTransfer()
    {
        for (const auto& item : visitedTransfer_) {
            v8::Local<v8::ArrayBuffer> arrayBuffer = v8::Local<v8::ArrayBuffer>::New(isolate_, item);
            if (!arrayBuffer->IsDetachable()) {
                std::string msg = "ArrayBuffer could not be transferred";
                isolate_->ThrowException(
                    v8::String::NewFromUtf8(isolate_, msg.c_str(), v8::NewStringType::kNormal).ToLocalChecked());
                return false;
            }

            auto backingStore = arrayBuffer->GetBackingStore();
            data_->backingStores_.push_back(std::move(backingStore));
            arrayBuffer->Detach();
        }

        return true;
    }

    v8::Isolate* isolate_ {nullptr};
    v8::ValueSerializer v8Serializer_;
    std::unique_ptr<SerializationData> data_;
    std::vector<v8::Global<v8::ArrayBuffer>> visitedTransfer_;
    std::vector<std::unique_ptr<v8::BackingStore>> backingStores_;

    DISALLOW_COPY_AND_ASSIGN(Serializer);
};

class Deserializer {
public:
    explicit Deserializer(v8::Isolate* isolate, std::unique_ptr<SerializationData> data)
        : isolate_(isolate), v8Deserializer_(isolate, data->GetData(), data->GetSize(), nullptr), data_(std::move(data))
    {
        v8Deserializer_.SetSupportsLegacyWireFormat(true);
    }
    ~Deserializer() = default;

    v8::MaybeLocal<v8::Value> DeserializeValue()
    {
        v8::Local<v8::Context> context = isolate_->GetCurrentContext();
        bool readResult = false;
        if (!v8Deserializer_.ReadHeader(context).To(&readResult)) {
            return v8::MaybeLocal<v8::Value>();
        }

        uint32_t index = 0;
        for (const auto& backingStore : data_->GetBackingStores()) {
            v8::Local<v8::ArrayBuffer> arrayBuffer = v8::ArrayBuffer::New(isolate_, std::move(backingStore));
            v8Deserializer_.TransferArrayBuffer(index++, arrayBuffer);
        }

        return v8Deserializer_.ReadValue(context);
    }

private:
    v8::Isolate* isolate_ {nullptr};
    v8::ValueDeserializer v8Deserializer_;
    std::unique_ptr<SerializationData> data_;

    DISALLOW_COPY_AND_ASSIGN(Deserializer);
};

NativeValue* V8NativeEngine::Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer)
{
    v8::Isolate* isolate = reinterpret_cast<V8NativeEngine*>(context)->GetIsolate();
    v8::Local<v8::Value> v8Value = *value;
    v8::Local<v8::Value> v8Transfer = *transfer;
    Serializer serializer(isolate);
    std::unique_ptr<SerializationData> data;
    if (serializer.SerializeValue(v8Value, v8Transfer)) {
        data = serializer.Release();
    }
    return reinterpret_cast<NativeValue*>(data.release());
}

NativeValue* V8NativeEngine::Deserialize(NativeEngine* context, NativeValue* recorder)
{
    v8::Isolate* isolate = reinterpret_cast<V8NativeEngine*>(context)->GetIsolate();
    std::unique_ptr<SerializationData> data(reinterpret_cast<SerializationData*>(recorder));
    Deserializer deserializer(isolate, std::move(data));
    v8::MaybeLocal<v8::Value> result = deserializer.DeserializeValue();
    return V8ValueToNativeValue(this, result.ToLocalChecked());
}

void V8NativeEngine::DeleteSerializationData(NativeValue* value) const
{
    SerializationData* data = reinterpret_cast<SerializationData*>(value);
    delete data;
}

void V8NativeEngine::SetPackagePath(const std::string& packagePath)
{
    auto moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager) {
        moduleManager->SetAppLibPath(packagePath.c_str());
    }
}

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value)
{
    return *value ? *value : "<string conversion failed>";
}

ExceptionInfo* V8NativeEngine::GetExceptionForWorker() const
{
    DCHECK(tryCatch_.HasCaught());
    v8::HandleScope handle_scope(isolate_);

    ExceptionInfo* exceptionInfo = new ExceptionInfo();
    v8::String::Utf8Value exception(isolate_, tryCatch_.Exception());
    const char* exceptionString = ToCString(exception);
    char* exceptionMessage = new char[strlen(exceptionString) + 1] { 0 };
    if (memcpy_s(exceptionMessage, strlen(exceptionString) + 1, exceptionString, strlen(exceptionString)) != EOK) {
        HILOG_INFO("worker:: memcpy_s error");
        delete exceptionInfo;
        delete[] exceptionMessage;
        return nullptr;
    }
    exceptionInfo->message_ = exceptionMessage;

    v8::Local<v8::Context> context = context_.Get(isolate_);
    v8::Context::Scope contextScope(context);
    v8::Local<v8::Message> message = tryCatch_.Message();
    if (!message.IsEmpty()) {
        int32_t lineno = message->GetLineNumber(context).FromJust();
        exceptionInfo->lineno_ = lineno;

        int32_t colno = message->GetStartColumn(context).FromJust();
        exceptionInfo->colno_ = colno;
    }
    return exceptionInfo;
}

NativeValue* V8NativeEngine::ValueToNativeValue(JSValueWrapper& value)
{
    v8::Local<v8::Value> v8Value = value;
    return V8ValueToNativeValue(this, v8Value);
}

void V8NativeEngine::SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef)
{
    if (rejectCallbackRef == nullptr || checkCallbackRef == nullptr) {
        HILOG_ERROR("rejectCallbackRef or checkCallbackRef is nullptr");
        return;
    }
    promiseRejectCallbackRef_ = rejectCallbackRef;
    checkCallbackRef_ = checkCallbackRef;
    isolate_->SetPromiseRejectCallback(PromiseRejectCallback);
}


void V8NativeEngine::PromiseRejectCallback(v8::PromiseRejectMessage message)
{
    v8::Local<v8::Promise> promise = message.GetPromise();
    v8::PromiseRejectEvent event = message.GetEvent();
    v8::Isolate* isolate = promise->GetIsolate();
    v8::Local<v8::Value> reason = message.GetValue();
    if (reason.IsEmpty()) {
        reason = v8::Undefined(isolate);
    }
    V8NativeEngine* engine = g_env;
    if (engine == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return;
    }
    v8::Local<v8::Function> promiseRejectCallback = *(engine->promiseRejectCallbackRef_->Get());

    if (promiseRejectCallback.IsEmpty()) {
        HILOG_ERROR("promiseRejectCallback is empty");
        return;
    }
    v8::Local<v8::Value> type = v8::Number::New(isolate, event);
    v8::Local<v8::Value> promiseValue(promise);
    v8::Local<v8::Context> context = engine->context_.Get(isolate);
    v8::Local<v8::Value> args[] = {type, promiseValue, reason};

    size_t size = sizeof(args) / sizeof(args[0]);
    bool succ = promiseRejectCallback->Call(context, v8::Undefined(isolate), size, args).IsEmpty();
    if (succ) {
        HILOG_ERROR("error : call function promiseRejectCallback is failed");
    }
    if (event == v8::kPromiseRejectWithNoHandler) {
        v8::Local<v8::Function> cb = *(engine->checkCallbackRef_->Get());
        isolate->EnqueueMicrotask(cb);
    }
}