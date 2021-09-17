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

#include "jerryscript_native_array_buffer.h"

#include "jerryscript_native_data_view.h"

JerryScriptNativeDataView::JerryScriptNativeDataView(JerryScriptNativeEngine* engine,
                                                     NativeValue* value,
                                                     size_t length,
                                                     size_t offset)
    : JerryScriptNativeDataView(engine, jerry_create_dataview((jerry_value_t)*value, offset, length))
{
}

JerryScriptNativeDataView::JerryScriptNativeDataView(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeObject(engine, value)
{
}

JerryScriptNativeDataView::~JerryScriptNativeDataView() {}

void* JerryScriptNativeDataView::GetInterface(int interfaceId)
{
    return (NativeDataView::INTERFACE_ID == interfaceId) ? (NativeDataView*)this
                                                         : JerryScriptNativeObject::GetInterface(interfaceId);
}

void* JerryScriptNativeDataView::GetBuffer()
{
    jerry_value_t arrybuffer = jerry_get_dataview_buffer(value_, nullptr, nullptr);
    uint8_t* data = jerry_get_arraybuffer_pointer(arrybuffer);
    jerry_release_value(arrybuffer);
    return data;
}

size_t JerryScriptNativeDataView::GetLength()
{
    jerry_value_t arrybuffer = jerry_get_dataview_buffer(value_, nullptr, nullptr);
    uint32_t length = jerry_get_arraybuffer_byte_length(arrybuffer);
    jerry_release_value(arrybuffer);
    return length;
}

NativeValue* JerryScriptNativeDataView::GetArrayBuffer()
{
    return new JerryScriptNativeArrayBuffer(engine_, jerry_get_dataview_buffer(value_, nullptr, nullptr));
}

size_t JerryScriptNativeDataView::GetOffset()
{
    unsigned int offset = 0;
    unsigned int length = 0;
    jerry_value_t arrybuffer = jerry_get_dataview_buffer(value_, &offset, &length);
    jerry_release_value(arrybuffer);
    return offset;
}
