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

#include "ark_native_boolean.h"

using panda::BooleanRef;
ArkNativeBoolean::ArkNativeBoolean(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeValue(engine, value) {}

ArkNativeBoolean::ArkNativeBoolean(ArkNativeEngine* engine, bool value)
    : ArkNativeBoolean(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<BooleanRef> object = BooleanRef::New(vm, value);
    value_ = Global<JSValueRef>(vm, object);
}

ArkNativeBoolean::~ArkNativeBoolean() {}

void* ArkNativeBoolean::GetInterface(int interfaceId)
{
    return (NativeBoolean::INTERFACE_ID == interfaceId) ? (NativeBoolean*)this : nullptr;
}

ArkNativeBoolean::operator bool()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<BooleanRef> value = value_;
    return value->Value();
}
