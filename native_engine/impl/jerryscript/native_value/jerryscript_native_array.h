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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_ARRAY_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_ARRAY_H

#include "jerryscript_native_object.h"

class JerryScriptNativeArray : public JerryScriptNativeObject, public NativeArray {
public:
    JerryScriptNativeArray(JerryScriptNativeEngine* engine, jerry_value_t value);
    JerryScriptNativeArray(JerryScriptNativeEngine* engine, int length);
    virtual ~JerryScriptNativeArray() override;

    void* GetInterface(int interfaceId) override;

    virtual bool SetElement(uint32_t index, NativeValue* value) override;
    virtual NativeValue* GetElement(uint32_t index) override;
    virtual bool HasElement(uint32_t index) override;
    virtual bool DeleteElement(uint32_t index) override;

    virtual uint32_t GetLength() override;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_ARRAY_H */
