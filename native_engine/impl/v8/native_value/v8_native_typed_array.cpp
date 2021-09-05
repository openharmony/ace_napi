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

#include "v8_native_typed_array.h"

V8NativeTypedArray::V8NativeTypedArray(V8NativeEngine* engine, v8::Local<v8::Value> value)
    : V8NativeObject(engine, value)
{
}

V8NativeTypedArray::V8NativeTypedArray(V8NativeEngine* engine,
                                       NativeTypedArrayType type,
                                       NativeValue* value,
                                       size_t length,
                                       size_t offset)
    : V8NativeTypedArray(engine, v8::Local<v8::TypedArray>())
{
    v8::Local<v8::Value> v8Value = *value;
    v8::Local<v8::ArrayBuffer> buffer = v8Value.As<v8::ArrayBuffer>();
    v8::Local<v8::TypedArray> typedArray;
    switch (type) {
        case NATIVE_INT8_ARRAY:
            typedArray = v8::Int8Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT8_ARRAY:
            typedArray = v8::Uint8Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT8_CLAMPED_ARRAY:
            typedArray = v8::Uint8ClampedArray::New(buffer, offset, length);
            break;
        case NATIVE_INT16_ARRAY:
            typedArray = v8::Int16Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT16_ARRAY:
            typedArray = v8::Uint16Array::New(buffer, offset, length);
            break;
        case NATIVE_INT32_ARRAY:
            typedArray = v8::Int32Array::New(buffer, offset, length);
            break;
        case NATIVE_UINT32_ARRAY:
            typedArray = v8::Uint32Array::New(buffer, offset, length);
            break;
        case NATIVE_FLOAT32_ARRAY:
            typedArray = v8::Float32Array::New(buffer, offset, length);
            break;
        case NATIVE_FLOAT64_ARRAY:
            typedArray = v8::Float64Array::New(buffer, offset, length);
            break;
        case NATIVE_BIGINT64_ARRAY:
            typedArray = v8::BigInt64Array::New(buffer, offset, length);
            break;
        case NATIVE_BIGUINT64_ARRAY:
            typedArray = v8::BigUint64Array::New(buffer, offset, length);
            break;
        default:;
    }
    value_ = typedArray;
}

V8NativeTypedArray::~V8NativeTypedArray() {}

void* V8NativeTypedArray::GetInterface(int interfaceId)
{
    return (NativeTypedArray::INTERFACE_ID == interfaceId) ? (NativeTypedArray*)this
                                                           : V8NativeObject::GetInterface(interfaceId);
}

NativeTypedArrayType V8NativeTypedArray::GetTypedArrayType()
{
    v8::Local<v8::TypedArray> value = value_;

    NativeTypedArrayType type = NATIVE_INT8_ARRAY;

    if (value->IsInt8Array()) {
        type = NATIVE_INT8_ARRAY;
    } else if (value->IsUint8Array()) {
        type = NATIVE_UINT8_ARRAY;
    } else if (value->IsUint8ClampedArray()) {
        type = NATIVE_UINT8_CLAMPED_ARRAY;
    } else if (value->IsInt16Array()) {
        type = NATIVE_INT16_ARRAY;
    } else if (value->IsUint16Array()) {
        type = NATIVE_UINT16_ARRAY;
    } else if (value->IsInt32Array()) {
        type = NATIVE_INT32_ARRAY;
    } else if (value->IsUint32Array()) {
        type = NATIVE_UINT32_ARRAY;
    } else if (value->IsFloat32Array()) {
        type = NATIVE_FLOAT32_ARRAY;
    } else if (value->IsFloat64Array()) {
        type = NATIVE_FLOAT64_ARRAY;
    } else if (value->IsBigInt64Array()) {
        type = NATIVE_BIGINT64_ARRAY;
    } else if (value->IsBigUint64Array()) {
        type = NATIVE_BIGUINT64_ARRAY;
    }
    return type;
}

size_t V8NativeTypedArray::GetLength()
{
    v8::Local<v8::TypedArray> value = value_;

    return value->ByteLength();
}

NativeValue* V8NativeTypedArray::GetArrayBuffer()
{
    v8::Local<v8::TypedArray> value = value_;

    return V8NativeEngine::V8ValueToNativeValue(engine_, value->Buffer());
}

void* V8NativeTypedArray::GetData()
{
    v8::Local<v8::TypedArray> value = value_;

    return value->Buffer()->GetBackingStore()->Data();
}

size_t V8NativeTypedArray::GetOffset()
{
    v8::Local<v8::TypedArray> value = value_;

    return value->ByteOffset();
}
