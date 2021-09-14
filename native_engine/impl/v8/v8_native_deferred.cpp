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

#include "v8_native_deferred.h"

V8NativeDeferred::V8NativeDeferred(V8NativeEngine* engine, v8::Local<v8::Promise::Resolver> deferred)
    : engine_(engine), deferred_(engine->GetIsolate(), deferred)
{
}

V8NativeDeferred::~V8NativeDeferred() {}

void V8NativeDeferred::Resolve(NativeValue* data)
{
    v8::Local<v8::Context> context = engine_->GetContext();
    v8::Isolate* isolate = engine_->GetIsolate();

    v8::Local<v8::Value> value = *data;
    auto v8Resolver = deferred_.Get(isolate);

    v8Resolver->Resolve(context, value).ToChecked();
}

void V8NativeDeferred::Reject(NativeValue* reason)
{
    v8::Local<v8::Context> context = engine_->GetContext();
    v8::Isolate* isolate = engine_->GetIsolate();

    v8::Local<v8::Value> value = *reason;
    auto v8Resolver = deferred_.Get(isolate);

    v8Resolver->Reject(context, value).ToChecked();
}