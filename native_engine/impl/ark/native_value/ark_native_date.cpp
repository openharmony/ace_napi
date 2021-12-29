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

#include "ark_native_date.h"

using panda::DateRef;

ArkNativeDate::ArkNativeDate(ArkNativeEngine* engine, Local<JSValueRef> value)
    : ArkNativeObject(engine, value) {}

ArkNativeDate::~ArkNativeDate() {}

void* ArkNativeDate::GetInterface(int interfaceId)
{
    return (NativeDate::INTERFACE_ID == interfaceId) ? (NativeDate*)this
        : ArkNativeObject::GetInterface(interfaceId);
}

double ArkNativeDate::GetTime()
{
    Global<DateRef> value = value_;
    return value->GetTime();
}