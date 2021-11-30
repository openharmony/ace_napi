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

#include "jerryscript_native_typed_array.h"
#include "jerryscript_native_array_buffer.h"

JerryScriptNativeTypedArray::JerryScriptNativeTypedArray(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeObject(engine, value)
{
}

JerryScriptNativeTypedArray::JerryScriptNativeTypedArray(JerryScriptNativeEngine* engine,
                                                         NativeTypedArrayType type,
                                                         NativeValue* value,
                                                         size_t length,
                                                         size_t offset)
    : JerryScriptNativeTypedArray(engine, 0)
{
    jerry_typedarray_type_t jtype;

    switch (type) {
        case NATIVE_INT8_ARRAY:
            jtype = JERRY_TYPEDARRAY_INT8;
            break;
        case NATIVE_UINT8_ARRAY:
            jtype = JERRY_TYPEDARRAY_UINT8;
            break;
        case NATIVE_UINT8_CLAMPED_ARRAY:
            jtype = JERRY_TYPEDARRAY_UINT8CLAMPED;
            break;
        case NATIVE_INT16_ARRAY:
            jtype = JERRY_TYPEDARRAY_INT16;
            break;
        case NATIVE_UINT16_ARRAY:
            jtype = JERRY_TYPEDARRAY_UINT16;
            break;
        case NATIVE_INT32_ARRAY:
            jtype = JERRY_TYPEDARRAY_INT32;
            break;
        case NATIVE_UINT32_ARRAY:
            jtype = JERRY_TYPEDARRAY_UINT32;
            break;
        case NATIVE_FLOAT32_ARRAY:
            jtype = JERRY_TYPEDARRAY_FLOAT32;
            break;
        case NATIVE_FLOAT64_ARRAY:
            jtype = JERRY_TYPEDARRAY_FLOAT64;
            break;
        default:
            jtype = JERRY_TYPEDARRAY_INVALID;
            break;
    }
    value_ = jerry_create_typedarray_for_arraybuffer_sz(jtype, *value, offset, length);
}

JerryScriptNativeTypedArray::~JerryScriptNativeTypedArray() {}

void* JerryScriptNativeTypedArray::GetInterface(int interfaceId)
{
    return (NativeTypedArray::INTERFACE_ID == interfaceId) ? (NativeTypedArray*)this
                                                           : JerryScriptNativeObject::GetInterface(interfaceId);
}

NativeTypedArrayType JerryScriptNativeTypedArray::GetTypedArrayType()
{
    NativeTypedArrayType result;
    switch (jerry_get_typedarray_type(value_)) {
        case JERRY_TYPEDARRAY_INT8:
            result = NATIVE_INT8_ARRAY;
            break;
        case JERRY_TYPEDARRAY_UINT8:
            result = NATIVE_UINT8_ARRAY;
            break;
        case JERRY_TYPEDARRAY_UINT8CLAMPED:
            result = NATIVE_UINT8_CLAMPED_ARRAY;
            break;
        case JERRY_TYPEDARRAY_INT16:
            result = NATIVE_INT16_ARRAY;
            break;
        case JERRY_TYPEDARRAY_UINT16:
            result = NATIVE_UINT16_ARRAY;
            break;
        case JERRY_TYPEDARRAY_INT32:
            result = NATIVE_INT32_ARRAY;
            break;
        case JERRY_TYPEDARRAY_UINT32:
            result = NATIVE_UINT32_ARRAY;
            break;
        case JERRY_TYPEDARRAY_FLOAT32:
            result = NATIVE_FLOAT32_ARRAY;
            break;
        case JERRY_TYPEDARRAY_FLOAT64:
            result = NATIVE_FLOAT64_ARRAY;
            break;
        default:
            result = NATIVE_FLOAT64_ARRAY;
            break;
    }
    return result;
}

size_t JerryScriptNativeTypedArray::GetLength()
{
    return jerry_get_typedarray_length(value_);
}

NativeValue* JerryScriptNativeTypedArray::GetArrayBuffer()
{
    return new JerryScriptNativeArrayBuffer(engine_, jerry_get_typedarray_buffer(value_, nullptr, nullptr));
}

void* JerryScriptNativeTypedArray::GetData()
{
    jerry_value_t arrayBuffer = jerry_get_typedarray_buffer(value_, nullptr, nullptr);
    uint8_t* pointer = jerry_get_arraybuffer_pointer(arrayBuffer);
    return pointer;
}

size_t JerryScriptNativeTypedArray::GetOffset()
{
    size_t offset = 0;
    jerry_get_typedarray_buffer(value_, (unsigned int*)&offset, nullptr);
    return offset;
}
