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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_QUICKJS_NATIVE_DATE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_QUICKJS_NATIVE_DATE_H

#include "jerryscript_native_object.h"

class JerryScriptNativeDate : public JerryScriptNativeObject, public NativeDate {
public:
    JerryScriptNativeDate(JerryScriptNativeEngine* engine, jerry_value_t value);
    ~JerryScriptNativeDate() override;

    void* GetInterface(int interfaceId) override;

    virtual double GetTime() override;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_QUICKJS_NATIVE_DATE_H */
