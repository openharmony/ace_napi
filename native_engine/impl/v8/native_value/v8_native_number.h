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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_NUMBER_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_NUMBER_H

#include "v8_native_value.h"

class V8NativeNumber : public V8NativeValue, public NativeNumber {
public:
    V8NativeNumber(V8NativeEngine* engine, v8::Local<v8::Value> value);
    V8NativeNumber(V8NativeEngine* engine, int32_t value);
    V8NativeNumber(V8NativeEngine* engine, uint32_t value);
    V8NativeNumber(V8NativeEngine* engine, int64_t value);
    V8NativeNumber(V8NativeEngine* engine, double value);
    virtual ~V8NativeNumber();

    virtual void* GetInterface(int interfaceId) override;

    virtual operator int32_t() override;
    virtual operator uint32_t() override;
    virtual operator int64_t() override;
    virtual operator double() override;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_NUMBER_H */