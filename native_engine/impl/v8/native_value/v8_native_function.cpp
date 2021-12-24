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

#include "v8_native_function.h"

#include "utils/log.h"

V8NativeFunction::V8NativeFunction(V8NativeEngine* engine, v8::Local<v8::Value> value) : V8NativeObject(engine, value)
{
}

V8NativeFunction::V8NativeFunction(V8NativeEngine* engine,
                                   const char* name,
                                   size_t length,
                                   NativeCallback cb,
                                   void* value)
    : V8NativeFunction(engine, v8::Local<v8::Value>())
{
    auto context = engine->GetContext();
    auto isolate = engine->GetIsolate();
    v8::Local<v8::Array> cbdata = v8::Array::New(isolate);

    int32_t index = 0;
    cbdata->Set(context, index++, v8::External::New(isolate, (void*)engine)).FromJust();
    cbdata->Set(context, index++, v8::External::New(isolate, (void*)cb)).FromJust();
    cbdata->Set(context, index, v8::External::New(isolate, value)).FromJust();

    v8::Local<v8::Function> fn =
        v8::Function::New(context, NativeFunctionCallback, cbdata).ToLocalChecked();

    v8::Local<v8::String> fnName = v8::String::NewFromUtf8(isolate, name).ToLocalChecked();

    fn->SetName(fnName);

    value_ = fn;
}

V8NativeFunction::~V8NativeFunction() {}

void* V8NativeFunction::GetInterface(int interfaceId)
{
    return (NativeFunction::INTERFACE_ID == interfaceId) ? (NativeFunction*)this
                                                         : V8NativeObject::GetInterface(interfaceId);
}

void V8NativeFunction::NativeFunctionCallback(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate::Scope isolateScope(info.GetIsolate());
    v8::HandleScope handleScope(info.GetIsolate());
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    v8::Local<v8::Array> cbdata = info.Data().As<v8::Array>();

    int32_t index = 0;
    V8NativeEngine* engine =
        (V8NativeEngine*)cbdata->Get(context, index++).ToLocalChecked().As<v8::External>()->Value();
    if (engine == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return;
    }
    NativeCallback cb = (NativeCallback)cbdata->Get(context, index++).ToLocalChecked().As<v8::External>()->Value();
    void* data = cbdata->Get(context, index).ToLocalChecked().As<v8::External>()->Value();

    auto funcinfo = new NativeFunctionInfo();
    if (funcinfo == nullptr) {
        HILOG_ERROR("create native function info failed");
        return;
    }

    funcinfo->engine = engine;
    funcinfo->callback = cb;
    funcinfo->data = data;

    NativeScopeManager* scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        HILOG_ERROR("Get scope manager failed");
        delete funcinfo;
        return;
    }
    NativeScope* scope = scopeManager->Open();
    if (scope == nullptr) {
        HILOG_ERROR("Open scope failed");
        delete funcinfo;
        return;
    }

    NativeCallbackInfo cbinfo = { 0 };
    cbinfo.thisVar = V8NativeEngine::V8ValueToNativeValue(engine, info.This());
    cbinfo.function = V8NativeEngine::V8ValueToNativeValue(engine, info.NewTarget());
    cbinfo.argc = info.Length();
    cbinfo.argv = nullptr;
    cbinfo.functionInfo = funcinfo;
    if (cbinfo.argc > 0) {
        cbinfo.argv = new NativeValue* [cbinfo.argc];
        for (size_t i = 0; i < cbinfo.argc && cbinfo.argv != nullptr; i++) {
            cbinfo.argv[i] = V8NativeEngine::V8ValueToNativeValue(engine, info[i]);
        }
    }

    NativeValue* result = nullptr;
    if (cb != nullptr) {
        result = cb(engine, &cbinfo);
    }

    if (cbinfo.argv != nullptr) {
        delete []cbinfo.argv;
    }
    delete funcinfo;

    v8::Local<v8::Value> v8Result;
    if (result == nullptr) {
        if (engine->IsExceptionPending()) {
            NativeValue* error = engine->GetAndClearLastException();
            if (error != nullptr) {
                v8Result = *error;
            }
        } else {
            v8Result = v8::Undefined(engine->GetIsolate());
        }
    } else {
        v8Result = *result;
    }

    info.GetReturnValue().Set<v8::Value>(v8Result);
    scopeManager->Close(scope);
}