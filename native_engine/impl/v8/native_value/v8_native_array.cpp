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

#include "v8_native_array.h"

V8NativeArray::V8NativeArray(V8NativeEngine* engine, v8::Local<v8::Value> value) : V8NativeObject(engine, value) {}

V8NativeArray::V8NativeArray(V8NativeEngine* engine, uint32_t length)
    : V8NativeArray(engine, v8::Array::New(engine->GetIsolate(), length))
{
}

V8NativeArray::~V8NativeArray() {}

void* V8NativeArray::GetInterface(int interfaceId)
{
    return (NativeArray::INTERFACE_ID == interfaceId) ? (NativeArray*)this
                                                      : V8NativeObject::GetInterface(interfaceId);
}

bool V8NativeArray::SetElement(uint32_t index, NativeValue* value)
{
    v8::Local<v8::Object> obj = value_;
    v8::Local<v8::Value> val = *value;
    auto setMaybe = obj->Set(engine_->GetContext(), index, val);
    return setMaybe.FromMaybe(false);
}

NativeValue* V8NativeArray::GetElement(uint32_t index)
{
    v8::Local<v8::Object> obj = value_;
    auto getMaybe = obj->Get(engine_->GetContext(), index);
    return V8NativeEngine::V8ValueToNativeValue(engine_, getMaybe.ToLocalChecked());
}

bool V8NativeArray::HasElement(uint32_t index)
{
    v8::Local<v8::Object> obj = value_;
    v8::Maybe<bool> hasMaybe = obj->Has(engine_->GetContext(), index);
    return hasMaybe.FromMaybe(false);
}

bool V8NativeArray::DeleteElement(uint32_t index)
{
    v8::Local<v8::Object> obj = value_;
    v8::Maybe<bool> deleteMaybe = obj->Delete(engine_->GetContext(), index);
    return deleteMaybe.FromMaybe(false);
}

uint32_t V8NativeArray::GetLength()
{
    v8::Local<v8::Array> obj = value_;
    return obj->Length();
}