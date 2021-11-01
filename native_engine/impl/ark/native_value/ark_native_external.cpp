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

#include "ark_native_external.h"
#include "ark_native_reference.h"
#include "utils/log.h"

using panda::NativePointerRef;

ArkNativeExternal::ArkNativeExternal(ArkNativeEngine* engine, void* value, NativeFinalize callback, void* hint)
    : ArkNativeExternal(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);

    NativeObjectInfo* info = NativeObjectInfo::CreateNewInstance();
    if (info == nullptr) {
        HILOG_ERROR("info is nullptr");
        return;
    }
    info->engine = engine;
    info->nativeObject = nullptr;
    info->callback = callback;
    info->hint = hint;

    Local<NativePointerRef> object = NativePointerRef::New(vm, value, ArkExternalDeleterCallback, info);
    value_ = Global<NativePointerRef>(vm, object);
}

ArkNativeExternal::ArkNativeExternal(ArkNativeEngine* engine, Local<JSValueRef> value)
    : ArkNativeValue(engine, value) {}

ArkNativeExternal::~ArkNativeExternal()
{
}

void* ArkNativeExternal::GetInterface(int interfaceId)
{
    return (NativeExternal::INTERFACE_ID == interfaceId) ? (NativeExternal*)this : nullptr;
}

ArkNativeExternal::operator void*()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<NativePointerRef> value = value_;
    return value->Value();
}

// static
void ArkNativeExternal::ArkExternalDeleterCallback(void* data, void* info)
{
    auto externalInfo = reinterpret_cast<NativeObjectInfo*>(info);
    auto engine = externalInfo->engine;
    auto callback = externalInfo->callback;
    auto hint = externalInfo->hint;
    if (callback != nullptr) {
        callback(engine, data, hint);
    }
    delete externalInfo;
}