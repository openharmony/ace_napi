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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_NATIVE_VALUE_QUICKJS_NATIVE_VALUE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_NATIVE_VALUE_QUICKJS_NATIVE_VALUE_H

#include "native_engine/native_value.h"
#include "quickjs_headers.h"

#include "quickjs_native_engine.h"

class QuickJSNativeValue : public NativeValue {
public:
    QuickJSNativeValue(QuickJSNativeEngine* engine, JSValue value);
    virtual ~QuickJSNativeValue();

    virtual void* GetInterface(int interfaceId) override;

    virtual NativeValueType TypeOf() override;
    virtual bool InstanceOf(NativeValue* obj) override;

    virtual bool IsArray() override;
    virtual bool IsArrayBuffer() override;
    virtual bool IsDate() override;
    virtual bool IsError() override;
    virtual bool IsTypedArray() override;
    virtual bool IsDataView() override;
    virtual bool IsPromise() override;

    virtual NativeValue* ToBoolean() override;
    virtual NativeValue* ToNumber() override;
    virtual NativeValue* ToString() override;
    virtual NativeValue* ToObject() override;

    virtual bool operator==(NativeValue* value) override;

protected:
    QuickJSNativeEngine* engine_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_NATIVE_VALUE_QUICKJS_NATIVE_VALUE_H */
