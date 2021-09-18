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

#include "jerryscript_native_array.h"

JerryScriptNativeArray::JerryScriptNativeArray(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeObject(engine, value)
{
}

JerryScriptNativeArray::JerryScriptNativeArray(JerryScriptNativeEngine* engine, int length)
    : JerryScriptNativeArray(engine, jerry_create_array(length))
{
}

JerryScriptNativeArray::~JerryScriptNativeArray() {}

void* JerryScriptNativeArray::GetInterface(int interfaceId)
{
    return (NativeArray::INTERFACE_ID == interfaceId) ? (NativeArray*)this
                                                      : JerryScriptNativeObject::GetInterface(interfaceId);
}

bool JerryScriptNativeArray::SetElement(uint32_t index, NativeValue* value)
{
    jerry_value_t returnValue = jerry_set_property_by_index(value_, index, value_);
    jerry_release_value(returnValue);
    return true;
}

NativeValue* JerryScriptNativeArray::GetElement(uint32_t index)
{
    jerry_value_t returnValue = jerry_get_property_by_index(value_, index);
    return JerryScriptNativeEngine::JerryValueToNativeValue(engine_, returnValue);
}

bool JerryScriptNativeArray::HasElement(uint32_t index)
{
    jerry_value_t returnValue = jerry_get_property_by_index(value_, index);
    bool result = !jerry_value_is_undefined(returnValue);
    jerry_release_value(returnValue);
    return result;
}

bool JerryScriptNativeArray::DeleteElement(uint32_t index)
{
    return jerry_delete_property_by_index(value_, index);
}

uint32_t JerryScriptNativeArray::GetLength()
{
    return jerry_get_array_length(value_);
}
