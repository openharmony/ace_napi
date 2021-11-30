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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_STRING_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_STRING_H

#include "jerryscript_native_value.h"

class JerryScriptNativeString : public JerryScriptNativeValue, public NativeString {
public:
    JerryScriptNativeString(JerryScriptNativeEngine* engine, const char* value, size_t length);
    JerryScriptNativeString(JerryScriptNativeEngine* engine, const char16_t* value, size_t length);
    JerryScriptNativeString(JerryScriptNativeEngine* engine, jerry_value_t value);
    virtual ~JerryScriptNativeString() override;

    void* GetInterface(int interfaceId) override;

    virtual void GetCString(char* buffer, size_t size, size_t* length) override;
    virtual size_t GetLength() override;
    virtual size_t EncodeWriteUtf8(char* buffer, size_t bufferSize, int32_t* nchars) override { return 0; }
    virtual void GetCString16(char16_t* buffer, size_t size, size_t* length) override;
private:
    char16_t* Utf8ToUtf16(const char* utf8Str, size_t u8len, char16_t* u16str, size_t u16len);
    static inline size_t Utf8CodePointLen(uint8_t ch);
    uint32_t Utf8ToUtf32CodePoint(const char* src, size_t length);
    static inline void Utf8ShiftAndMask(uint32_t* codePoint, const uint8_t byte);
    int Utf8ToUtf16Length(const char* str8, size_t str8Len);
    size_t Utf32CodePointUtf8Length(char32_t srcChar);
    int Utf16ToUtf8Length(const char16_t* str16, size_t str16Len);
    char* Char16ToChar8(const char16_t* str16, size_t str16Len);
    void StrncpyStr16ToStr8(const char16_t* utf16Str, size_t str16Len, char* utf8Str, size_t str8Len);
    void Utf32CodePointToUtf8(uint8_t* dstP, char32_t srcChar, size_t bytes);
    jerry_value_t CreateStringFromUtf16(const char16_t* value, size_t length);
    constexpr static char32_t ONE_BYTE_UTF8 = 0x00000080;
    constexpr static char32_t TWO_BYTES_UTF8 = 0x00000800;
    constexpr static char32_t THREE_BYTES_UTF8 = 0x00010000;

    constexpr static char32_t UNICODE_RESERVED_START = 55296;
    constexpr static char32_t UNICODE_RESERVED_END = 0x0000DFFF;
    constexpr static char32_t UNICODE_MAX_NUM = 0x0010FFFF;
    constexpr static unsigned int UTF8_OFFSET = 6;

    constexpr static char32_t UTF8_BYTE_MASK = 0x000000BF;
    constexpr static char32_t UTF8_BYTE_MARK = 0x00000080;
    constexpr static char32_t UTF8_FIRST_BYTE_MARK[] = {
        0x00000000, 0x00000000, 0x000000C0, 0x000000E0, 0x000000F0
    };
    constexpr static int UTF8_BYTES_ONE = 1;
    constexpr static int UTF8_BYTES_TWO = 2;
    constexpr static int UTF8_BYTES_THREE = 3;
    constexpr static int UTF8_BYTES_FOUR = 4;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_STRING_H */
