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

#include "v8_native_array_buffer.h"

struct V8NativeArrayBufferInfo {
    static V8NativeArrayBufferInfo* CreateNewInstance() { return new V8NativeArrayBufferInfo(); }
    NativeEngine* engine = nullptr;
    NativeFinalize cb = nullptr;
    void* hint = nullptr;
};

V8NativeArrayBuffer::V8NativeArrayBuffer(V8NativeEngine* engine, v8::Local<v8::Value> value)
    : V8NativeObject(engine, value)
{
}

V8NativeArrayBuffer::V8NativeArrayBuffer(V8NativeEngine* engine, uint8_t** value, size_t length)
    : V8NativeArrayBuffer(engine, v8::Local<v8::Value>())
{
    value_ = v8::ArrayBuffer::New(engine->GetIsolate(), length);
    if (value != nullptr) {
        v8::Local<v8::ArrayBuffer> obj = value_;
        *value = (uint8_t*)obj->GetBackingStore()->Data();
    }
}

V8NativeArrayBuffer::V8NativeArrayBuffer(V8NativeEngine* engine,
                                         uint8_t* value,
                                         size_t length,
                                         NativeFinalize cb,
                                         void* hint)
    : V8NativeArrayBuffer(engine, v8::Local<v8::Value>())
{
    auto cbinfo = V8NativeArrayBufferInfo::CreateNewInstance();
    if (cbinfo != nullptr) {
        cbinfo->engine = engine_;
        cbinfo->cb = cb;
        cbinfo->hint = hint;
    }

    auto backingStore = v8::ArrayBuffer::NewBackingStore(
        value, length,
        [](void* data, size_t length, void* deleterData) -> void {
            auto cbinfo = (V8NativeArrayBufferInfo*)deleterData;
            if (cbinfo != nullptr) {
                cbinfo->cb(cbinfo->engine, data, cbinfo->hint);
                delete cbinfo;
            }
        },
        cbinfo);

    value_ = v8::ArrayBuffer::New(engine->GetIsolate(), std::shared_ptr<v8::BackingStore>(backingStore.release()));
}

V8NativeArrayBuffer::~V8NativeArrayBuffer() {}

void* V8NativeArrayBuffer::GetInterface(int interfaceId)
{
    return (NativeArrayBuffer::INTERFACE_ID == interfaceId) ? (NativeArrayBuffer*)this
                                                            : V8NativeObject::GetInterface(interfaceId);
}

void* V8NativeArrayBuffer::GetBuffer()
{
    v8::Local<v8::ArrayBuffer> v = value_;
    return v->GetBackingStore()->Data();
}

size_t V8NativeArrayBuffer::GetLength()
{
    v8::Local<v8::ArrayBuffer> v = value_;
    return v->ByteLength();
}