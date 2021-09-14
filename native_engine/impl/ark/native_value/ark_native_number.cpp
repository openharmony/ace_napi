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

#include "ark_native_number.h"

#include <math.h>

using panda::NumberRef;
ArkNativeNumber::ArkNativeNumber(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeValue(engine, value) {}

ArkNativeNumber::ArkNativeNumber(ArkNativeEngine* engine, int32_t value)
    : ArkNativeNumber(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<NumberRef> object = NumberRef::New(vm, value);
    value_ = Global<NumberRef>(vm, object);
}

ArkNativeNumber::ArkNativeNumber(ArkNativeEngine* engine, uint32_t value)
    : ArkNativeNumber(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<NumberRef> object = NumberRef::New(vm, value);
    value_ = Global<NumberRef>(vm, object);
}

ArkNativeNumber::ArkNativeNumber(ArkNativeEngine* engine, int64_t value)
    : ArkNativeNumber(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<NumberRef> object = NumberRef::New(vm, value);
    value_ = Global<NumberRef>(vm, object);
}

ArkNativeNumber::ArkNativeNumber(ArkNativeEngine* engine, double value)
    : ArkNativeNumber(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<NumberRef> object = NumberRef::New(vm, value);
    value_ = Global<NumberRef>(vm, object);
}

ArkNativeNumber::~ArkNativeNumber() {}

void* ArkNativeNumber::GetInterface(int interfaceId)
{
    return (NativeNumber::INTERFACE_ID == interfaceId) ? (NativeNumber*)this : nullptr;
}

ArkNativeNumber::operator int32_t()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    return value->Int32Value(vm);
}

ArkNativeNumber::operator uint32_t()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    return value->Uint32Value(vm);
}

ArkNativeNumber::operator int64_t()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    return value->IntegerValue(vm);
}

ArkNativeNumber::operator double()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    Local<NumberRef> number(value.ToLocal(vm));
    return number->Value();
}
