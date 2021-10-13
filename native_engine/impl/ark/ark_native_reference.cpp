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

#include "utils/log.h"

ArkNativeReference::ArkNativeReference(ArkNativeEngine* engine, NativeValue* value, uint32_t initialRefcount)
    : engine_(engine),
      value_(Global<JSValueRef>(engine->GetEcmaVm(), JSValueRef::Undefined(engine->GetEcmaVm()))),
      refCount_(initialRefcount)
{
    ASSERT(initialRefcount != 0);
    Global<JSValueRef> oldValue = *value;
    auto vm = engine->GetEcmaVm();
    Global<JSValueRef> newValue(vm, oldValue.ToLocal(vm));
    value_ = newValue;
}

ArkNativeReference::~ArkNativeReference()
{
    if (refCount_ != 0) {
        // Addr of Global stored in ArkNativeReference should be released.
        refCount_ = 0;
        value_.FreeGlobalHandleAddr();
    }
}

uint32_t ArkNativeReference::Ref()
{
    if (refCount_ != 0) {
        ++refCount_;
    }
    return refCount_;
}

uint32_t ArkNativeReference::Unref()
{
    if (refCount_ == 1) {
        refCount_ = 0;
        value_.FreeGlobalHandleAddr();
    } else if (refCount_ > 0) {
        --refCount_;
    }
    return refCount_;
}

NativeValue* ArkNativeReference::Get()
{
    if (refCount_ == 0) {
        return nullptr;
    }
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Local<JSValueRef> value = value_.ToLocal(vm);
    return ArkNativeEngine::ArkValueToNativeValue(engine_, value);
}

ArkNativeReference::operator NativeValue*()
{
    return Get();
}