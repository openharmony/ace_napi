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

#include "ark_native_array_buffer.h"
#include "utils/log.h"

using panda::ArrayBufferRef;
ArkNativeArrayBuffer::ArkNativeArrayBuffer(ArkNativeEngine* engine, Local<JSValueRef> value)
    : ArkNativeObject(engine, value)
{
}

ArkNativeArrayBuffer::ArkNativeArrayBuffer(ArkNativeEngine* engine, uint8_t** value, size_t length)
    : ArkNativeArrayBuffer(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    value_ = Global<ArrayBufferRef>(vm, ArrayBufferRef::New(vm, length));
    if (value != nullptr) {
        Global<ArrayBufferRef> obj = value_;
        *value = reinterpret_cast<uint8_t*>(obj->GetBuffer());
    }
}

ArkNativeArrayBuffer::ArkNativeArrayBuffer(ArkNativeEngine* engine,
                                           uint8_t* value,
                                           size_t length,
                                           NativeFinalize cb,
                                           void* hint)
    : ArkNativeArrayBuffer(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);

    NativeObjectInfo* cbinfo = NativeObjectInfo::CreateNewInstance();
    if (cbinfo == nullptr) {
        HILOG_ERROR("cbinfo is nullptr");
        return;
    }
    cbinfo->engine = engine_;
    cbinfo->callback = cb;
    cbinfo->hint = hint;

    Local<ArrayBufferRef> object = ArrayBufferRef::New(vm, value, length,
        [](void* data, void* info) {
            auto externalInfo = reinterpret_cast<NativeObjectInfo*>(info);
            auto engine = externalInfo->engine;
            auto callback = externalInfo->callback;
            auto hint = externalInfo->hint;
            if (callback != nullptr) {
                callback(engine, data, hint);
            }
            delete externalInfo;
        },
        cbinfo);

    value_ = Global<ArrayBufferRef>(vm, object);
}

ArkNativeArrayBuffer::~ArkNativeArrayBuffer() {}

void* ArkNativeArrayBuffer::GetInterface(int interfaceId)
{
    return (NativeArrayBuffer::INTERFACE_ID == interfaceId) ? (NativeArrayBuffer*)this
                                                            : ArkNativeObject::GetInterface(interfaceId);
}

void* ArkNativeArrayBuffer::GetBuffer()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ArrayBufferRef> v = value_;
    return v->GetBuffer();
}

size_t ArkNativeArrayBuffer::GetLength()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ArrayBufferRef> v = value_;
    return v->ByteLength(vm);
}

bool ArkNativeArrayBuffer::IsDetachedArrayBuffer()
{
    return true;
}

bool ArkNativeArrayBuffer::DetachArrayBuffer()
{
    return false;
}