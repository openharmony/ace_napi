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

#include "v8_native_value.h"

V8NativeValue::V8NativeValue(V8NativeEngine* engine, v8::Local<v8::Value> value)
{
    engine_ = engine;
    value_ = value;

    NativeScopeManager* scopeManager = engine_->GetScopeManager();
    if (scopeManager != nullptr) {
        scopeManager->CreateHandle(this);
    }
}

V8NativeValue::~V8NativeValue() {}

void* V8NativeValue::GetInterface(int interfaceId)
{
    return nullptr;
}

NativeValueType V8NativeValue::TypeOf()
{
    v8::Local<v8::Value> value = value_;
    NativeValueType result;

    if (value->IsNumber()) {
        result = NATIVE_NUMBER;
    } else if (value->IsBigInt()) {
        result = NATIVE_BIGINT;
    } else if (value->IsString()) {
        result = NATIVE_STRING;
    } else if (value->IsFunction()) {
        result = NATIVE_FUNCTION;
    } else if (value->IsExternal()) {
        result = NATIVE_EXTERNAL;
    } else if (value->IsObject()) {
        result = NATIVE_OBJECT;
    } else if (value->IsBoolean()) {
        result = NATIVE_BOOLEAN;
    } else if (value->IsUndefined()) {
        result = NATIVE_UNDEFINED;
    } else if (value->IsSymbol()) {
        result = NATIVE_SYMBOL;
    } else if (value->IsNull()) {
        result = NATIVE_NULL;
    } else {
        result = NATIVE_UNDEFINED;
    }

    return result;
}

bool V8NativeValue::InstanceOf(NativeValue* obj)
{
    v8::Local<v8::Value> value = value_;
    return value->InstanceOf(engine_->GetContext(), *obj).FromJust();
}

bool V8NativeValue::IsArray()
{
    v8::Local<v8::Value> value = value_;
    return value->IsArray();
}

bool V8NativeValue::IsArrayBuffer()
{
    v8::Local<v8::Value> value = value_;
    return value->IsArrayBuffer();
}

bool V8NativeValue::IsDate()
{
    v8::Local<v8::Value> value = value_;
    return value->IsDate();
}

bool V8NativeValue::IsError()
{
    v8::Local<v8::Value> value = value_;
    return value->IsNativeError();
}

bool V8NativeValue::IsTypedArray()
{
    v8::Local<v8::Value> value = value_;
    return value->IsTypedArray();
}

bool V8NativeValue::IsDataView()
{
    v8::Local<v8::Value> value = value_;
    return value->IsDataView();
}

bool V8NativeValue::IsPromise()
{
    v8::Local<v8::Value> value = value_;
    return value->IsPromise();
}

bool V8NativeValue::IsCallable()
{
    v8::Local<v8::Value> value = value_;
    return value->IsFunction();
}

NativeValue* V8NativeValue::ToBoolean()
{
    v8::Local<v8::Value> value = value_;
    return V8NativeEngine::V8ValueToNativeValue(engine_, value->ToBoolean(engine_->GetIsolate()));
}

NativeValue* V8NativeValue::ToNumber()
{
    v8::Local<v8::Value> value = value_;
    return V8NativeEngine::V8ValueToNativeValue(engine_, value->ToNumber(engine_->GetContext()).ToLocalChecked());
}

NativeValue* V8NativeValue::ToString()
{
    v8::Local<v8::Value> value = value_;
    return V8NativeEngine::V8ValueToNativeValue(engine_, value->ToString(engine_->GetContext()).ToLocalChecked());
}

NativeValue* V8NativeValue::ToObject()
{
    v8::Local<v8::Value> value = value_;
    return V8NativeEngine::V8ValueToNativeValue(engine_, value->ToObject(engine_->GetContext()).ToLocalChecked());
}

bool V8NativeValue::StrictEquals(NativeValue* value)
{
    v8::Local<v8::Value> v8Value = value_;
    return v8Value->StrictEquals(*value);
}