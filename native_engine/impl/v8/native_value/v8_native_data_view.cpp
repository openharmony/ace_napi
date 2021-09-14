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

#include "v8_native_data_view.h"

#include "v8_native_array_buffer.h"

V8NativeDataView::V8NativeDataView(V8NativeEngine* engine, v8::Local<v8::Value> value) : V8NativeObject(engine, value)
{
}

V8NativeDataView::V8NativeDataView(V8NativeEngine* engine, NativeValue* value, size_t length, size_t offset)
    : V8NativeDataView(engine, v8::Local<v8::DataView>())
{
    v8::Local<v8::ArrayBuffer> arrybuffer = *value;
    value_ = v8::DataView::New(arrybuffer, offset, length);
}

V8NativeDataView::~V8NativeDataView() {}

void* V8NativeDataView::GetInterface(int interfaceId)
{
    return (NativeDataView::INTERFACE_ID == interfaceId) ? (NativeDataView*)this
                                                         : V8NativeObject::GetInterface(interfaceId);
}

void* V8NativeDataView::GetBuffer()
{
    v8::Local<v8::DataView> value = value_;

    return value->Buffer()->GetBackingStore()->Data();
}

size_t V8NativeDataView::GetLength()
{
    v8::Local<v8::DataView> value = value_;

    return value->ByteLength();
}

NativeValue* V8NativeDataView::GetArrayBuffer()
{
    v8::Local<v8::DataView> value = value_;

    return new V8NativeArrayBuffer(engine_, value->Buffer());
}

size_t V8NativeDataView::GetOffset()
{
    v8::Local<v8::DataView> value = value_;

    return value->ByteOffset();
}
