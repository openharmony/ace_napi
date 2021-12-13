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

#include "jerryscript_native_array_buffer.h"

JerryScriptNativeArrayBuffer::JerryScriptNativeArrayBuffer(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeObject(engine, value)
{
}

JerryScriptNativeArrayBuffer::JerryScriptNativeArrayBuffer(JerryScriptNativeEngine* engine, void** value, size_t length)
    : JerryScriptNativeArrayBuffer(engine, jerry_create_arraybuffer(length))
{
    *value = jerry_get_arraybuffer_pointer(value_);
}

JerryScriptNativeArrayBuffer::JerryScriptNativeArrayBuffer(JerryScriptNativeEngine* engine,
                                                           void* value,
                                                           size_t length,
                                                           NativeFinalize callback,
                                                           void* hint)
    : JerryScriptNativeArrayBuffer(engine, 0)
{
    value_ = jerry_create_arraybuffer_external(length, (uint8_t*)value, [](void* nativePoint) { free(nativePoint); });
}

JerryScriptNativeArrayBuffer::~JerryScriptNativeArrayBuffer() {}

void* JerryScriptNativeArrayBuffer::GetInterface(int interfaceId)
{
    return (NativeArrayBuffer::INTERFACE_ID == interfaceId) ? (NativeArrayBuffer*)this
                                                            : JerryScriptNativeObject::GetInterface(interfaceId);
}

void* JerryScriptNativeArrayBuffer::GetBuffer()
{
    return jerry_get_arraybuffer_pointer(value_);
}

size_t JerryScriptNativeArrayBuffer::GetLength()
{
    return jerry_get_arraybuffer_byte_length(value_);
}

bool JerryScriptNativeArrayBuffer::IsDetachedArrayBuffer()
{
    int testResult = 40;
    int result = jerry_is_arraybuffer_detachable(value_);
    if (result == testResult) {
        return true;
    } else {
        return false;
    }
}

bool JerryScriptNativeArrayBuffer::DetachArrayBuffer()
{
    if (!IsDetachedArrayBuffer()) {
        jerry_detach_arraybuffer(value_);
        return true;
    } else {
        return false;
    }
}