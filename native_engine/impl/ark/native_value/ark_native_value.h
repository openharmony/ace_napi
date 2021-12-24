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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_VALUE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_VALUE_H

#include "ark_native_engine.h"

using panda::Local;
using panda::LocalScope;
using panda::Global;
using panda::JSValueRef;
class ArkNativeValue : public NativeValue {
public:
    ArkNativeValue(ArkNativeEngine* engine, Local<JSValueRef> value);
    ~ArkNativeValue() override;

    void* GetInterface(int interfaceId) override;

    NativeValueType TypeOf() override;
    bool InstanceOf(NativeValue* obj) override;

    bool IsArray() override;
    bool IsArrayBuffer() override;
    bool IsBuffer() override;
    bool IsDate() override;
    bool IsError() override;
    bool IsTypedArray() override;
    bool IsDataView() override;
    bool IsPromise() override;
    bool IsCallable() override;

    bool IsArgumentsObject() override;
    bool IsAsyncFunction() override;
    bool IsBooleanObject() override;
    bool IsGeneratorFunction() override;
    bool IsGeneratorObject() override;
    bool IsMap() override;
    bool IsMapIterator() override;
    bool IsModuleNamespaceObject() override;
    bool IsNumberObject() override;
    bool IsProxy() override;
    bool IsRegExp() override;
    bool IsSet() override;
    bool IsSetIterator() override;
    bool IsStringObject() override;
    bool IsSymbolObject() override;
    bool IsWeakMap() override;
    bool IsWeakSet() override;

    NativeValue* ToBoolean() override;
    NativeValue* ToNumber() override;
    NativeValue* ToString() override;
    NativeValue* ToObject() override;

    bool StrictEquals(NativeValue* value) override;

    // value_ in NativeValue should not be replaced directly.
    // If you must replace it, you should use this method.
    void UpdateValue(Local<JSValueRef> value);

protected:
    ArkNativeEngine* engine_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_VALUE_H */