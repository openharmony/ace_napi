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

#include "ark_native_typed_array.h"

ArkNativeTypedArray::ArkNativeTypedArray(ArkNativeEngine* engine, Local<JSValueRef> value)
    : ArkNativeObject(engine, value)
{
}

ArkNativeTypedArray::ArkNativeTypedArray(ArkNativeEngine* engine,
                                         NativeTypedArrayType type,
                                         NativeValue* value,
                                         size_t length,
                                         size_t offset)
    : ArkNativeTypedArray(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);

    Global<JSValueRef> globalValue = *value;
    Local<JSValueRef> localValue = globalValue.ToLocal(vm);
    Local<ArrayBufferRef> buffer(localValue);

    Local<TypedArrayRef> typedArray(JSValueRef::Undefined(vm));
    switch (type) {
        case NATIVE_INT8_ARRAY:
            typedArray = panda::Int8ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_UINT8_ARRAY:
            typedArray = panda::Uint8ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_UINT8_CLAMPED_ARRAY:
            typedArray = panda::Uint8ClampedArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_INT16_ARRAY:
            typedArray = panda::Int16ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_UINT16_ARRAY:
            typedArray = panda::Uint16ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_INT32_ARRAY:
            typedArray = panda::Int32ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_UINT32_ARRAY:
            typedArray = panda::Uint32ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_FLOAT32_ARRAY:
            typedArray = panda::Float32ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_FLOAT64_ARRAY:
            typedArray = panda::Float64ArrayRef::New(vm, buffer, offset, length);
            break;
        case NATIVE_BIGINT64_ARRAY:
            // not support yet
            break;
        case NATIVE_BIGUINT64_ARRAY:
            // not support yet
            break;
        default:;
    }
    Global<JSValueRef> globalTypedArray(vm, typedArray);
    value_ = globalTypedArray;
}

ArkNativeTypedArray::~ArkNativeTypedArray() {}

void* ArkNativeTypedArray::GetInterface(int interfaceId)
{
    return (NativeTypedArray::INTERFACE_ID == interfaceId) ? (NativeTypedArray*)this
                                                           : ArkNativeObject::GetInterface(interfaceId);
}

NativeTypedArrayType ArkNativeTypedArray::GetTypedArrayType()
{
    Global<TypedArrayRef> value = value_;
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
    }
    return type;
}

size_t ArkNativeTypedArray::GetLength()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<TypedArrayRef> value = value_;

    return value->ByteLength(vm);
}

NativeValue* ArkNativeTypedArray::GetArrayBuffer()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<TypedArrayRef> value = value_;

    return ArkNativeEngine::ArkValueToNativeValue(engine_, value->GetArrayBuffer(vm));
}

void* ArkNativeTypedArray::GetData()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<TypedArrayRef> value = value_;

    return value->GetArrayBuffer(vm)->GetBuffer();
}

size_t ArkNativeTypedArray::GetOffset()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<TypedArrayRef> value = value_;

    return value->ByteOffset(vm);
}
