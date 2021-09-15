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

#include "ark_native_data_view.h"
#include "ark_native_array_buffer.h"

using panda::DataViewRef;
using panda::ArrayBufferRef;
ArkNativeDataView::ArkNativeDataView(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeObject(engine, value)
{
}

ArkNativeDataView::ArkNativeDataView(ArkNativeEngine* engine, NativeValue* value, size_t length, size_t offset)
    : ArkNativeDataView(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Global<ArrayBufferRef> arraybuffer = *value;
    Local<DataViewRef> dataView = DataViewRef::New(vm, arraybuffer.ToLocal(vm), offset, length);
    value_ = Global<DataViewRef>(vm, dataView);
}

ArkNativeDataView::~ArkNativeDataView() {}

void* ArkNativeDataView::GetInterface(int interfaceId)
{
    return (NativeDataView::INTERFACE_ID == interfaceId) ? (NativeDataView*)this
                                                         : ArkNativeObject::GetInterface(interfaceId);
}

void* ArkNativeDataView::GetBuffer()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<DataViewRef> value = value_;

    return value->GetArrayBuffer(vm)->GetBuffer();
}

size_t ArkNativeDataView::GetLength()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<DataViewRef> value = value_;

    return value->ByteLength();
}

NativeValue* ArkNativeDataView::GetArrayBuffer()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<DataViewRef> value = value_;

    return new ArkNativeArrayBuffer(engine_, value->GetArrayBuffer(vm));
}

size_t ArkNativeDataView::GetOffset()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<DataViewRef> value = value_;

    return value->ByteOffset();
}
