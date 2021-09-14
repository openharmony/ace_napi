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

#include "ark_native_deferred.h"

using panda::Global;
using panda::Local;
using panda::JSValueRef;
ArkNativeDeferred::ArkNativeDeferred(ArkNativeEngine* engine, Local<PromiseCapabilityRef> deferred)
    : engine_(engine), deferred_(engine->GetEcmaVm(), deferred)
{
}

ArkNativeDeferred::~ArkNativeDeferred()
{
    // Addr of Global stored in ArkNativeDeferred should be released.
    deferred_.FreeGlobalHandleAddr();
}

void ArkNativeDeferred::Resolve(NativeValue* data)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = *data;
    deferred_->Resolve(vm, value.ToLocal(vm));
}

void ArkNativeDeferred::Reject(NativeValue* reason)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = *reason;
    deferred_->Reject(vm, value.ToLocal(vm));
}