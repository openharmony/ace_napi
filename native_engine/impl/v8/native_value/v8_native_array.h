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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_ARRAY_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_ARRAY_H

#include "v8_native_object.h"

class V8NativeArray : public V8NativeObject, public NativeArray {
public:
    explicit V8NativeArray(V8NativeEngine* engine, v8::Local<v8::Value> value);
    explicit V8NativeArray(V8NativeEngine* engine, uint32_t length);
    virtual ~V8NativeArray();

    void* GetInterface(int interfaceId) override;

    virtual bool SetElement(uint32_t index, NativeValue* value) override;
    virtual NativeValue* GetElement(uint32_t index) override;
    virtual bool HasElement(uint32_t index) override;
    virtual bool DeleteElement(uint32_t index) override;

    virtual uint32_t GetLength() override;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_ARRAY_H */