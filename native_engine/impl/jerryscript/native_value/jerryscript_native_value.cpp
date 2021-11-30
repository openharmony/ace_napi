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

#include "jerryscript_native_value.h"

#include "jerryscript-ext/handler.h"
#include "jerryscript_native_boolean.h"
#include "jerryscript_native_number.h"
#include "jerryscript_native_object.h"
#include "jerryscript_native_string.h"

JerryScriptNativeValue::JerryScriptNativeValue(JerryScriptNativeEngine* engine, jerry_value_t value) : engine_(engine)
{
    value_ = value;
    engine_->GetScopeManager()->CreateHandle(this);
}

JerryScriptNativeValue::~JerryScriptNativeValue()
{
    jerry_release_value(value_);
}

void* JerryScriptNativeValue::GetInterface(const int interfaceId)
{
    return nullptr;
}

NativeValueType JerryScriptNativeValue::TypeOf()
{
    NativeValueType result;
    switch (jerry_value_get_type(value_)) {
        case JERRY_TYPE_NONE:
            result = NATIVE_UNDEFINED;
            break;
        case JERRY_TYPE_UNDEFINED:
            result = NATIVE_UNDEFINED;
            break;
        case JERRY_TYPE_NULL:
            result = NATIVE_NULL;
            break;
        case JERRY_TYPE_BOOLEAN:
            result = NATIVE_BOOLEAN;
            break;
        case JERRY_TYPE_NUMBER:
            result = NATIVE_NUMBER;
            break;
        case JERRY_TYPE_STRING:
            result = NATIVE_STRING;
            break;
        case JERRY_TYPE_OBJECT:
            if (jerry_value_is_external(value_)) {
                result = NATIVE_EXTERNAL;
            } else {
                result = NATIVE_OBJECT;
            }
            break;
        case JERRY_TYPE_FUNCTION:
            result = NATIVE_FUNCTION;
            break;
        case JERRY_TYPE_ERROR:
            result = NATIVE_OBJECT;
            break;
        case JERRY_TYPE_SYMBOL:
            result = NATIVE_SYMBOL;
            break;
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
        case JERRY_TYPE_BIGINT:
            result = NATIVE_BIGINT;
            break;
#endif
        default:
            result = NATIVE_UNDEFINED;
            break;
    }
    return result;
}

bool JerryScriptNativeValue::InstanceOf(NativeValue* obj)
{
    jerry_value_t op = jerry_binary_operation(JERRY_BIN_OP_INSTANCEOF, value_, *obj);
    return jerry_get_boolean_value(op);
}

bool JerryScriptNativeValue::IsArray()
{
    return jerry_value_is_array(value_);
}

bool JerryScriptNativeValue::IsArrayBuffer()
{
    return jerry_value_is_arraybuffer(value_);
}

bool JerryScriptNativeValue::IsBuffer()
{
    return false;
}

bool JerryScriptNativeValue::IsDate()
{
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t date = jerryx_get_property_str(global, "Date");
    jerry_value_t result = jerry_binary_operation(JERRY_BIN_OP_INSTANCEOF, value_, date);

    bool retVal = jerry_get_boolean_value(result);

    jerry_release_value(result);
    jerry_release_value(date);
    jerry_release_value(global);

    return retVal;
}

bool JerryScriptNativeValue::IsError()
{
    return jerry_value_is_error(value_);
}

bool JerryScriptNativeValue::IsTypedArray()
{
    return jerry_value_is_typedarray(value_);
}

bool JerryScriptNativeValue::IsDataView()
{
    return jerry_value_is_dataview(value_);
}

bool JerryScriptNativeValue::IsPromise()
{
    return jerry_value_is_promise(value_);
}

NativeValue* JerryScriptNativeValue::ToBoolean()
{
    return new JerryScriptNativeBoolean(engine_, jerry_value_to_boolean(value_));
}

NativeValue* JerryScriptNativeValue::ToNumber()
{
    return new JerryScriptNativeNumber(engine_, jerry_value_to_number(value_));
}

NativeValue* JerryScriptNativeValue::ToString()
{
    return new JerryScriptNativeString(engine_, jerry_value_to_string(value_));
}

NativeValue* JerryScriptNativeValue::ToObject()
{
    return new JerryScriptNativeObject(engine_, jerry_value_to_object(value_));
}

bool JerryScriptNativeValue::StrictEquals(NativeValue* value)
{
    jerry_value_t op = jerry_binary_operation(JERRY_BIN_OP_STRICT_EQUAL, value_, *value);
    return jerry_get_boolean_value(op);
}
