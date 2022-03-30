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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_OBJECT_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_OBJECT_H

#include "jerryscript_native_value.h"
#include "native_engine/native_property.h"

class JerryScriptNativeObject : public JerryScriptNativeValue, public NativeObject {
public:
    explicit JerryScriptNativeObject(JerryScriptNativeEngine* engine);
    JerryScriptNativeObject(JerryScriptNativeEngine* engine, jerry_value_t value);
    ~JerryScriptNativeObject() override;

    void* GetInterface(int interfaceId) override;

    void SetNativePointer(void* pointer, NativeFinalize cb, void* hint) override;
    void* GetNativePointer() override;

    void AddFinalizer(void* pointer, NativeFinalize cb, void* hint) override;

    NativeValue* GetPropertyNames() override;

    NativeValue* GetPrototype() override;

    bool DefineProperty(NativePropertyDescriptor propertyDescriptor) override;

    bool SetProperty(NativeValue* key, NativeValue* value) override;
    NativeValue* GetProperty(NativeValue* key) override;
    bool HasProperty(NativeValue* key) override;
    bool DeleteProperty(NativeValue* key) override;

    bool SetProperty(const char* name, NativeValue* value) override;
    NativeValue* GetProperty(const char* name) override;
    bool HasProperty(const char* name) override;
    bool DeleteProperty(const char* name) override;

    bool SetPrivateProperty(const char* name, NativeValue* value) override;
    NativeValue* GetPrivateProperty(const char* name) override;
    bool HasPrivateProperty(const char* name) override;
    bool DeletePrivateProperty(const char* name) override;

    NativeValue* GetAllPropertyNames(
        napi_key_collection_mode keyMode, napi_key_filter keyFilter, napi_key_conversion keyConversion) override;

    bool AssociateTypeTag(NapiTypeTag* typeTag) override;
    bool CheckTypeTag(NapiTypeTag* typeTag) override;

    void Freeze() override;
    void Seal() override;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_OBJECT_H */
