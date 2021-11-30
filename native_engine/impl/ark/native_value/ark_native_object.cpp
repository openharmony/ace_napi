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

#include "ark_native_object.h"

#include "ark_headers.h"
#include "ark_native_array.h"
#include "ark_native_external.h"
#include "ark_native_function.h"
#include "ark_native_reference.h"
#include "native_engine/native_property.h"

#include "utils/log.h"

using panda::ObjectRef;
using panda::StringRef;
using panda::NativePointerRef;
using panda::ArrayRef;
using panda::PropertyAttribute;
ArkNativeObject::ArkNativeObject(ArkNativeEngine* engine)
    : ArkNativeObject(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<ObjectRef> object = ObjectRef::New(vm);
    value_ = Global<ObjectRef>(vm, object);
}

ArkNativeObject::ArkNativeObject(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeValue(engine, value) {}

ArkNativeObject::~ArkNativeObject() {}

void* ArkNativeObject::GetInterface(int interfaceId)
{
    return (NativeObject::INTERFACE_ID == interfaceId) ? (NativeObject*)this : nullptr;
}

void ArkNativeObject::SetNativePointer(void* pointer, NativeFinalize cb, void* hint)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> value = value_;

    Local<StringRef> key = StringRef::NewFromUtf8(vm, "_napiwrapper");
    if (value->Has(vm, key) && pointer == nullptr) {
        Local<ObjectRef> wrapper = value->Get(vm, key);
        auto oldInfo = reinterpret_cast<NativeObjectInfo*>(wrapper->GetNativePointerField(0));
        // Try to remove native pointer from ArrayDataList
        wrapper->SetNativePointerField(0, nullptr, nullptr, nullptr);
        value->Delete(vm, key);
        delete oldInfo;
    } else {
        NativeObjectInfo* objInfo = NativeObjectInfo::CreateNewInstance();
        objInfo->engine = engine_;
        objInfo->nativeObject = pointer;
        objInfo->callback = cb;
        objInfo->hint = hint;

        Local<ObjectRef> object = ObjectRef::New(vm);
        object->SetNativePointerFieldCount(1);
        object->SetNativePointerField(0, objInfo,
            [](void* data, void* info) {
                auto externalInfo = reinterpret_cast<NativeObjectInfo*>(data);
                auto engine = externalInfo->engine;
                auto nativeObject = externalInfo->nativeObject;
                auto callback = externalInfo->callback;
                auto hint = externalInfo->hint;
                if (callback != nullptr) {
                    callback(engine, nativeObject, hint);
                }
                delete externalInfo;
            },
            nullptr);
        value->Set(vm, key, object);
    }
}

void* ArkNativeObject::GetNativePointer()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> value = value_;
    Local<StringRef> key = StringRef::NewFromUtf8(vm, "_napiwrapper");
    Local<JSValueRef> val = value->Get(vm, key);
    void* result = nullptr;
    if (val->IsObject()) {
        Local<ObjectRef> ext(val);
        result = ext->GetNativePointerField(0);
    }
    return (result != nullptr) ? reinterpret_cast<NativeObjectInfo*>(result)->nativeObject : nullptr;
}

NativeValue* ArkNativeObject::GetPropertyNames()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> val = value_;
    Local<ArrayRef> arrayVal = val->GetOwnEnumerablePropertyNames(vm);

    return new ArkNativeArray(engine_, arrayVal);
}

NativeValue* ArkNativeObject::GetPrototype()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    Local<JSValueRef> val = obj->GetPrototype(vm);

    return ArkNativeEngine::ArkValueToNativeValue(engine_, val);
}

bool ArkNativeObject::DefineProperty(NativePropertyDescriptor propertyDescriptor)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    bool result = false;
    Local<StringRef> propertyName = StringRef::NewFromUtf8(vm, propertyDescriptor.utf8name);

    bool writable = (propertyDescriptor.attributes & NATIVE_WRITABLE) != 0;
    bool enumable = (propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0;
    bool configable = (propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0;

    NativeScopeManager* scopeManager = engine_->GetScopeManager();
    if (scopeManager == nullptr) {
        HILOG_ERROR("scope manager is null");
        return false;
    }
    NativeScope* nativeScope = scopeManager->Open();
    if (propertyDescriptor.getter != nullptr || propertyDescriptor.setter != nullptr) {
        Global<JSValueRef> localGetter(vm, JSValueRef::Undefined(vm));
        Global<JSValueRef> localSetter(vm, JSValueRef::Undefined(vm));

        if (propertyDescriptor.getter != nullptr) {
            NativeValue* getter =
                new ArkNativeFunction(engine_, "getter", 0, propertyDescriptor.getter, propertyDescriptor.data);
            localGetter = *getter;
        }
        if (propertyDescriptor.setter != nullptr) {
            NativeValue* setter =
                new ArkNativeFunction(engine_, "setter", 0, propertyDescriptor.setter, propertyDescriptor.data);
            localSetter = *setter;
        }

        PropertyAttribute attr(JSValueRef::Undefined(engine_->GetEcmaVm()), false, enumable, configable);
        result = obj->SetAccessorProperty(vm, propertyName, localGetter.ToLocal(vm), localSetter.ToLocal(vm), attr);
    } else if (propertyDescriptor.method != nullptr) {
        NativeValue* cb = new ArkNativeFunction(engine_, propertyDescriptor.utf8name, 0, propertyDescriptor.method,
                                               propertyDescriptor.data);
        Global<JSValueRef> globalCb = *cb;
        PropertyAttribute attr(globalCb.ToLocal(vm), writable, enumable, configable);
        result = obj->DefineProperty(vm, propertyName, attr);
    } else {
        Global<JSValueRef> value = *(propertyDescriptor.value);

        PropertyAttribute attr(value.ToLocal(vm), writable, enumable, configable);
        result = obj->DefineProperty(vm, propertyName, attr);
    }
    scopeManager->Close(nativeScope);
    return result;
}

bool ArkNativeObject::SetProperty(NativeValue* key, NativeValue* value)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    Global<JSValueRef> k = *key;
    Global<JSValueRef> val = *value;

    return obj->Set(vm, k.ToLocal(vm), val.ToLocal(vm));
}

NativeValue* ArkNativeObject::GetProperty(NativeValue* key)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> k = *key;
    Global<ObjectRef> obj = value_;

    Local<JSValueRef> val = obj->Get(vm, k.ToLocal(vm));
    return ArkNativeEngine::ArkValueToNativeValue(engine_, val);
}

bool ArkNativeObject::HasProperty(NativeValue* key)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<ObjectRef> obj = value_;
    Global<JSValueRef> k = *key;

    return obj->Has(vm, k.ToLocal(vm));
}

bool ArkNativeObject::DeleteProperty(NativeValue* key)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<JSValueRef> k = *key;
    Global<ObjectRef> obj = value_;

    return obj->Delete(vm, k.ToLocal(vm));
}

bool ArkNativeObject::SetProperty(const char* name, NativeValue* value)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);

    Global<ObjectRef> obj = value_;
    Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
    Global<JSValueRef> val = *value;

    return obj->Set(vm, key, val.ToLocal(vm));
}

NativeValue* ArkNativeObject::GetProperty(const char* name)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);

    Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
    Global<ObjectRef> obj = value_;
    Local<JSValueRef> val = obj->Get(vm, key);
    return ArkNativeEngine::ArkValueToNativeValue(engine_, val);
}

bool ArkNativeObject::HasProperty(const char* name)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);

    Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
    Global<ObjectRef> obj = value_;

    return obj->Has(vm, key);
}

bool ArkNativeObject::DeleteProperty(const char* name)
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);

    Local<StringRef> key = StringRef::NewFromUtf8(vm, name);
    Global<ObjectRef> obj = value_;

    return obj->Delete(vm, key);
}

bool ArkNativeObject::SetPrivateProperty(const char* name, NativeValue* value)
{
    return false;
}

NativeValue* ArkNativeObject::GetPrivateProperty(const char* name)
{
    return nullptr;
}

bool ArkNativeObject::HasPrivateProperty(const char* name)
{
    return false;
}

bool ArkNativeObject::DeletePrivateProperty(const char* name)
{
    return false;
}

NativeValue* ArkNativeObject::GetAllPropertyNames(
    napi_key_collection_mode keyMode, napi_key_filter keyFilter, napi_key_conversion keyConversion)
{
    return nullptr;
}

bool ArkNativeObject::AssociateTypeTag(NapiTypeTag* typeTag)
{
    return true;
}

bool ArkNativeObject::CheckTypeTag(NapiTypeTag* typeTag)
{
    return true;
}

void ArkNativeObject::AddFinalizer(void* pointer, NativeFinalize cb, void* hint) {}

void ArkNativeObject::Freeze() {}

void ArkNativeObject::Seal() {}
