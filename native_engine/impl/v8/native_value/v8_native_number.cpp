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

#include "v8_native_number.h"

#include <math.h>

V8NativeNumber::V8NativeNumber(V8NativeEngine* engine, v8::Local<v8::Value> value) : V8NativeValue(engine, value) {}

V8NativeNumber::V8NativeNumber(V8NativeEngine* engine, int32_t value)
    : V8NativeNumber(engine, v8::Number::New(engine->GetIsolate(), value))
{
}

V8NativeNumber::V8NativeNumber(V8NativeEngine* engine, uint32_t value)
    : V8NativeNumber(engine, v8::Number::New(engine->GetIsolate(), value))
{
}

V8NativeNumber::V8NativeNumber(V8NativeEngine* engine, int64_t value)
    : V8NativeNumber(engine, v8::Number::New(engine->GetIsolate(), value))
{
}

V8NativeNumber::V8NativeNumber(V8NativeEngine* engine, double value)
    : V8NativeNumber(engine, v8::Number::New(engine->GetIsolate(), value))
{
}

V8NativeNumber::~V8NativeNumber() {}

void* V8NativeNumber::GetInterface(int interfaceId)
{
    return (NativeNumber::INTERFACE_ID == interfaceId) ? (NativeNumber*)this : nullptr;
}

V8NativeNumber::operator int32_t()
{
    v8::Local<v8::Value> value = value_;
    int32_t result = 0;

    if (value->IsInt32()) {
        result = value.As<v8::Int32>()->Value();
    }

    double doubleValue = value.As<v8::Number>()->Value();
    if (isfinite(doubleValue)) {
        result = value->IntegerValue(engine_->GetContext()).FromJust();
    } else {
        result = 0;
    }

    return result;
}

V8NativeNumber::operator uint32_t()
{
    v8::Local<v8::Value> value = value_;
    uint32_t result = 0;

    if (value->IsUint32()) {
        result = value.As<v8::Uint32>()->Value();
    } else {
        result = value->Uint32Value(engine_->GetContext()).FromJust();
    }

    return result;
}

V8NativeNumber::operator int64_t()
{
    v8::Local<v8::Value> value = value_;
    int64_t result = 0;

    if (value->IsInt32()) {
        result = value.As<v8::Int32>()->Value();
    }

    double doubleValue = value.As<v8::Number>()->Value();
    if (isfinite(doubleValue)) {
        result = value->IntegerValue(engine_->GetContext()).FromJust();
    } else {
        result = 0;
    }

    return result;
}

V8NativeNumber::operator double()
{
    v8::Local<v8::Value> value = value_;
    double result = 0.0;

    result = value.As<v8::Number>()->Value();

    return result;
}