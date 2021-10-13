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

#include "v8_native_object.h"

#include "native_engine/native_property.h"
#include "v8_headers.h"
#include "v8_native_array.h"
#include "v8_native_function.h"
#include "v8_native_reference.h"

V8NativeObject::V8NativeObject(V8NativeEngine* engine) : V8NativeObject(engine, v8::Object::New(engine->GetIsolate()))
{
}

V8NativeObject::V8NativeObject(V8NativeEngine* engine, v8::Local<v8::Value> value) : V8NativeValue(engine, value) {}

V8NativeObject::~V8NativeObject() {}

void* V8NativeObject::GetInterface(int interfaceId)
{
    return (NativeObject::INTERFACE_ID == interfaceId) ? (NativeObject*)this : nullptr;
}

void V8NativeObject::SetNativePointer(void* pointer, NativeFinalize cb, void* hint)
{
    v8::Local<v8::Object> value = value_;
    v8::Local<v8::String> key = v8::String::NewFromUtf8(engine_->GetIsolate(), "_napiwrapper").ToLocalChecked();
    bool has = value->Has(engine_->GetContext(), key).FromJust();
    auto context = engine_->GetContext();
    if (has && pointer == nullptr) {
        v8::Local<v8::External> val = value->Get(context, key).ToLocalChecked().As<v8::External>();
        value->Delete(context, key).FromJust();
        auto ref = reinterpret_cast<V8NativeReference*>(val->Value());
        delete ref;
    } else {
        void* ref = new V8NativeReference(engine_, this, 0, true, cb, pointer, hint);
        v8::Local<v8::External> val = v8::External::New(engine_->GetIsolate(), ref);
        value->Set(context, key, val).FromJust();
    }
}

void* V8NativeObject::GetNativePointer()
{
    v8::Local<v8::Object> value = value_;
    v8::Local<v8::String> key = v8::String::NewFromUtf8(engine_->GetIsolate(), "_napiwrapper").ToLocalChecked();
    v8::Local<v8::Value> val = value->Get(engine_->GetContext(), key).ToLocalChecked();
    void* result = nullptr;
    if (val->IsExternal()) {
        v8::Local<v8::External> ext = val.As<v8::External>();
        auto ref = reinterpret_cast<V8NativeReference*>(ext->Value());
        result = ref->GetData();
    }
    return result;
}

NativeValue* V8NativeObject::GetPropertyNames()
{
    v8::Local<v8::Object> val = value_;
    v8::Local<v8::Array> arrayVal =
        val->GetPropertyNames(
            engine_->GetContext(), v8::KeyCollectionMode::kIncludePrototypes,
            static_cast<v8::PropertyFilter>(v8::PropertyFilter::ONLY_ENUMERABLE | v8::PropertyFilter::SKIP_SYMBOLS),
            v8::IndexFilter::kIncludeIndices, v8::KeyConversionMode::kConvertToString).ToLocalChecked();

    return new V8NativeArray(engine_, arrayVal);
}

NativeValue* V8NativeObject::GetPrototype()
{
    v8::Local<v8::Object> obj = value_;

    v8::Local<v8::Value> val = obj->GetPrototype();
    return V8NativeEngine::V8ValueToNativeValue(engine_, val);
}

bool V8NativeObject::DefineProperty(NativePropertyDescriptor propertyDescriptor)
{
    v8::Local<v8::Object> obj = value_;

    bool result = false;

    v8::Local<v8::Name> propertyName =
        v8::String::NewFromUtf8(engine_->GetIsolate(), propertyDescriptor.utf8name, v8::NewStringType::kNormal)
            .ToLocalChecked();

    if (propertyDescriptor.getter != nullptr || propertyDescriptor.setter != nullptr) {
        v8::Local<v8::Value> localGetter;
        v8::Local<v8::Value> localSetter;

        if (propertyDescriptor.getter != nullptr) {
            NativeValue* getter =
                new V8NativeFunction(engine_, "getter", 0, propertyDescriptor.getter, propertyDescriptor.data);
            if (getter != nullptr) {
                localGetter = *getter;
            }
        }
        if (propertyDescriptor.setter != nullptr) {
            NativeValue* setter =
                new V8NativeFunction(engine_, "setter", 0, propertyDescriptor.setter, propertyDescriptor.data);
            if (setter != nullptr) {
                localSetter = *setter;
            }
        }

        v8::PropertyDescriptor descriptor(localGetter, localSetter);
        descriptor.set_enumerable((propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0);
        descriptor.set_configurable((propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0);

        result = obj->DefineProperty(engine_->GetContext(), propertyName, descriptor).FromMaybe(false);
    } else if (propertyDescriptor.method != nullptr) {
        NativeValue* cb = new V8NativeFunction(engine_, propertyDescriptor.utf8name, 0, propertyDescriptor.method,
                                               propertyDescriptor.data);
        if (cb != nullptr) {
            v8::PropertyDescriptor descriptor(*cb, (propertyDescriptor.attributes & NATIVE_WRITABLE) != 0);
            descriptor.set_enumerable((propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0);
            descriptor.set_configurable((propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0);
            result = obj->DefineProperty(engine_->GetContext(), propertyName, descriptor).FromMaybe(false);
        }
    } else {
        v8::Local<v8::Value> value = *(propertyDescriptor.value);

        v8::PropertyDescriptor descriptor(value, (propertyDescriptor.attributes & NATIVE_WRITABLE) != 0);
        descriptor.set_enumerable((propertyDescriptor.attributes & NATIVE_ENUMERABLE) != 0);
        descriptor.set_configurable((propertyDescriptor.attributes & NATIVE_CONFIGURABLE) != 0);

        result = obj->DefineProperty(engine_->GetContext(), propertyName, descriptor).FromMaybe(false);
    }
    return result;
}

bool V8NativeObject::SetProperty(NativeValue* key, NativeValue* value)
{
    v8::Local<v8::Object> obj = value_;

    v8::Local<v8::Value> k = *key;
    v8::Local<v8::Value> val = *value;

    v8::Maybe<bool> setMaybe = obj->Set(engine_->GetContext(), k, val);

    return setMaybe.FromMaybe(false);
}

NativeValue* V8NativeObject::GetProperty(NativeValue* key)
{
    v8::Local<v8::Value> k = *key;
    v8::Local<v8::Object> obj = value_;

    auto getMaybe = obj->Get(engine_->GetContext(), k);
    v8::Local<v8::Value> val = getMaybe.ToLocalChecked();
    return V8NativeEngine::V8ValueToNativeValue(engine_, val);
}

bool V8NativeObject::HasProperty(NativeValue* key)
{
    v8::Local<v8::Object> obj = value_;
    v8::Local<v8::Value> k = *key;

    v8::Maybe<bool> hasMaybe = obj->Has(engine_->GetContext(), k);

    return hasMaybe.FromMaybe(false);
}

bool V8NativeObject::DeleteProperty(NativeValue* key)
{
    v8::Local<v8::Value> k = *key;
    v8::Local<v8::Object> obj = value_;

    v8::Maybe<bool> deleteMaybe = obj->Delete(engine_->GetContext(), k);

    return deleteMaybe.FromMaybe(false);
}

bool V8NativeObject::SetProperty(const char* name, NativeValue* value)
{
    v8::Local<v8::Object> obj = value_;

    v8::Local<v8::Name> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();

    v8::Local<v8::Value> val = *value;

    v8::Maybe<bool> setMaybe = obj->Set(engine_->GetContext(), key, val);

    return setMaybe.FromMaybe(false);
}

NativeValue* V8NativeObject::GetProperty(const char* name)
{
    v8::Local<v8::Name> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();
    v8::Local<v8::Object> obj = value_;

    auto getMaybe = obj->Get(engine_->GetContext(), key);

    v8::Local<v8::Value> val = getMaybe.ToLocalChecked();

    return V8NativeEngine::V8ValueToNativeValue(engine_, val);
}

bool V8NativeObject::HasProperty(const char* name)
{
    v8::Local<v8::Name> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();
    v8::Local<v8::Object> obj = value_;

    v8::Maybe<bool> hasMaybe = obj->Has(engine_->GetContext(), key);
    return hasMaybe.FromMaybe(false);
}

bool V8NativeObject::DeleteProperty(const char* name)
{
    v8::Local<v8::Name> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();
    v8::Local<v8::Object> obj = value_;

    v8::Maybe<bool> deleteMaybe = obj->Delete(engine_->GetContext(), key);

    return deleteMaybe.FromMaybe(false);
}

bool V8NativeObject::SetPrivateProperty(const char* name, NativeValue* value)
{
    v8::Local<v8::String> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();
    v8::Local<v8::Value> val = *value;
    v8::Local<v8::Object> obj = value_;

    v8::Maybe<bool> setMaybe =
        obj->SetPrivate(engine_->GetContext(), v8::Private::New(engine_->GetIsolate(), key), val);

    return setMaybe.FromMaybe(false);
}

NativeValue* V8NativeObject::GetPrivateProperty(const char* name)
{
    v8::Local<v8::String> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();
    v8::Local<v8::Object> obj = value_;

    auto getMaybe = obj->GetPrivate(engine_->GetContext(), v8::Private::New(engine_->GetIsolate(), key));
    v8::Local<v8::Value> val = getMaybe.ToLocalChecked();

    return V8NativeEngine::V8ValueToNativeValue(engine_, val);
}

bool V8NativeObject::HasPrivateProperty(const char* name)
{
    v8::Local<v8::String> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();
    v8::Local<v8::Object> obj = value_;

    v8::Maybe<bool> hasMaybe = obj->HasPrivate(engine_->GetContext(), v8::Private::New(engine_->GetIsolate(), key));

    return hasMaybe.FromMaybe(false);
}

bool V8NativeObject::DeletePrivateProperty(const char* name)
{
    v8::Local<v8::String> key = v8::String::NewFromUtf8(engine_->GetIsolate(), name).ToLocalChecked();
    v8::Local<v8::Object> obj = value_;

    v8::Maybe<bool> deleteMaybe =
        obj->DeletePrivate(engine_->GetContext(), v8::Private::New(engine_->GetIsolate(), key));

    return deleteMaybe.FromMaybe(false);
}