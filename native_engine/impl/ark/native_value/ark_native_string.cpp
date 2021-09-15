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

#include "ark_native_string.h"
#include "securec.h"
#include "utils/log.h"

#include <codecvt>
#include <locale>
#include <string>
#include <uchar.h>

using panda::StringRef;
ArkNativeString::ArkNativeString(ArkNativeEngine* engine, const char* value, size_t length)
    : ArkNativeString(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = engine->GetEcmaVm();
    LocalScope scope(vm);
    Local<StringRef> object = StringRef::NewFromUtf8(vm, value, length);
    value_ = Global<StringRef>(vm, object);
}
ArkNativeString::ArkNativeString(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeValue(engine, value) {}

ArkNativeString::~ArkNativeString() {}

void* ArkNativeString::GetInterface(int interfaceId)
{
    return (NativeString::INTERFACE_ID == interfaceId) ? (NativeString*)this : nullptr;
}

void ArkNativeString::GetCString(char* buffer, size_t size, size_t* length)
{
    if (length == nullptr) {
        return;
    }
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<StringRef> val = value_;
    if (buffer == nullptr) {
        *length = val->Length();
    } else if (size != 0) {
        int copied = val->WriteUtf8(buffer, size) - 1;
        buffer[copied] = '\0';
        *length = copied;
    } else {
        *length = 0;
    }
}

size_t ArkNativeString::GetLength()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<StringRef> value = value_;
    return value->Length();
}

size_t ArkNativeString::EncodeWriteUtf8(char* buffer, size_t bufferSize, int32_t* nchars)
{
    if (buffer == nullptr || nchars == nullptr) {
        HILOG_ERROR("buffer is null or nchars is null");
        return 0;
    }

    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<StringRef> val = value_;
    std::string valString = val->ToString();
    std::u16string src = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(valString);
    char* oldLocale = setlocale(LC_CTYPE, "");
    setlocale(LC_CTYPE, "en_US.UTF-8");

    mbstate_t state;
    size_t pos = 0;
    size_t writableSize = bufferSize;
    char u8Char[4] = {};
    uint32_t i = 0;
    for (; i < src.length(); i++) {
        size_t rc = c16rtomb(u8Char, src.at(i), &state);
        if (rc == -1 || rc > writableSize) {
            break;
        }

        int ret = memcpy_s((buffer + pos), writableSize, u8Char, rc);
        if (ret != EOK) {
            HILOG_ERROR("memcpy_s failed");
            break;
        }
        writableSize -= rc;
        pos += rc;
    }

    setlocale(LC_CTYPE, oldLocale);
    *nchars = i;
    buffer[bufferSize] = '\0';
    HILOG_DEBUG("EncodeWriteUtf8 the result of buffer: %{public}s", buffer);
    return pos;
}
