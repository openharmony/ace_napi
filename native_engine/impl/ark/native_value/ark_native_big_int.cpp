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

#include "ark_native_big_int.h"
using panda::BigIntRef;

ArkNativeBigInt::ArkNativeBigInt(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeValue(engine, value)
{}

ArkNativeBigInt::ArkNativeBigInt(ArkNativeEngine* engine, int64_t value)
    : ArkNativeBigInt(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<BigIntRef> object = BigIntRef::New(vm, value);
    value_ = Global<BigIntRef>(vm, object);
}

ArkNativeBigInt::ArkNativeBigInt(ArkNativeEngine* engine, uint64_t value, bool isUnit64)
    : ArkNativeBigInt(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<BigIntRef> object = BigIntRef::New(vm, value);
    value_ = Global<BigIntRef>(vm, object);
}

ArkNativeBigInt::~ArkNativeBigInt() {}

void* ArkNativeBigInt::GetInterface(int interfaceId)
{
    return (NativeBigint::INTERFACE_ID == interfaceId) ? (NativeBigint*)this : nullptr;
}

void ArkNativeBigInt::Uint64Value(uint64_t* cValue, bool* lossless)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<BigIntRef> value = value_;
    value->BigIntToUint64(vm, cValue, lossless);
}

void ArkNativeBigInt::Int64Value(int64_t* cValue, bool* lossless)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<BigIntRef> value = value_;
    value->BigIntToInt64(vm, cValue, lossless);
}

bool ArkNativeBigInt::GetWordsArray(int* signBit, size_t* wordCount, uint64_t* words)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<BigIntRef> value = value_;
    if (wordCount == nullptr) {
        return false;
    }
    size_t size = static_cast<size_t>(value->GetWordsArraySize());
    if (signBit == nullptr && words == nullptr) {
        *wordCount = size;
        return true;
    } else if (signBit != nullptr && words != nullptr) {
        if (size > *wordCount) {
            size = *wordCount;
        }
        bool sign = false;
        value->GetWordsArray(&sign, size, words);
        if (sign) {
            *signBit = 1;
        } else {
            *signBit = 0;
        }
        *wordCount = size;
        return true;
    }
    return false;
}

ArkNativeBigInt::operator int64_t()
{
    int64_t cValue = 0;
    bool lossless = true;
    Int64Value(&cValue, &lossless);
    return cValue;
}

ArkNativeBigInt::operator uint64_t()
{
    uint64_t cValue = 0;
    bool lossless = true;
    Uint64Value(&cValue, &lossless);
    return cValue;
}