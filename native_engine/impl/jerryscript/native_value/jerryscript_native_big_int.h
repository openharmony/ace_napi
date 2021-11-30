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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_QUICKJS_NATIVE_BIG_INT_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_QUICKJS_NATIVE_BIG_INT_H

#include "jerryscript_native_value.h"

class JerryScriptNativeBigInt : public JerryScriptNativeValue, public NativeBigint {
public:
    JerryScriptNativeBigInt(JerryScriptNativeEngine* engine, jerry_value_t value);
    JerryScriptNativeBigInt(JerryScriptNativeEngine* engine, int64_t value);
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    JerryScriptNativeBigInt(JerryScriptNativeEngine* engine, uint64_t value, bool isUnit64);
#endif
    ~JerryScriptNativeBigInt() override;

    void* GetInterface(int interfaceId) override;

    virtual operator int64_t() override;
    virtual operator uint64_t() override;
    virtual void Uint64Value(uint64_t* cValue, bool* lossless = nullptr) override;
    virtual void Int64Value(int64_t* cValue, bool* lossless = nullptr) override;
    virtual bool GetWordsArray(int* signBit, size_t* wordCount, uint64_t* words) override;

private:
    jerry_value_t CreateBigintByInt(int64_t value);
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_QUICKJS_NATIVE_BIG_INT_H */