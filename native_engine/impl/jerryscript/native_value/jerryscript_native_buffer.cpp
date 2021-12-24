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

#include "jerryscript_native_buffer.h"

#include <map>

#include "utils/log.h"

// The maximum length for NativaBuffer, default is 2MiB.
static constexpr size_t kMaxByteLength = 2097152;

static std::map<size_t, JerryScriptBufferCallback*> g_freeCallbackStorage;

struct JerryScriptBufferCallback {
    static JerryScriptBufferCallback* CreateNewInstance()
    {
        return new JerryScriptBufferCallback();
    }

    NativeEngine* engine = nullptr;
    NativeFinalize cb = nullptr;
    void* hint = nullptr;
};

JerryScriptNativeBuffer::JerryScriptNativeBuffer(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeObject(engine, value)
{
}

JerryScriptNativeBuffer::JerryScriptNativeBuffer(JerryScriptNativeEngine* engine, uint8_t** value, size_t length)
    : JerryScriptNativeBuffer(engine, CheckAndCreateBuffer(length))
{
    if (length > kMaxByteLength) {
        *value = nullptr;
    } else {
        *value = jerry_get_arraybuffer_pointer(value_);
    }
}

JerryScriptNativeBuffer::JerryScriptNativeBuffer(JerryScriptNativeEngine* engine, uint8_t** value, size_t length,
    const uint8_t* data) : JerryScriptNativeBuffer(engine, CheckAndCreateBuffer(length))
{
    if (length > kMaxByteLength) {
        *value = nullptr;
        return;
    }
    if (data != nullptr) {
        jerry_arraybuffer_write(value_, 0, data, length);
    }
    *value = jerry_get_arraybuffer_pointer(value_);
}

JerryScriptNativeBuffer::JerryScriptNativeBuffer(JerryScriptNativeEngine* engine,
                                                 uint8_t* data,
                                                 size_t length,
                                                 NativeFinalize callback,
                                                 void* hint)
    : JerryScriptNativeBuffer(engine, NULL)
{
    if (length > kMaxByteLength) {
        return;
    }
    auto freeCallback = JerryScriptBufferCallback::CreateNewInstance();
    if (freeCallback != nullptr) {
        freeCallback->engine = engine_;
        freeCallback->cb = callback;
        freeCallback->hint = hint;
    }

    g_freeCallbackStorage.insert(std::pair<size_t, JerryScriptBufferCallback*>((size_t)data, freeCallback));
    value_ = jerry_create_arraybuffer_external(length, data, [](void* nativePoint) {
        auto iter = g_freeCallbackStorage.find((size_t)nativePoint);
        if (iter != g_freeCallbackStorage.end()) {
            auto callb = iter->second;
            if (callb != nullptr && callb->cb != nullptr) {
                callb->cb(callb->engine, nativePoint, callb->hint);
                g_freeCallbackStorage.erase(iter);
                delete callb;
                callb = NULL;
            } else {
                free(nativePoint);
            }
        } else {
            free(nativePoint);
        }
    });
}

JerryScriptNativeBuffer::~JerryScriptNativeBuffer() {}

void* JerryScriptNativeBuffer::GetInterface(int interfaceId)
{
    return (NativeBuffer::INTERFACE_ID == interfaceId) ? (NativeBuffer*)this
        : JerryScriptNativeObject::GetInterface(interfaceId);
}

void* JerryScriptNativeBuffer::GetBuffer()
{
    return jerry_get_arraybuffer_pointer(value_);
}

size_t JerryScriptNativeBuffer::GetLength()
{
    return jerry_get_arraybuffer_byte_length(value_);
}

jerry_value_t JerryScriptNativeBuffer::CheckAndCreateBuffer(size_t length)
{
    jerry_value_t buf = NULL;

    if (length > kMaxByteLength) {
        HILOG_ERROR("The length(%{public}zu) exceeds the maximum byte length definition(%{public}zu).",
            length, kMaxByteLength);
        buf = jerry_create_null();
    } else {
        buf = jerry_create_arraybuffer(length);
    }
    return buf;
}
