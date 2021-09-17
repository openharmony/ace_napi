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

#include "jerryscript_native_number.h"

JerryScriptNativeNumber::JerryScriptNativeNumber(JerryScriptNativeEngine* engine, double value)
    : JerryScriptNativeNumber(engine, jerry_create_number((double)value))
{
}

JerryScriptNativeNumber::JerryScriptNativeNumber(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeValue(engine, value)
{
}

JerryScriptNativeNumber::~JerryScriptNativeNumber() {}

void* JerryScriptNativeNumber::GetInterface(int interfaceId)
{
    return (NativeNumber::INTERFACE_ID == interfaceId) ? (NativeNumber*)this : nullptr;
}

JerryScriptNativeNumber::operator int32_t()
{
    return (int32_t)jerry_get_number_value(value_);
}

JerryScriptNativeNumber::operator uint32_t()
{
    return (uint32_t)jerry_get_number_value(value_);
}

JerryScriptNativeNumber::operator int64_t()
{
    return (int64_t)jerry_get_number_value(value_);
}

JerryScriptNativeNumber::operator double()
{
    return (double)jerry_get_number_value(value_);
}
