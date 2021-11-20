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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_STRING_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_STRING_H

#include "ark_native_value.h"

class ArkNativeString : public ArkNativeValue, public NativeString {
public:
    ArkNativeString(ArkNativeEngine* engine, const char* value, size_t length);
    ArkNativeString(ArkNativeEngine* engine, Local<JSValueRef> value);
    ~ArkNativeString() override;

    void* GetInterface(int interfaceId) override;

    void GetCString(char* buffer, size_t size, size_t* length) override;
    void GetCString16(char16_t* buffer, size_t size, size_t* length) override;
    size_t GetLength() override;
    size_t EncodeWriteUtf8(char* buffer, size_t bufferSize, int32_t* nchars) override;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_ARK_NATIVE_VALUE_ARK_NATIVE_STRING_H */
