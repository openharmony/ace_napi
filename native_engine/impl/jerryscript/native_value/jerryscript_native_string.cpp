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

#include "jerryscript_native_string.h"

JerryScriptNativeString::JerryScriptNativeString(JerryScriptNativeEngine* engine, const char* value, size_t length)
    : JerryScriptNativeString(engine, jerry_create_string_sz_from_utf8((const unsigned char*)value, length))
{
}

JerryScriptNativeString::JerryScriptNativeString(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeValue(engine, value)
{
}

JerryScriptNativeString::~JerryScriptNativeString() {}

void* JerryScriptNativeString::GetInterface(int interfaceId)
{
    return (NativeString::INTERFACE_ID == interfaceId) ? (NativeString*)this : nullptr;
}

void JerryScriptNativeString::GetCString(char* buffer, size_t size, size_t* length)
{
    if (buffer == nullptr || size == 0) {
        *length = GetLength();
    } else {
        *length = jerry_string_to_utf8_char_buffer(value_, (jerry_char_t*)buffer, size);
    }
}

size_t JerryScriptNativeString::GetLength()
{
    return jerry_get_utf8_string_size(value_);
}
