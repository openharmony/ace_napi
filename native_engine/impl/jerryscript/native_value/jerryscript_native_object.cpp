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

#include "jerryscript_native_object.h"
#include "jerryscript_native_array.h"
#include "jerryscript_native_function.h"

#include "jerryscript-ext/handler.h"

struct JerryScriptNativeObjectInfo {
    NativeEngine* engine = nullptr;
    NativeFinalize cb = nullptr;
    void* data = nullptr;
    void* hint = nullptr;
};

namespace {
jerry_object_native_info_t g_freeCallback = {
    .free_cb = [](void* nativePointer) -> void {
        auto info = (JerryScriptNativeObjectInfo*)nativePointer;
        if (info != nullptr) {
            info->cb(info->engine, info->data, info->hint);
            delete info;
        }
    },
};
} // namespace

JerryScriptNativeObject::JerryScriptNativeObject(JerryScriptNativeEngine* engine)
    : JerryScriptNativeObject(engine, jerry_create_object())
{
}

JerryScriptNativeObject::JerryScriptNativeObject(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeValue(engine, value)
{
}

JerryScriptNativeObject::~JerryScriptNativeObject() {}

void JerryScriptNativeObject::SetNativePointer(void* pointer, NativeFinalize cb, void* hint)
{
    if (pointer == nullptr) {
        jerry_delete_object_native_pointer(value_, &g_freeCallback);
        return;
    }

    JerryScriptNativeObjectInfo* info = new JerryScriptNativeObjectInfo {
        .engine = engine_,
        .cb = cb,
        .data = pointer,
        .hint = hint,
    };

    jerry_set_object_native_pointer(value_, info, &g_freeCallback);
}

void* JerryScriptNativeObject::GetNativePointer()
{
    JerryScriptNativeObjectInfo* info = nullptr;
    jerry_get_object_native_pointer(value_, (void**)&info, &g_freeCallback);
    if (info != nullptr) {
        return info->data;
    } else {
        return nullptr;
    }
}

void* JerryScriptNativeObject::GetInterface(int interfaceId)
{
    return (NativeObject::INTERFACE_ID == interfaceId) ? (NativeObject*)this : nullptr;
}

NativeValue* JerryScriptNativeObject::GetPropertyNames()
{
    return new JerryScriptNativeArray(engine_, jerry_get_object_keys(value_));
}

NativeValue* JerryScriptNativeObject::GetPrototype()
{
    return JerryScriptNativeEngine::JerryValueToNativeValue(engine_, jerry_get_prototype(value_));
}

bool JerryScriptNativeObject::DefineProperty(NativePropertyDescriptor propertyDescriptor)
{
    jerry_value_t propName = jerry_create_string_from_utf8((const unsigned char*)propertyDescriptor.utf8name);
    jerry_property_descriptor_t prop = { 0 };

    jerry_init_property_descriptor_fields(&prop);

    prop.is_writable_defined = propertyDescriptor.attributes & NATIVE_WRITABLE;
    prop.is_writable = propertyDescriptor.attributes & NATIVE_WRITABLE;

    prop.is_enumerable_defined = propertyDescriptor.attributes & NATIVE_ENUMERABLE;
    prop.is_enumerable = propertyDescriptor.attributes & NATIVE_ENUMERABLE;

    prop.is_configurable_defined = propertyDescriptor.attributes & NATIVE_CONFIGURABLE;
    prop.is_configurable = propertyDescriptor.attributes & NATIVE_CONFIGURABLE;

    if (propertyDescriptor.value != nullptr) {
        prop.value = *propertyDescriptor.value;
        prop.is_value_defined = true;
    }

    if (propertyDescriptor.method != nullptr) {
        prop.value = *(new JerryScriptNativeFunction(engine_, propertyDescriptor.utf8name, propertyDescriptor.method,
                                                     propertyDescriptor.data));
        prop.is_value_defined = true;
    }

    if (propertyDescriptor.getter != nullptr) {
        prop.getter =
            *(new JerryScriptNativeFunction(engine_, "getter", propertyDescriptor.getter, propertyDescriptor.data));
        prop.is_get_defined = true;
        prop.is_writable_defined = true;
    }
    if (propertyDescriptor.setter != nullptr) {
        prop.setter =
            *(new JerryScriptNativeFunction(engine_, "setter", propertyDescriptor.setter, propertyDescriptor.data));
        prop.is_set_defined = true;
        prop.is_writable_defined = true;
    }

    jerry_value_t returnValue = jerry_define_own_property(value_, propName, &prop);
    jerry_release_value(returnValue);
    jerry_release_value(propName);

    return true;
}

bool JerryScriptNativeObject::SetProperty(NativeValue* key, NativeValue* value)
{
    jerry_value_t returnValue = jerry_set_property(value_, *key, *value);
    jerry_release_value(returnValue);
    return true;
}

NativeValue* JerryScriptNativeObject::GetProperty(NativeValue* key)
{
    jerry_value_t returnValue = jerry_get_property(value_, *key);
    return JerryScriptNativeEngine::JerryValueToNativeValue(engine_, returnValue);
}

bool JerryScriptNativeObject::HasProperty(NativeValue* key)
{
    jerry_value_t returnValue = jerry_has_property(value_, *key);
    bool result = jerry_value_to_boolean(returnValue);
    jerry_release_value(returnValue);
    return result;
}

bool JerryScriptNativeObject::DeleteProperty(NativeValue* key)
{
    return jerry_delete_property(value_, *key);
}

bool JerryScriptNativeObject::SetProperty(const char* name, NativeValue* value)
{
    jerry_value_t returnValue = jerryx_set_property_str(value_, name, *value);
    jerry_release_value(returnValue);
    return true;
}

NativeValue* JerryScriptNativeObject::GetProperty(const char* name)
{
    jerry_value_t returnValue = jerryx_get_property_str(value_, name);
    return JerryScriptNativeEngine::JerryValueToNativeValue(engine_, returnValue);
}

bool JerryScriptNativeObject::HasProperty(const char* name)
{
    bool result = jerryx_has_property_str(value_, name);
    return result;
}

bool JerryScriptNativeObject::DeleteProperty(const char* name)
{
    jerry_value_t key = jerry_create_string_from_utf8((const unsigned char*)name);
    bool result = jerry_delete_property(value_, key);
    jerry_release_value(key);
    return result;
}

bool JerryScriptNativeObject::SetPrivateProperty(const char* name, NativeValue* value)
{
    bool result = false;
    jerry_value_t key = jerry_create_string_from_utf8((const unsigned char*)name);
    result = jerry_set_property(value_, key, *value);
    jerry_release_value(key);
    return result;
}

NativeValue* JerryScriptNativeObject::GetPrivateProperty(const char* name)
{
    jerry_value_t result = 0;
    jerry_value_t key = jerry_create_string_from_utf8((const unsigned char*)name);
    result = jerry_get_property(value_, key);
    jerry_release_value(key);
    return JerryScriptNativeEngine::JerryValueToNativeValue(engine_, result);
}

bool JerryScriptNativeObject::HasPrivateProperty(const char* name)
{
    jerry_value_t key = jerry_create_string_from_utf8((const unsigned char*)name);
    jerry_value_t returnValue = jerry_has_property(value_, key);
    bool result = jerry_value_to_boolean(returnValue);
    jerry_release_value(returnValue);
    jerry_release_value(key);
    return result;
}

bool JerryScriptNativeObject::DeletePrivateProperty(const char* name)
{
    bool result = false;
    jerry_value_t key = jerry_create_string_from_utf8((const unsigned char*)name);
    result = jerry_delete_property(value_, key);
    jerry_release_value(key);
    return result;
}