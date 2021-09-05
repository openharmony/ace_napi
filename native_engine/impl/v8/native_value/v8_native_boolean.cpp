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

#include "v8_native_boolean.h"

V8NativeBoolean::V8NativeBoolean(V8NativeEngine* engine, v8::Local<v8::Value> value) : V8NativeValue(engine, value) {}

V8NativeBoolean::V8NativeBoolean(V8NativeEngine* engine, bool value)
    : V8NativeBoolean(engine, v8::Boolean::New(engine->GetIsolate(), value))
{
}

V8NativeBoolean::~V8NativeBoolean() {}

void* V8NativeBoolean::GetInterface(int interfaceId)
{
    return (NativeBoolean::INTERFACE_ID == interfaceId) ? (NativeBoolean*)this : nullptr;
}

V8NativeBoolean::operator bool()
{
    v8::Local<v8::Boolean> value = value_;
    return value->Value();
}