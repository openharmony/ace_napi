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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_VALUE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_VALUE_H

#include "jerryscript_headers.h"

#include "native_engine/native_value.h"

#include "jerryscript_native_engine.h"

class JerryScriptNativeValue : public NativeValue {
public:
    JerryScriptNativeValue(JerryScriptNativeEngine* engine, jerry_value_t value);
    virtual ~JerryScriptNativeValue() override;

    void* GetInterface(const int interfaceId) override;

    virtual NativeValueType TypeOf() override;
    virtual bool InstanceOf(NativeValue* obj) override;

    virtual bool IsArray() override;
    virtual bool IsArrayBuffer() override;
    virtual bool IsBuffer() override;
    virtual bool IsDate() override;
    virtual bool IsError() override;
    virtual bool IsTypedArray() override;
    virtual bool IsDataView() override;
    virtual bool IsPromise() override;
    virtual bool IsCallable() override { return false; }

    virtual bool IsMapIterator() override;
    virtual bool IsSetIterator() override;
    virtual bool IsGeneratorObject() override;
    virtual bool IsModuleNamespaceObject() override;
    virtual bool IsProxy() override;
    virtual bool IsRegExp() override;
    virtual bool IsNumberObject() override;
    virtual bool IsMap() override;
    virtual bool IsSet() override;
    virtual bool IsKeyObject() override;
    virtual bool IsArgumentsObject() override;
    virtual bool IsAsyncFunction() override;
    virtual bool IsBooleanObject() override;
    virtual bool IsCryptoKey() override;
    virtual bool IsGeneratorFunction() override;
    virtual bool IsSharedArrayBuffer() override;
    virtual bool IsStringObject() override;
    virtual bool IsSymbolObject() override;
    virtual bool IsWeakMap() override;
    virtual bool IsWeakSet() override;

    virtual NativeValue* ToBoolean() override;
    virtual NativeValue* ToNumber() override;
    virtual NativeValue* ToString() override;
    virtual NativeValue* ToObject() override;

    virtual bool StrictEquals(NativeValue* value) override;

protected:
    JerryScriptNativeEngine* engine_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_VALUE_H */
