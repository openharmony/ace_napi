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
#include "jerryscript_native_string.h"
#include "securec.h"

JerryScriptNativeString::JerryScriptNativeString(JerryScriptNativeEngine* engine, const char* value, size_t length)
    : JerryScriptNativeString(engine, jerry_create_string_sz_from_utf8((const unsigned char*)value, length))
{
}

JerryScriptNativeString::JerryScriptNativeString(JerryScriptNativeEngine* engine, const char16_t* value, size_t length)
    : JerryScriptNativeString(engine, CreateStringFromUtf16(value, length))
{
}

JerryScriptNativeString::JerryScriptNativeString(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeValue(engine, value)
{
}

JerryScriptNativeString::~JerryScriptNativeString() {}

void* JerryScriptNativeString::GetInterface(int interfaceId)
{
    return (NativeString::INTERFACE_ID == interfaceId) ? (NativeString*)this : nullptr;
}

void JerryScriptNativeString::GetCString(char* buffer, size_t size, size_t* length)
{
    if (buffer == nullptr || size == 0) {
        *length = GetLength();
    } else {
        *length = jerry_string_to_utf8_char_buffer(value_, (jerry_char_t*)buffer, size);
    }
}

size_t JerryScriptNativeString::GetLength()
{
    return jerry_get_utf8_string_size(value_);
}

void JerryScriptNativeString::GetCString16(char16_t* buffer, size_t size, size_t* length)
{
    constexpr int maxStringLength = 1024;
    size_t utf8Length = GetLength();
    if (utf8Length <= 0 || utf8Length > maxStringLength) {
        return;
    }
    char *str = new char[utf8Length];
    jerry_string_to_utf8_char_buffer(value_, (jerry_char_t*)str, utf8Length);
    if (size < 0) {
        return;
    }
    if (length != nullptr) {
        *length = Utf8ToUtf16Length(str, utf8Length);
        if (buffer != nullptr) {
            memset_s(buffer, sizeof(char16_t) * size, 0x0, sizeof(char16_t) * size);
            Utf8ToUtf16(str, strlen(str), buffer, size);
        }
    }
    delete[] str;
}

char16_t* JerryScriptNativeString::Utf8ToUtf16(const char* utf8Str, size_t u8len, char16_t* u16str, size_t u16len)
{
    if (u16len == 0) {
        return u16str;
    }
    const char* u8end = utf8Str + u8len;
    const char* u8cur = utf8Str;
    const char16_t* u16end = u16str + u16len - 1;
    constexpr int  offset = 10;
    char16_t* u16cur = u16str;

    while ((u8cur < u8end) && (u16cur < u16end)) {
        size_t len = Utf8CodePointLen(*u8cur);
        uint32_t codepoint = Utf8ToUtf32CodePoint(u8cur, len);
        // Convert the UTF32 codepoint to one or more UTF16 codepoints
        if (codepoint <= 0xFFFF) {
            // Single UTF16 character
            *u16cur++ = (char16_t)codepoint;
        } else {
            // Multiple UTF16 characters with surrogates
            codepoint = codepoint - 0x10000;
            *u16cur++ = (char16_t)((codepoint >> offset) + 0xD800);
            if (u16cur >= u16end) {
                // Ooops...  not enough room for this surrogate pair.
                return u16cur - 1;
            }
            *u16cur++ = (char16_t)((codepoint & 0x3FF) + 0xDC00);
        }

        u8cur += len;
    }
    return u16cur;
}

size_t JerryScriptNativeString::Utf8CodePointLen(uint8_t ch)
{
    constexpr int  offset = 3;
    return ((0xe5000000 >> ((ch >> offset) & 0x1e)) & offset) + 1;
}

uint32_t JerryScriptNativeString::Utf8ToUtf32CodePoint(const char* src, size_t length)
{
    uint32_t unicode = 0;
    constexpr int  lengthSizeOne = 1;
    constexpr int  lengthSizeTwo = 2;
    constexpr int  lengthSizeThree = 3;
    constexpr int  lengthSizeFour = 4;
    constexpr int  offsetZero = 0;
    constexpr int  offsetOne = 1;
    constexpr int  offsetTwo = 2;
    constexpr int  offsetThree = 3;
    switch (length) {
        case lengthSizeOne:
            return src[offsetZero];
        case lengthSizeTwo:
            unicode = src[offsetZero] & 0x1f;
            Utf8ShiftAndMask(&unicode, src[offsetOne]);
            return unicode;
        case lengthSizeThree:
            unicode = src[offsetZero] & 0x0f;
            Utf8ShiftAndMask(&unicode, src[offsetOne]);
            Utf8ShiftAndMask(&unicode, src[offsetTwo]);
            return unicode;
        case lengthSizeFour:
            unicode = src[offsetZero] & 0x07;
            Utf8ShiftAndMask(&unicode, src[offsetOne]);
            Utf8ShiftAndMask(&unicode, src[offsetTwo]);
            Utf8ShiftAndMask(&unicode, src[offsetThree]);
            return unicode;
        default:
            return 0xffff;
    }
}

void JerryScriptNativeString::Utf8ShiftAndMask(uint32_t* codePoint, const uint8_t byte)
{
    *codePoint <<= 6;
    *codePoint |= 0x3F & byte;
}

int JerryScriptNativeString::Utf8ToUtf16Length(const char* str8, size_t str8Len)
{
    const char* str8end = str8 + str8Len;
    int utf16len = 0;
    while (str8 < str8end) {
        utf16len++;
        int u8charlen = Utf8CodePointLen(*str8);
        if (str8 + u8charlen - 1 >= str8end) {
            return -1;
        }
        uint32_t codepoint = Utf8ToUtf32CodePoint(str8, u8charlen);
        if (codepoint > 0xFFFF) {
            utf16len++; // this will be a surrogate pair in utf16
        }
        str8 += u8charlen;
    }
    if (str8 != str8end) {
        return -1;
    }
    return utf16len;
}

size_t JerryScriptNativeString::Utf32CodePointUtf8Length(char32_t srcChar)
{
    if (srcChar < ONE_BYTE_UTF8) {
        return UTF8_BYTES_ONE;
    } else if (srcChar < TWO_BYTES_UTF8) {
        return UTF8_BYTES_TWO;
    } else if (srcChar < THREE_BYTES_UTF8) {
        if ((srcChar < UNICODE_RESERVED_START) || (srcChar > UNICODE_RESERVED_END)) {
            return UTF8_BYTES_THREE;
        } else {
            // Surrogates are invalid UTF-32 characters.
            return 0;
        }
    } else if (srcChar <= UNICODE_MAX_NUM) {
        // Max code point for Unicode is 0x0010FFFF.
        return UTF8_BYTES_FOUR;
    } else {
        // Invalid UTF-32 character.
        return 0;
    }
}

// get the length of utf8 from utf16
int JerryScriptNativeString::Utf16ToUtf8Length(const char16_t* str16, size_t str16Len)
{
    if (str16 == nullptr || str16Len == 0) {
        return -1;
    }

    const char16_t* str16End = str16 + str16Len;
    int utf8Len = 0;
    while (str16 < str16End) {
        int charLen = 0;
        if (((*str16 & 0xFC00) == 0xD800) && ((str16 + 1) < str16End)
            && ((*(str16 + 1) & 0xFC00) == 0xDC00)) {
            // surrogate pairs are always 4 bytes.
            charLen = UTF8_BYTES_FOUR;
            str16 += UTF8_BYTES_TWO;
        } else {
            charLen = Utf32CodePointUtf8Length((char32_t)*str16++);
        }

        if (utf8Len > (INT_MAX - charLen)) {
            return -1;
        }
        utf8Len += charLen;
    }
    return utf8Len;
}

// inner function and str16 is not null
char* JerryScriptNativeString::Char16ToChar8(const char16_t* str16, size_t str16Len)
{
    char* str8 = nullptr;
    int utf8Len = Utf16ToUtf8Length(str16, str16Len);
    if (utf8Len < 0) {
        return nullptr;
    }

    // Allow for closing '\0'
    utf8Len += 1;
    str8 = reinterpret_cast<char*>(calloc(utf8Len, sizeof(char)));
    if (str8 == nullptr) {
        return nullptr;
    }

    StrncpyStr16ToStr8(str16, str16Len, str8, utf8Len);
    return str8;
}

// inner function, utf8Str and utf16Str is not nullptr
void JerryScriptNativeString::StrncpyStr16ToStr8(const char16_t* utf16Str,
    size_t str16Len, char* utf8Str, size_t str8Len)
{
    constexpr int  shiftLeftSize = 10;
    const char16_t* curUtf16 = utf16Str;
    const char16_t* endUtf16 = utf16Str + str16Len;
    char* cur = utf8Str;
    while (curUtf16 < endUtf16) {
        char32_t utf32;
        // surrogate pairs
        if (((*curUtf16 & 0xFC00) == 0xD800) && ((curUtf16 + 1) < endUtf16)
            && (((*(curUtf16 + 1) & 0xFC00)) == 0xDC00)) {
            utf32 = (*curUtf16++ - 0xD800) << shiftLeftSize;
            utf32 |= *curUtf16++ - 0xDC00;
            utf32 += 0x10000;
        } else {
            utf32 = *curUtf16++;
        }
        const size_t len = Utf32CodePointUtf8Length(utf32);
        if (str8Len < len) {
            break;
        }

        Utf32CodePointToUtf8(reinterpret_cast<uint8_t*>(cur), utf32, len);
        cur += len;
        str8Len -= len;
    }
    *cur = '\0';
}

void JerryScriptNativeString::Utf32CodePointToUtf8(uint8_t* dstP, char32_t srcChar, size_t bytes)
{
    dstP += bytes;
    if (bytes >= UTF8_BYTES_FOUR) {
        *--dstP = (uint8_t)((srcChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK);
        srcChar >>= UTF8_OFFSET;
    }

    if (bytes >= UTF8_BYTES_THREE) {
        *--dstP = (uint8_t)((srcChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK);
        srcChar >>= UTF8_OFFSET;
    }

    if (bytes >= UTF8_BYTES_TWO) {
        *--dstP = (uint8_t)((srcChar | UTF8_BYTE_MARK) & UTF8_BYTE_MASK);
        srcChar >>= UTF8_OFFSET;
    }

    if (bytes >= UTF8_BYTES_ONE) {
        *--dstP = (uint8_t)(srcChar | UTF8_FIRST_BYTE_MARK[bytes]);
    }
}

jerry_value_t JerryScriptNativeString::CreateStringFromUtf16(const char16_t* value, size_t length)
{
    int utf8Len = Utf16ToUtf8Length(value, length);
    const char* utf8Char = Char16ToChar8(value, length);
    return jerry_create_string_sz_from_utf8((const unsigned char*)utf8Char, utf8Len);
}