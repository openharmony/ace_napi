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

#include "ark_native_array.h"

using panda::ObjectRef;
using panda::ArrayRef;
ArkNativeArray::ArkNativeArray(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeObject(engine, value) {}

ArkNativeArray::ArkNativeArray(ArkNativeEngine* engine, uint32_t length)
    : ArkNativeArray(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<ArrayRef> object = ArrayRef::New(vm, length);
    value_ = Global<ArrayRef>(vm, object);
}

ArkNativeArray::~ArkNativeArray() {}

void* ArkNativeArray::GetInterface(int interfaceId)
{
    return (NativeArray::INTERFACE_ID == interfaceId) ? (NativeArray*)this
                                                      : ArkNativeObject::GetInterface(interfaceId);
}

bool ArkNativeArray::SetElement(uint32_t index, NativeValue* value)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    Global<JSValueRef> val = *value;
    return obj->Set(vm, index, val.ToLocal(vm));
}

NativeValue* ArkNativeArray::GetElement(uint32_t index)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    auto val = obj->Get(vm, index);
    return ArkNativeEngine::ArkValueToNativeValue(engine_, val);
}

bool ArkNativeArray::HasElement(uint32_t index)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    return obj->Has(vm, index);
}

bool ArkNativeArray::DeleteElement(uint32_t index)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    return obj->Delete(vm, index);
}

uint32_t ArkNativeArray::GetLength()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ArrayRef> obj = value_;
    return obj->Length(vm);
}
