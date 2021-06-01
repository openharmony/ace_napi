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

#include "native_api_internal.h"

#include "native_engine/native_property.h"
#include "native_engine/native_value.h"

#include "utils/log.h"

NAPI_EXTERN napi_status napi_get_last_error_info(napi_env env, const napi_extended_error_info** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    *result = reinterpret_cast<napi_extended_error_info*>(engine->GetLastError());

    return napi_clear_last_error(env);
}

// Getters for defined singletons
NAPI_EXTERN napi_status napi_get_undefined(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateUndefined();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_null(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateNull();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_global(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->GetGlobal();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_boolean(napi_env env, bool value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateBoolean(value);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

// Methods to create Primitive types/Objects
NAPI_EXTERN napi_status napi_create_object(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateObject();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateArray(0);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_array_with_length(napi_env env, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateArray(length);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_double(napi_env env, double value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateNumber(value);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int32(napi_env env, int32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateNumber(value);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_uint32(napi_env env, uint32_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateNumber(value);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_int64(napi_env env, int64_t value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateNumber(value);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_latin1(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateString(str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_string_utf8(napi_env env, const char* str, size_t length, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, str);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreateString(str, (length == NAPI_AUTO_LENGTH) ? strlen(str) : length);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_symbol(napi_env env, napi_value description, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, description);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto descriptionValue = reinterpret_cast<NativeValue*>(description);

    RETURN_STATUS_IF_FALSE(env, (descriptionValue->TypeOf() == NATIVE_STRING), napi_invalid_arg);

    auto resultValue = engine->CreateSymbol(descriptionValue);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_function(napi_env env,
                                             const char* utf8name,
                                             size_t length,
                                             napi_callback cb,
                                             void* data,
                                             napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, cb);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NativeCallback>(cb);

    auto resultValue =
        engine->CreateFunction(utf8name, (length == NAPI_AUTO_LENGTH) ? strlen(utf8name) : length, callback, data);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto codeValue = reinterpret_cast<NativeValue*>(code);
    auto msgValue = reinterpret_cast<NativeValue*>(msg);

    if (codeValue != nullptr) {
        RETURN_STATUS_IF_FALSE(env, codeValue->TypeOf() == NATIVE_STRING, napi_invalid_arg);
    }
    RETURN_STATUS_IF_FALSE(env, msgValue->TypeOf() == NATIVE_STRING, napi_invalid_arg);

    auto resultValue = engine->CreateError(codeValue, msgValue);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_type_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto codeValue = reinterpret_cast<NativeValue*>(code);
    auto msgValue = reinterpret_cast<NativeValue*>(msg);

    if (codeValue != nullptr) {
        RETURN_STATUS_IF_FALSE(env, codeValue->TypeOf() == NATIVE_STRING, napi_invalid_arg);
    }
    RETURN_STATUS_IF_FALSE(env, msgValue->TypeOf() == NATIVE_STRING, napi_invalid_arg);

    auto resultValue = engine->CreateError(codeValue, msgValue);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_range_error(napi_env env, napi_value code, napi_value msg, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto codeValue = reinterpret_cast<NativeValue*>(code);
    auto msgValue = reinterpret_cast<NativeValue*>(msg);

    if (codeValue != nullptr) {
        RETURN_STATUS_IF_FALSE(env, codeValue->TypeOf() == NATIVE_STRING, napi_invalid_arg);
    }
    RETURN_STATUS_IF_FALSE(env, msgValue->TypeOf() == NATIVE_STRING, napi_invalid_arg);

    auto resultValue = engine->CreateError(codeValue, msgValue);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

// Methods to get the native napi_value from Primitive type
NAPI_EXTERN napi_status napi_typeof(napi_env env, napi_value value, napi_valuetype* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    *result = (napi_valuetype)nativeValue->TypeOf();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_double(napi_env env, napi_value value, double* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_NUMBER, napi_number_expected);

    *result = *reinterpret_cast<NativeNumber*>(nativeValue->GetInterface(NativeNumber::INTERFACE_ID));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_int32(napi_env env, napi_value value, int32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_NUMBER, napi_number_expected);

    *result = *reinterpret_cast<NativeNumber*>(nativeValue->GetInterface(NativeNumber::INTERFACE_ID));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_uint32(napi_env env, napi_value value, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_NUMBER, napi_number_expected);

    *result = *reinterpret_cast<NativeNumber*>(nativeValue->GetInterface(NativeNumber::INTERFACE_ID));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_int64(napi_env env, napi_value value, int64_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_NUMBER, napi_number_expected);

    *result = *reinterpret_cast<NativeNumber*>(nativeValue->GetInterface(NativeNumber::INTERFACE_ID));
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_BOOLEAN, napi_boolean_expected);

    *result = *reinterpret_cast<NativeBoolean*>(nativeValue->GetInterface(NativeBoolean::INTERFACE_ID));
    return napi_clear_last_error(env);
}

// Copies LATIN-1 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status
napi_get_value_string_latin1(napi_env env, napi_value value, char* buf, size_t bufsize, size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_STRING, napi_string_expected);

    auto nativeString = reinterpret_cast<NativeString*>(nativeValue->GetInterface(NativeString::INTERFACE_ID));

    nativeString->GetCString(buf, bufsize, result);
    return napi_clear_last_error(env);
}

// Copies UTF-8 encoded bytes from a string into a buffer.
NAPI_EXTERN napi_status
napi_get_value_string_utf8(napi_env env, napi_value value, char* buf, size_t bufsize, size_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_STRING, napi_string_expected);

    auto nativeString = reinterpret_cast<NativeString*>(nativeValue->GetInterface(NativeString::INTERFACE_ID));

    nativeString->GetCString(buf, bufsize, result);
    return napi_clear_last_error(env);
}

// Methods to coerce values
// These APIs may execute user scripts
NAPI_EXTERN napi_status napi_coerce_to_bool(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    auto resultValue = nativeValue->ToBoolean();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_number(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    auto resultValue = nativeValue->ToNumber();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_object(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    auto resultValue = nativeValue->ToObject();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_coerce_to_string(napi_env env, napi_value value, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    auto resultValue = nativeValue->ToString();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

// Methods to work with Objects
NAPI_EXTERN napi_status napi_get_prototype(napi_env env, napi_value object, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_FUNCTION, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    auto resultValue = nativeObject->GetPrototype();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_property_names(napi_env env, napi_value object, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    auto resultValue = nativeObject->GetPropertyNames();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_property(napi_env env, napi_value object, napi_value key, napi_value value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, value);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    auto propKey = reinterpret_cast<NativeValue*>(key);
    auto propValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    nativeObject->SetProperty(propKey, propValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    auto propKey = reinterpret_cast<NativeValue*>(key);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->HasProperty(propKey);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_property(napi_env env, napi_value object, napi_value key, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    auto resultValue = nativeObject->GetProperty((NativeValue*)key);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_delete_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    auto propKey = reinterpret_cast<NativeValue*>(key);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->DeleteProperty(propKey);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_own_property(napi_env env, napi_value object, napi_value key, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    auto propKey = reinterpret_cast<NativeValue*>(key);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->HasProperty(propKey);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_named_property(napi_env env, napi_value object, const char* utf8name, napi_value value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, value);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    auto propValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    nativeObject->SetProperty(utf8name, propValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_named_property(napi_env env, napi_value object, const char* utf8name, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->HasProperty(utf8name);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_named_property(napi_env env,
                                                napi_value object,
                                                const char* utf8name,
                                                napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    auto resultValue = nativeObject->GetProperty(utf8name);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_set_element(napi_env env, napi_value object, uint32_t index, napi_value value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, value);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    auto elementValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArray(), napi_array_expected);

    auto nativeArray = reinterpret_cast<NativeArray*>(nativeValue->GetInterface(NativeArray::INTERFACE_ID));

    nativeArray->SetElement(index, elementValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_has_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArray(), napi_array_expected);

    auto nativeArray = reinterpret_cast<NativeArray*>(nativeValue->GetInterface(NativeArray::INTERFACE_ID));

    *result = nativeArray->HasElement(index);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_element(napi_env env, napi_value object, uint32_t index, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArray(), napi_array_expected);

    auto nativeArray = reinterpret_cast<NativeArray*>(nativeValue->GetInterface(NativeArray::INTERFACE_ID));

    auto resultValue = nativeArray->GetElement(index);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_delete_element(napi_env env, napi_value object, uint32_t index, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArray(), napi_array_expected);

    auto nativeArray = reinterpret_cast<NativeArray*>(nativeValue->GetInterface(NativeArray::INTERFACE_ID));

    *result = nativeArray->DeleteElement(index);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_define_properties(napi_env env,
                                               napi_value object,
                                               size_t property_count,
                                               const napi_property_descriptor* properties)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, properties);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    NativeObject* nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    for (size_t i = 0; i < property_count; i++) {
        NativePropertyDescriptor property;

        property.utf8name = properties[i].utf8name;
        property.name = reinterpret_cast<NativeValue*>(properties[i].name);
        property.method = reinterpret_cast<NativeCallback>(properties[i].method);
        property.getter = reinterpret_cast<NativeCallback>(properties[i].getter);
        property.setter = reinterpret_cast<NativeCallback>(properties[i].setter);
        property.value = reinterpret_cast<NativeValue*>(properties[i].value);
        property.attributes = (uint32_t)properties[i].attributes;
        property.data = properties[i].data;

        nativeObject->DefineProperty(property);
    }

    return napi_clear_last_error(env);
}

// Methods to work with Arrays
NAPI_EXTERN napi_status napi_is_array(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    *result = nativeValue->IsArray();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_array_length(napi_env env, napi_value value, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArray(), napi_array_expected);

    auto nativeArray = reinterpret_cast<NativeArray*>(nativeValue->GetInterface(NativeArray::INTERFACE_ID));

    *result = nativeArray->GetLength();

    return napi_clear_last_error(env);
}

// Methods to compare values
NAPI_EXTERN napi_status napi_strict_equals(napi_env env, napi_value lhs, napi_value rhs, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, lhs);
    CHECK_ARG(env, rhs);
    CHECK_ARG(env, result);

    auto nativeLhs = reinterpret_cast<NativeValue*>(lhs);
    auto nativeRhs = reinterpret_cast<NativeValue*>(rhs);

    *result = *nativeLhs == nativeRhs;
    return napi_clear_last_error(env);
}

// Methods to work with Functions
NAPI_EXTERN napi_status napi_call_function(napi_env env,
                                           napi_value recv,
                                           napi_value func,
                                           size_t argc,
                                           const napi_value* argv,
                                           napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, func);
    if (argc > 0) {
        CHECK_ARG(env, argv);
    }
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeRecv = reinterpret_cast<NativeValue*>(recv);
    auto nativeFunc = reinterpret_cast<NativeValue*>(func);
    auto nativeArgv = reinterpret_cast<NativeValue* const*>(argv);

    RETURN_STATUS_IF_FALSE(env, nativeFunc->TypeOf() == NATIVE_FUNCTION, napi_function_expected);

    HILOG_INFO("engine: %{public}p, nativeRecv: %{public}p, nativeFunc: %{public}p, nativeArgv: %{public}p",
        engine, nativeRecv, nativeFunc, nativeArgv);

    auto resultValue = engine->CallFunction(nativeRecv, nativeFunc, nativeArgv, argc);

    HILOG_INFO("engine: %{public}p, nativeRecv: %{public}p, nativeFunc: %{public}p, nativeArgv: %{public}p, "
        "resultValue: %{public}p", engine, nativeRecv, nativeFunc, nativeArgv, resultValue);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status
napi_new_instance(napi_env env, napi_value constructor, size_t argc, const napi_value* argv, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, constructor);
    if (argc > 0) {
        CHECK_ARG(env, argv);
    }
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeConstructor = reinterpret_cast<NativeValue*>(constructor);
    auto nativeArgv = reinterpret_cast<NativeValue* const*>(argv);

    RETURN_STATUS_IF_FALSE(env, nativeConstructor->TypeOf() == NATIVE_FUNCTION, napi_function_expected);

    auto resultValue = engine->CreateInstance(nativeConstructor, nativeArgv, argc);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_instanceof(napi_env env, napi_value object, napi_value constructor, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, object);
    CHECK_ARG(env, constructor);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(object);
    auto nativeConstructor = reinterpret_cast<NativeValue*>(constructor);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    RETURN_STATUS_IF_FALSE(env, nativeConstructor->TypeOf() == NATIVE_FUNCTION, napi_function_expected);

    *result = nativeValue->InstanceOf(nativeConstructor);
    return napi_clear_last_error(env);
}

// Methods to work with napi_callbacks
// Gets all callback info in a single call. (Ugly, but faster.)
NAPI_EXTERN napi_status napi_get_cb_info(napi_env env,              // [in] NAPI environment handle
                                         napi_callback_info cbinfo, // [in] Opaque callback-info handle
                                         size_t* argc,         // [in-out] Specifies the size of the provided argv array
                                                               // and receives the actual count of args.
                                         napi_value* argv,     // [out] Array of values
                                         napi_value* this_arg, // [out] Receives the JS 'this' arg for the call
                                         void** data)          // [out] Receives the data pointer for the callback.
{
    CHECK_ENV(env);
    CHECK_ARG(env, cbinfo);

    auto info = reinterpret_cast<NativeCallbackInfo*>(cbinfo);

    if ((argc != nullptr) && (argv != nullptr)) {
        size_t i = 0;
        for (i = 0; (i < *argc) && (i < info->argc); i++) {
            argv[i] = reinterpret_cast<napi_value>(info->argv[i]);
        }
        *argc = i;
    }

    if (argc != nullptr) {
        *argc = info->argc;
    }

    if (this_arg != nullptr) {
        *this_arg = reinterpret_cast<napi_value>(info->thisVar);
    }

    if (data != nullptr && info->functionInfo != nullptr) {
        *data = info->functionInfo->data;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_new_target(napi_env env, napi_callback_info cbinfo, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, cbinfo);
    CHECK_ARG(env, result);

    auto info = reinterpret_cast<NativeCallbackInfo*>(cbinfo);

    if (info->thisVar->InstanceOf(info->function)) {
        *result = reinterpret_cast<napi_value>(info->function);
    } else {
        *result = nullptr;
    }

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_define_class(napi_env env,
                                          const char* utf8name,
                                          size_t length,
                                          napi_callback constructor,
                                          void* data,
                                          size_t property_count,
                                          const napi_property_descriptor* properties,
                                          napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, utf8name);
    CHECK_ARG(env, constructor);
    if (property_count > 0) {
        CHECK_ARG(env, properties);
    }
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NativeCallback>(constructor);
    auto nativeProperties = reinterpret_cast<const NativePropertyDescriptor*>(properties);

    auto resultValue = engine->DefineClass(utf8name, callback, data, nativeProperties, property_count);

    *result = reinterpret_cast<napi_value>(resultValue);

    return napi_clear_last_error(env);
}

// Methods to work with external data objects
NAPI_EXTERN napi_status napi_wrap(napi_env env,
                                  napi_value js_object,
                                  void* native_object,
                                  napi_finalize finalize_cb,
                                  void* finalize_hint,
                                  napi_ref* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, native_object);
    CHECK_ARG(env, finalize_cb);

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    nativeObject->SetNativePointer(native_object, callback, finalize_hint);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_unwrap(napi_env env, napi_value js_object, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->GetNativePointer();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_remove_wrap(napi_env env, napi_value js_object, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, js_object);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(js_object);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_OBJECT, napi_object_expected);

    auto nativeObject = reinterpret_cast<NativeObject*>(nativeValue->GetInterface(NativeObject::INTERFACE_ID));

    *result = nativeObject->GetNativePointer();
    nativeObject->SetNativePointer(nullptr, nullptr, nullptr);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status
napi_create_external(napi_env env, void* data, napi_finalize finalize_cb, void* finalize_hint, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, finalize_cb);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);

    auto resultValue = engine->CreateExternal(data, callback, finalize_hint);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_value_external(napi_env env, napi_value value, void** result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    RETURN_STATUS_IF_FALSE(env, nativeValue->TypeOf() == NATIVE_EXTERNAL, napi_object_expected);

    auto external = reinterpret_cast<NativeExternal*>(nativeValue->GetInterface(NativeExternal::INTERFACE_ID));

    *result = *external;
    return napi_clear_last_error(env);
}

// Methods to control object lifespan
// Set initial_refcount to 0 for a weak reference, >0 for a strong reference.
NAPI_EXTERN napi_status napi_create_reference(napi_env env,
                                              napi_value value,
                                              uint32_t initial_refcount,
                                              napi_ref* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    auto reference = engine->CreateReference(nativeValue, initial_refcount);

    *result = reinterpret_cast<napi_ref>(reference);
    return napi_clear_last_error(env);
}

// Deletes a reference. The referenced value is released, and may
// be GC'd unless there are other references to it.
NAPI_EXTERN napi_status napi_delete_reference(napi_env env, napi_ref ref)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);

    auto reference = reinterpret_cast<NativeReference*>(ref);

    delete reference;
    reference = nullptr;

    return napi_clear_last_error(env);
}

// Increments the reference count, optionally returning the resulting count.
// After this call the  reference will be a strong reference because its
// refcount is >0, and the referenced object is effectively "pinned".
// Calling this when the refcount is 0 and the object is unavailable
// results in an error.
NAPI_EXTERN napi_status napi_reference_ref(napi_env env, napi_ref ref, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);
    CHECK_ARG(env, result);

    auto reference = reinterpret_cast<NativeReference*>(ref);

    *result = reference->Ref();

    return napi_clear_last_error(env);
}

// Decrements the reference count, optionally returning the resulting count.
// If the result is 0 the reference is now weak and the object may be GC'd
// at any time if there are no other references. Calling this when the
// refcount is already 0 results in an error.
NAPI_EXTERN napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);
    CHECK_ARG(env, result);

    auto reference = reinterpret_cast<NativeReference*>(ref);

    *result = reference->Unref();

    return napi_clear_last_error(env);
}

// Attempts to get a referenced value. If the reference is weak,
// the value might no longer be available, in that case the call
// is still successful but the result is nullptr.
NAPI_EXTERN napi_status napi_get_reference_value(napi_env env, napi_ref ref, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, ref);
    CHECK_ARG(env, result);

    auto reference = reinterpret_cast<NativeReference*>(ref);

    auto resultValue = reference->Get();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }
    *result = reinterpret_cast<napi_handle_scope>(scopeManager->Open());
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeScope = reinterpret_cast<NativeScope*>(scope);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }
    scopeManager->Close(nativeScope);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_open_escapable_handle_scope(napi_env env, napi_escapable_handle_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto scopeManager = engine->GetScopeManager();

    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }

    auto NativeScope = scopeManager->OpenEscape();

    *result = reinterpret_cast<napi_escapable_handle_scope>(NativeScope);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_close_escapable_handle_scope(napi_env env, napi_escapable_handle_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeScope = reinterpret_cast<NativeScope*>(scope);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }
    scopeManager->CloseEscape(nativeScope);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_escape_handle(napi_env env,
                                           napi_escapable_handle_scope scope,
                                           napi_value escapee,
                                           napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);
    CHECK_ARG(env, escapee);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeScope = reinterpret_cast<NativeScope*>(scope);
    auto escapeeValue = reinterpret_cast<NativeValue*>(escapee);

    auto scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        return napi_set_last_error(env, napi_generic_failure);
    }

    auto resultValue = scopeManager->Escape(nativeScope, escapeeValue);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

// Methods to support error handling
NAPI_EXTERN napi_status napi_throw(napi_env env, napi_value error)
{
    CHECK_ENV(env);
    CHECK_ARG(env, error);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto nativeValue = reinterpret_cast<NativeValue*>(error);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsError(), napi_invalid_arg);

    engine->Throw(nativeValue);
    return napi_generic_failure;
}

NAPI_EXTERN napi_status napi_throw_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    engine->Throw(NativeErrorType::NATIVE_COMMON_ERROR, code, msg);
    return napi_generic_failure;
}

NAPI_EXTERN napi_status napi_throw_type_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    engine->Throw(NativeErrorType::NATIVE_TYPE_ERROR, code, msg);
    return napi_generic_failure;
}

NAPI_EXTERN napi_status napi_throw_range_error(napi_env env, const char* code, const char* msg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, msg);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    engine->Throw(NativeErrorType::NATIVE_RANGE_ERROR, code, msg);
    return napi_generic_failure;
}

NAPI_EXTERN napi_status napi_is_error(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    *result = nativeValue->IsError();
    return napi_clear_last_error(env);
}

// Methods to support catching exceptions
NAPI_EXTERN napi_status napi_is_exception_pending(napi_env env, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    *result = engine->IsExceptionPending();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_and_clear_last_exception(napi_env env, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    auto resultValue = engine->GetAndClearLastException();

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

// Methods to work with array buffers and typed arrays
NAPI_EXTERN napi_status napi_is_arraybuffer(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    *result = nativeValue->IsArrayBuffer();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_arraybuffer(napi_env env, size_t byte_length, void** data, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, data);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);

    auto resultValue = engine->CreateArrayBuffer(data, byte_length);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_external_arraybuffer(napi_env env,
                                                         void* external_data,
                                                         size_t byte_length,
                                                         napi_finalize finalize_cb,
                                                         void* finalize_hint,
                                                         napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, external_data);
    CHECK_ARG(env, finalize_cb);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callback = reinterpret_cast<NativeFinalize>(finalize_cb);

    auto resultValue = engine->CreateArrayBufferExternal(external_data, byte_length, callback, finalize_hint);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_arraybuffer_info(napi_env env,
                                                  napi_value arraybuffer,
                                                  void** data,
                                                  size_t* byte_length)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, data);
    CHECK_ARG(env, byte_length);

    auto nativeValue = reinterpret_cast<NativeValue*>(arraybuffer);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsArrayBuffer(), napi_status::napi_arraybuffer_expected);

    auto nativeArrayBuffer =
        reinterpret_cast<NativeArrayBuffer*>(nativeValue->GetInterface(NativeArrayBuffer::INTERFACE_ID));

    *data = nativeArrayBuffer->GetBuffer();
    *byte_length = nativeArrayBuffer->GetLength();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_typedarray(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    *result = nativeValue->IsTypedArray();

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_create_typedarray(napi_env env,
                                               napi_typedarray_type type,
                                               size_t length,
                                               napi_value arraybuffer,
                                               size_t byte_offset,
                                               napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto value = reinterpret_cast<NativeValue*>(arraybuffer);
    auto typedArrayType = (NativeTypedArrayType)type;

    RETURN_STATUS_IF_FALSE(env, value->IsArrayBuffer(), napi_status::napi_arraybuffer_expected);

    auto resultValue = engine->CreateTypedArray(typedArrayType, value, length, byte_offset);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_typedarray_info(napi_env env,
                                                 napi_value typedarray,
                                                 napi_typedarray_type* type,
                                                 size_t* length,
                                                 void** data,
                                                 napi_value* arraybuffer,
                                                 size_t* byte_offset)
{
    CHECK_ENV(env);
    CHECK_ARG(env, typedarray);
    CHECK_ARG(env, type);
    CHECK_ARG(env, length);
    CHECK_ARG(env, data);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, byte_offset);

    auto value = reinterpret_cast<NativeValue*>(typedarray);

    RETURN_STATUS_IF_FALSE(env, value->IsTypedArray(), napi_status::napi_invalid_arg);

    auto nativeTypedArray = reinterpret_cast<NativeTypedArray*>(value->GetInterface(NativeTypedArray::INTERFACE_ID));

    *type = (napi_typedarray_type)nativeTypedArray->GetTypedArrayType();
    *length = nativeTypedArray->GetLength();
    *data = nativeTypedArray->GetData();
    *arraybuffer = reinterpret_cast<napi_value>(nativeTypedArray->GetArrayBuffer());
    *byte_offset = nativeTypedArray->GetOffset();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status
napi_create_dataview(napi_env env, size_t length, napi_value arraybuffer, size_t byte_offset, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto arrayBufferValue = reinterpret_cast<NativeValue*>(arraybuffer);

    RETURN_STATUS_IF_FALSE(env, arrayBufferValue->IsArrayBuffer(), napi_status::napi_arraybuffer_expected);

    auto resultValue = engine->CreateDataView(arrayBufferValue, length, byte_offset);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_dataview(napi_env env, napi_value value, bool* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    *result = nativeValue->IsDataView();
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_get_dataview_info(napi_env env,
                                               napi_value dataview,
                                               size_t* bytelength,
                                               void** data,
                                               napi_value* arraybuffer,
                                               size_t* byte_offset)
{
    CHECK_ENV(env);
    CHECK_ARG(env, dataview);
    CHECK_ARG(env, bytelength);
    CHECK_ARG(env, data);
    CHECK_ARG(env, arraybuffer);
    CHECK_ARG(env, byte_offset);

    auto nativeValue = reinterpret_cast<NativeValue*>(dataview);

    RETURN_STATUS_IF_FALSE(env, nativeValue->IsDataView(), napi_status::napi_invalid_arg);

    auto nativeDataView = reinterpret_cast<NativeDataView*>(nativeValue->GetInterface(NativeDataView::INTERFACE_ID));

    *bytelength = nativeDataView->GetLength();
    *data = nativeDataView->GetBuffer();
    *arraybuffer = reinterpret_cast<napi_value>(nativeDataView->GetArrayBuffer());
    *byte_offset = nativeDataView->GetOffset();
    return napi_clear_last_error(env);
}

// version management
NAPI_EXTERN napi_status napi_get_version(napi_env env, uint32_t* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    *result = NAPI_VERSION;
    return napi_clear_last_error(env);
}

// Promises
NAPI_EXTERN napi_status napi_create_promise(napi_env env, napi_deferred* deferred, napi_value* promise)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, promise);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto resultValue = engine->CreatePromise(reinterpret_cast<NativeDeferred**>(deferred));

    *promise = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_resolve_deferred(napi_env env, napi_deferred deferred, napi_value resolution)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, resolution);

    auto nativeDeferred = reinterpret_cast<NativeDeferred*>(deferred);
    auto resolutionValue = reinterpret_cast<NativeValue*>(resolution);

    nativeDeferred->Resolve(resolutionValue);
    delete nativeDeferred;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_reject_deferred(napi_env env, napi_deferred deferred, napi_value rejection)
{
    CHECK_ENV(env);
    CHECK_ARG(env, deferred);
    CHECK_ARG(env, rejection);

    auto nativeDeferred = reinterpret_cast<NativeDeferred*>(deferred);
    auto rejectionValue = reinterpret_cast<NativeValue*>(rejection);

    nativeDeferred->Reject(rejectionValue);
    delete nativeDeferred;
    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_is_promise(napi_env env, napi_value value, bool* is_promise)
{
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, is_promise);

    auto nativeValue = reinterpret_cast<NativeValue*>(value);

    *is_promise = nativeValue->IsPromise();
    return napi_clear_last_error(env);
}

// Running a script
NAPI_EXTERN napi_status napi_run_script(napi_env env, napi_value script, napi_value* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, script);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto scriptValue = reinterpret_cast<NativeValue*>(script);

    RETURN_STATUS_IF_FALSE(env, scriptValue->TypeOf() == NATIVE_STRING, napi_status::napi_string_expected);

    auto resultValue = engine->RunScript(scriptValue);

    *result = reinterpret_cast<napi_value>(resultValue);
    return napi_clear_last_error(env);
}

// Memory management
NAPI_EXTERN napi_status napi_adjust_external_memory(napi_env env, int64_t change_in_bytes, int64_t* adjusted_value)
{
    CHECK_ENV(env);
    CHECK_ARG(env, adjusted_value);

    return napi_clear_last_error(env);
}
