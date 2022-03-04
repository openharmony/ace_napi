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

#include "ark_native_engine.h"

#include "ark_native_reference.h"

#ifdef ENABLE_CONTAINER_SCOPE
#include "core/common/container_scope.h"
#endif

#include "utils/log.h"

ArkNativeReference::ArkNativeReference(ArkNativeEngine* engine, NativeValue* value, uint32_t initialRefcount,
    NativeFinalize callback, void* data, void* hint)
    : engine_(engine),
      value_(Global<JSValueRef>(engine->GetEcmaVm(), JSValueRef::Undefined(engine->GetEcmaVm()))),
      refCount_(initialRefcount), callback_(callback), data_(data), hint_(hint)
{
    Global<JSValueRef> oldValue = *value;
    auto vm = engine->GetEcmaVm();
    Global<JSValueRef> newValue(vm, oldValue.ToLocal(vm));
    value_ = newValue;
    if (initialRefcount == 0) {
        value_.SetWeak();
    }

#ifdef ENABLE_CONTAINER_SCOPE
    scopeId_ = OHOS::Ace::ContainerScope::CurrentId();
#endif
}

ArkNativeReference::~ArkNativeReference()
{
    if (!value_.IsWeak()) {
        value_.SetWeak();
    }
    refCount_ = 0;
    FinalizeCallback();
}

uint32_t ArkNativeReference::Ref()
{
    ++refCount_;
    if (refCount_ == 1) {
        value_.ClearWeak();
    }
    return refCount_;
}

uint32_t ArkNativeReference::Unref()
{
    if (refCount_ == 0) {
        return refCount_;
    }
    --refCount_;
    if (refCount_ == 0) {
        value_.SetWeak();
        FinalizeCallback();
    }
    return refCount_;
}

NativeValue* ArkNativeReference::Get()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Local<JSValueRef> value = value_.ToLocal(vm);
#ifdef ENABLE_CONTAINER_SCOPE
    OHOS::Ace::ContainerScope containerScope(scopeId_);
#endif
    return ArkNativeEngine::ArkValueToNativeValue(engine_, value);
}

ArkNativeReference::operator NativeValue*()
{
    return Get();
}

void ArkNativeReference::FinalizeCallback()
{
    if (callback_ != nullptr) {
        callback_(engine_, data_, hint_);
    }
    callback_ = nullptr;
    data_ = nullptr;
    hint_ = nullptr;
}