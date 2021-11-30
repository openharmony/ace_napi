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

#include "jerryscript_native_big_int.h"

JerryScriptNativeBigInt::JerryScriptNativeBigInt(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeValue(engine, value)
{}

JerryScriptNativeBigInt::JerryScriptNativeBigInt(JerryScriptNativeEngine* engine, int64_t value)
    : JerryScriptNativeBigInt(engine, CreateBigintByInt(value))
{}

#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
JerryScriptNativeBigInt::JerryScriptNativeBigInt(JerryScriptNativeEngine* engine, uint64_t value, bool isUnit64)
    : JerryScriptNativeBigInt(engine, jerry_create_bigint(&value, 1, false))
{}
#endif

JerryScriptNativeBigInt::~JerryScriptNativeBigInt() {}

void* JerryScriptNativeBigInt::GetInterface(int interfaceId)
{
    return (NativeBigint::INTERFACE_ID == interfaceId) ? (NativeBigint*)this : nullptr;
}

void JerryScriptNativeBigInt::Uint64Value(uint64_t* cValue, bool* lossless)
{
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    size_t size = (size_t)jerry_get_bigint_size_in_digits(value_);
    if (size == 1) {
        *lossless = true;
    } else {
        *lossless = false;
    }
    bool sign = false;
    jerry_get_bigint_digits(value_, cValue, 1, &sign);
#endif
}
void JerryScriptNativeBigInt::Int64Value(int64_t* cValue, bool* lossless)
{
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    size_t size = (size_t)jerry_get_bigint_size_in_digits(value_);
    if (size == 1) {
        *lossless = true;
    } else {
        *lossless = false;
    }
    bool sign = false;
    uint64_t uintValue = 0;
    jerry_get_bigint_digits(value_, &uintValue, 1, &sign);
    auto intValue = static_cast<int64_t>(uintValue);
    if (sign) {
        *cValue = -intValue;
    } else {
        *cValue = intValue;
    }
#endif
}

JerryScriptNativeBigInt::operator int64_t()
{
    int64_t cValue = 0;
    bool lossless = true;
    Int64Value(&cValue, &lossless);
    return cValue;
}

JerryScriptNativeBigInt::operator uint64_t()
{
    uint64_t cValue = 0;
    bool lossless = true;
    Uint64Value(&cValue, &lossless);
    return cValue;
}

bool JerryScriptNativeBigInt::GetWordsArray(int* signBit, size_t* wordCount, uint64_t* words)
{
    if (wordCount == nullptr) {
        return false;
    }
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    size_t size = (size_t)jerry_get_bigint_size_in_digits(value_);

    if (signBit == nullptr && words == nullptr) {
        *wordCount = size;
        return true;
    } else if (signBit != nullptr && words != nullptr) {
        if (size > *wordCount) {
            size = *wordCount;
        }
        bool sign = false;
        jerry_get_bigint_digits(value_, words, size, &sign);
        if (sign) {
            *signBit = 1;
        } else {
            *signBit = 0;
        }
        *wordCount = size;
        return true;
    }
    return false;
#else
    return true;
#endif
}

jerry_value_t JerryScriptNativeBigInt::CreateBigintByInt(int64_t value)
{
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    bool isHaveSign = false;
    isHaveSign = value < 0;
    auto uintValue = value < 0 ? static_cast<uint64_t>(-value) : static_cast<uint64_t>(value);
    return jerry_create_bigint(&uintValue, 1, isHaveSign);
#else
    jerry_value_t temp = 0;
    return temp;
#endif
}