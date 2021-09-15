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

#include "ark_native_value.h"

ArkNativeValue::ArkNativeValue(ArkNativeEngine* engine, Local<JSValueRef> value)
{
    engine_ = engine;
    Global<JSValueRef> globalValue(engine->GetEcmaVm(), value);
    value_ = globalValue;

    NativeScopeManager* scopeManager = engine_->GetScopeManager();
    if (scopeManager != nullptr) {
        scopeManager->CreateHandle(this);
    }
}

ArkNativeValue::~ArkNativeValue()
{
    // Addr of Global stored in ArkNativeValue should be released.
    Global<JSValueRef> oldValue = value_;
    oldValue.FreeGlobalHandleAddr();
}

void* ArkNativeValue::GetInterface(int interfaceId)
{
    return nullptr;
}

void ArkNativeValue::UpdateValue(Local<JSValueRef> value)
{
    auto vm = engine_->GetEcmaVm();
    Global<JSValueRef> oldValue = value_;
    oldValue.FreeGlobalHandleAddr();

    Global<JSValueRef> newValue(vm, value);
    value_ = newValue;
}

NativeValueType ArkNativeValue::TypeOf()
{
    Global<JSValueRef> value = value_;
    NativeValueType result;

    // TODO: optimize it
    if (value->IsNumber()) {
        result = NATIVE_NUMBER;
    } else if (value->IsString()) {
        result = NATIVE_STRING;
    } else if (value->IsFunction()) {
        result = NATIVE_FUNCTION;
    } else if (value->IsNativeObject() || value->IsNativePointer()) {
        result = NATIVE_EXTERNAL;
    } else if (value->IsNull()) {
        result = NATIVE_NULL;
    } else if (value->IsBoolean()) {
        result = NATIVE_BOOLEAN;
    } else if (value->IsUndefined()) {
        result = NATIVE_UNDEFINED;
    } else if (value->IsSymbol()) {
        result = NATIVE_SYMBOL;
    } else if (value->IsObject()) {
        result = NATIVE_OBJECT;
    } else {
        result = NATIVE_UNDEFINED;
    }

    return result;
}

bool ArkNativeValue::InstanceOf(NativeValue* obj)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    Global<JSValueRef> object = *obj;
    return value->InstanceOf(vm, object.ToLocal(vm));
}

bool ArkNativeValue::IsArray()
{
    Global<JSValueRef> value = value_;
    return value->IsArray(engine_->GetEcmaVm());
}

bool ArkNativeValue::IsArrayBuffer()
{
    Global<JSValueRef> value = value_;
    return value->IsArrayBuffer();
}

bool ArkNativeValue::IsDate()
{
    Global<JSValueRef> value = value_;
    return value->IsDate();
}

bool ArkNativeValue::IsError()
{
    Global<JSValueRef> value = value_;
    return value->IsError();
}

bool ArkNativeValue::IsTypedArray()
{
    Global<JSValueRef> value = value_;
    return value->IsTypedArray();
}

bool ArkNativeValue::IsDataView()
{
    Global<JSValueRef> value = value_;
    return value->IsDataView();
}

bool ArkNativeValue::IsPromise()
{
    Global<JSValueRef> value = value_;
    return value->IsPromise();
}

bool ArkNativeValue::IsCallable()
{
    Global<JSValueRef> value = value_;
    return value->IsFunction();
}

NativeValue* ArkNativeValue::ToBoolean()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    return ArkNativeEngine::ArkValueToNativeValue(engine_, value->ToBoolean(vm));
}

NativeValue* ArkNativeValue::ToNumber()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    return ArkNativeEngine::ArkValueToNativeValue(engine_, value->ToNumber(vm));
}

NativeValue* ArkNativeValue::ToString()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    return ArkNativeEngine::ArkValueToNativeValue(engine_, value->ToString(vm));
}

NativeValue* ArkNativeValue::ToObject()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value = value_;
    return ArkNativeEngine::ArkValueToNativeValue(engine_, value->ToObject(vm));
}

bool ArkNativeValue::StrictEquals(NativeValue* value)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> value1 = value_;
    Global<JSValueRef> value2 = *value;
    return value1->IsStrictEquals(vm, value2.ToLocal(vm));
}