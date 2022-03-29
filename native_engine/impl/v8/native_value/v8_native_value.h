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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_VALUE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_VALUE_H

#include "v8_native_engine.h"

class V8NativeValue : public NativeValue {
public:
    V8NativeValue(V8NativeEngine* engine, v8::Local<v8::Value> value);
    ~V8NativeValue() override;

    void* GetInterface(int interfaceId) override;

    NativeValueType TypeOf() override;
    bool InstanceOf(NativeValue* obj) override;

    bool IsArray() override;
    bool IsArrayBuffer() override;
    bool IsDate() override;
    bool IsError() override;
    bool IsTypedArray() override;
    bool IsDataView() override;
    bool IsPromise() override;
    bool IsCallable() override;
    bool IsBuffer() override
    {
        return false;
    }

    NativeValue* ToBoolean() override;
    NativeValue* ToNumber() override;
    NativeValue* ToString() override;
    NativeValue* ToObject() override;

    bool StrictEquals(NativeValue* value) override;

protected:
    V8NativeEngine* engine_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_VALUE_H */
