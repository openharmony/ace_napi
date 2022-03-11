/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_BIGINT_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_BIGINT_H

#include "ark_native_value.h"

class ArkNativeBigInt : public ArkNativeValue, public NativeBigint {
public:
    ArkNativeBigInt(ArkNativeEngine* engine, Local<JSValueRef> value);
    ArkNativeBigInt(ArkNativeEngine* engine, int64_t value);
    ArkNativeBigInt(ArkNativeEngine* engine, uint64_t value, bool isUnit64);
    ~ArkNativeBigInt() override;

    void* GetInterface(int interfaceId) override;
    virtual operator int64_t() override;
    virtual operator uint64_t() override;
    virtual void Uint64Value(uint64_t* cValue, bool* lossless = nullptr) override;
    virtual void Int64Value(int64_t* cValue, bool* lossless = nullptr) override;
    virtual bool GetWordsArray(int* signBit, size_t* wordCount, uint64_t* words) override;
};
#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_BIGINT_H */