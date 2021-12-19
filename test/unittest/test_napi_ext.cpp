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

#include "napi/native_common.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "test.h"
#include "test_common.h"
#include "utils/log.h"
#ifdef FOR_JERRYSCRIPT_TEST
#include "jerryscript-core.h"
#endif

/**
 * @tc.name: UndefinedTest
 * @tc.desc: Test undefined type.
 * @tc.type: FUNC
 */
static constexpr int32_t NAPI_UT_BUFFER_SIZE = 64;

HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0100, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = NAPI_UT_BUFFER_SIZE;
    napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);

    ASSERT_EQ(BufferPtr, tmpBufferPtr);
    ASSERT_EQ(BufferSize, BufferLength);
}

HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0200, testing::ext::TestSize.Level2)
{
    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = -1;
    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);

    ASSERT_EQ(creatresult, napi_status::napi_invalid_arg);
    ASSERT_EQ(BufferPtr, nullptr);
}

HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0100, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    const char bufferdata[] = "for test";
    const char* data = bufferdata;
    size_t BufferSize = NAPI_UT_BUFFER_SIZE;
    napi_create_buffer_copy(env, BufferSize, data, &BufferPtr, &Buffer);

    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);

    ASSERT_EQ(BufferPtr, tmpBufferPtr);
    ASSERT_EQ(BufferSize, BufferLength);
    ASSERT_EQ(0, memcmp(bufferdata, BufferPtr, BufferSize));
}

HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0200, testing::ext::TestSize.Level2)
{
    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    const char* data = nullptr;
    size_t BufferSize = -1;
    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, data, &BufferPtr, &Buffer);

    ASSERT_EQ(creatresult, napi_status::napi_invalid_arg);
    ASSERT_EQ(BufferPtr, nullptr);
}

HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Extern_0100, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    char testStr[] = "test";
    void* BufferPtr = testStr;

    size_t BufferSize = NAPI_UT_BUFFER_SIZE;
    napi_create_external_buffer(
        env, BufferSize, BufferPtr, [](napi_env env, void* data, void* hint) {}, (void*)testStr, &Buffer);

    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    bool isBuffer = false;
    napi_is_buffer(env, Buffer, &isBuffer);

    ASSERT_EQ(BufferSize, BufferLength);
}

HWTEST_F(NativeEngineTest, ACE_Napi_is_Buffer_0100, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = NAPI_UT_BUFFER_SIZE;
    bool isBuffer = false;

    napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);

    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    napi_is_buffer(env, Buffer, &isBuffer);

    ASSERT_TRUE(isBuffer);
}

HWTEST_F(NativeEngineTest, ACE_Napi_is_Buffer_0200, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = -1;
    bool isBuffer = false;

    napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);

    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    napi_is_buffer(env, Buffer, &isBuffer);

    ASSERT_EQ(isBuffer, false);
}

/**
 * @tc.name: StringTestAce
 * @tc.desc: Test string type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, StringTestAce, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    const char16_t testStr[] = u"中文,English,123456,!@#$%$#^%&12345";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_string);

    char16_t* buffer = nullptr;
    size_t bufferSize = 0;
    size_t strLength = 0;
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, nullptr, 0, &bufferSize));
    ASSERT_GT(bufferSize, (size_t)0);
    buffer = new char16_t[bufferSize + 1] { 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, buffer, bufferSize + 1, &strLength));
    for (int i = 0; i < testStrLength; i++) {
        ASSERT_EQ(testStr[i], buffer[i]);
    }
    ASSERT_EQ(testStrLength, strLength);
    delete[] buffer;
    buffer = nullptr;
    char16_t* bufferShort = nullptr;
    int bufferShortSize = 3;
    bufferShort = new char16_t[bufferShortSize] { 0 };
    ASSERT_CHECK_CALL(napi_get_value_string_utf16(env, result, bufferShort, bufferShortSize, &strLength));
    for (int i = 0; i < bufferShortSize; i++) {
        if (i == (bufferShortSize - 1)) {
            ASSERT_EQ(0, bufferShort[i]);
        } else {
            ASSERT_EQ(testStr[i], bufferShort[i]);
        }
    }
    ASSERT_EQ(testStrLength, strLength);
    delete[] bufferShort;
    bufferShort = nullptr;
}

HWTEST_F(NativeEngineTest, Is_detached_ArrayBuffer_Test, testing::ext::TestSize.Level0)
{
    static constexpr size_t arrayBufferSize = 1024;
    napi_env env = (napi_env)engine_;
    napi_value arrayBuffer = nullptr;
    void* arrayBufferPtr = nullptr;
    napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    bool result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));

    auto out = napi_detach_arraybuffer(env, arrayBuffer);
    if (out == napi_ok) {
        arrayBufferPtr = nullptr;
    }
    ASSERT_EQ(out, napi_ok);

    result = false;
    ASSERT_CHECK_CALL(napi_is_detached_arraybuffer(env, arrayBuffer, &result));
    ASSERT_TRUE(result);
}

#if  (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
    // jerryscript 2.3 do nothing
#else
// jerryscript 2.4 or quickjs or V8

static constexpr size_t NAPI_UT_STR_LENGTH = 30;

/**
 * @tc.name: BigIntTest
 * @tc.desc: Test number type.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, BigIntTest, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    // uint64
    {
        uint64_t testValue = UINT64_MAX;
        napi_value result = nullptr;
        bool flag = false;
        ASSERT_CHECK_CALL(napi_create_bigint_uint64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);

        uint64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_bigint_uint64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, UINT64_MAX);
        ASSERT_TRUE(flag);
    }
    {
        uint64_t testValue = 0xffffffffffffffff;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_bigint_uint64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);
        bool flag = false;
        uint64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_bigint_uint64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, testValue);
        ASSERT_TRUE(flag);
    }
    {
        uint64_t testValue = 9007199254740991;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_bigint_uint64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);
        bool flag = false;
        uint64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_bigint_uint64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, testValue);
        ASSERT_TRUE(flag);
    }
}

HWTEST_F(NativeEngineTest, BigIntTestTwo, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    // int64
    {
        int64_t testValue = INT64_MAX;
        napi_value result = nullptr;
        bool flag = false;
        ASSERT_CHECK_CALL(napi_create_bigint_int64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);

        int64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_bigint_int64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, INT64_MAX);
        ASSERT_TRUE(flag);
    }
    {
        int64_t testValue = 9007199254740991;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_bigint_int64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);
        bool flag = false;
        int64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_bigint_int64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, testValue);
        ASSERT_TRUE(flag);
    }
    {
        int64_t testValue = -1;
        napi_value result = nullptr;
        ASSERT_CHECK_CALL(napi_create_bigint_int64(env, testValue, &result));
        ASSERT_CHECK_VALUE_TYPE(env, result, napi_bigint);
        bool flag = false;
        int64_t resultValue = 0;
        ASSERT_CHECK_CALL(napi_get_value_bigint_int64(env, result, &resultValue, &flag));
        ASSERT_EQ(resultValue, testValue);
        ASSERT_TRUE(flag);
    }
}

HWTEST_F(NativeEngineTest, BigInt_Words_Test, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    int signBit = 0;
    size_t wordCount = 4;
    uint64_t words[] = { 0xFFFFFFFFFFFFFFFF, 34ULL, 56ULL, 0xFFFFFFFFFFFFFFFF };
    uint64_t wordsOut[] = { 0ULL, 0ULL, 0ULL, 0ULL };
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_bigint_words(env, signBit, wordCount, words, &result));

    ASSERT_CHECK_CALL(napi_get_value_bigint_words(env, result, &signBit, &wordCount, wordsOut));

    ASSERT_EQ(signBit, 0);
    ASSERT_EQ(wordCount, 4);
    ASSERT_EQ(words[0], wordsOut[0]);
    ASSERT_EQ(words[1], wordsOut[1]);
    ASSERT_EQ(words[2], wordsOut[2]);
    ASSERT_EQ(words[3], wordsOut[3]);
}

HWTEST_F(NativeEngineTest, BigInt_Words_Test_Normal, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    int signBit = 0;
    size_t wordCount = 5;
    uint64_t words[] = { 12ULL, 34ULL, 56ULL, 78ULL, 90ULL };
    uint64_t wordsOut[] = { 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_bigint_words(env, signBit, wordCount, words, &result));

    ASSERT_CHECK_CALL(napi_get_value_bigint_words(env, result, &signBit, &wordCount, wordsOut));

    ASSERT_EQ(signBit, 0);
    ASSERT_EQ(wordCount, 5);
    for (size_t i = 0; i < wordCount; i++) {
        ASSERT_EQ(words[i], wordsOut[i]);
    }
}

HWTEST_F(NativeEngineTest, BigInt_Words_Test_Minus, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    int signBit = 1;
    size_t wordCount = 4;
    uint64_t words[] = { 0xFFFFFFFFFFFFFFFF, 34ULL, 56ULL, 0xFFFFFFFFFFFFFFFF };
    uint64_t wordsOut[] = { 0ULL, 0ULL, 0ULL, 0ULL };
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_bigint_words(env, signBit, wordCount, words, &result));

    ASSERT_CHECK_CALL(napi_get_value_bigint_words(env, result, &signBit, &wordCount, wordsOut));

    ASSERT_EQ(signBit, 1);
    ASSERT_EQ(wordCount, 4);
    ASSERT_EQ(words[0], wordsOut[0]);
    ASSERT_EQ(words[1], wordsOut[1]);
    ASSERT_EQ(words[2], wordsOut[2]);
    ASSERT_EQ(words[3], wordsOut[3]);
}

HWTEST_F(NativeEngineTest, BigInt_Words_Test_Minus_Normal, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    int signBit = 1;
    size_t wordCount = 5;
    uint64_t words[] = { 12ULL, 34ULL, 56ULL, 78ULL, 0x000000FF98765432 };
    uint64_t wordsOut[] = { 0ULL, 0ULL, 0ULL, 0ULL, 0ULL };
    napi_value result = nullptr;
    ASSERT_CHECK_CALL(napi_create_bigint_words(env, signBit, wordCount, words, &result));

    ASSERT_CHECK_CALL(napi_get_value_bigint_words(env, result, &signBit, &wordCount, wordsOut));

    ASSERT_EQ(signBit, 1);
    ASSERT_EQ(wordCount, 5);
    for (size_t i = 0; i < wordCount; i++) {
        ASSERT_EQ(words[i], wordsOut[i]);
    }
}

HWTEST_F(NativeEngineTest, freeze_object_Test, testing::ext::TestSize.Level0)
{
    constexpr int DataSize = 60;
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_create_object(env, &object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, object, "strAttribute", strAttribute);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, object, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_object_freeze(env, object));

    int32_t testNumber2 = 111111;
    napi_value numberAttribute2 = nullptr;
    napi_create_int32(env, testNumber2, &numberAttribute2);
    ASSERT_CHECK_CALL(napi_set_named_property(env, object, "test", numberAttribute2));

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_all_property_names(env, object, keyMode, keyFilter, keyConversion, &propNames));

    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, (uint32_t)2);

    char names[2][30];
    memset_s(names, DataSize, 0, DataSize);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);
    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = NAPI_UT_STR_LENGTH;
        char testStrInner[NAPI_UT_STR_LENGTH + 1];
        size_t outStrLength = 0;
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

HWTEST_F(NativeEngineTest, seal_object_Test, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;

    napi_create_object(env, &object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, object, "strAttribute", strAttribute);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, object, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_object_seal(env, object));

    int32_t testNumber2 = 111111;
    napi_value numberAttribute2 = nullptr;
    napi_create_int32(env, testNumber2, &numberAttribute2);
    ASSERT_CHECK_CALL(napi_set_named_property(env, object, "test", numberAttribute2));

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    ASSERT_CHECK_CALL(napi_get_all_property_names(env, object, keyMode, keyFilter, keyConversion, &propNames));

    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, (uint32_t)2);

    char names[2][NAPI_UT_STR_LENGTH];
    memset_s(names, NAPI_UT_STR_LENGTH * 2, 0, NAPI_UT_STR_LENGTH * 2);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = NAPI_UT_STR_LENGTH;
        char testStrInner[NAPI_UT_STR_LENGTH + 1];
        size_t outStrLength = 0;
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

HWTEST_F(NativeEngineTest, all_property_names_Test, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value result = nullptr;
    napi_value propNames = nullptr;

    ASSERT_CHECK_CALL(napi_create_object(env, &result));
    ASSERT_CHECK_VALUE_TYPE(env, result, napi_object);

    const char testStr[] = "1234567";
    napi_value strAttribute = nullptr;
    napi_create_string_utf8(env, testStr, strlen(testStr), &strAttribute);
    napi_set_named_property(env, result, "strAttribute", strAttribute);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_create_int32(env, testNumber, &numberAttribute);
    napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    ASSERT_CHECK_CALL(napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames));

    ASSERT_CHECK_VALUE_TYPE(env, propNames, napi_object);
    bool isArray = false;
    ASSERT_CHECK_CALL(napi_is_array(env, propNames, &isArray));
    ASSERT_TRUE(isArray);
    uint32_t arrayLength = 0;
    ASSERT_CHECK_CALL(napi_get_array_length(env, propNames, &arrayLength));
    ASSERT_EQ(arrayLength, (uint32_t)2);

    char names[2][NAPI_UT_STR_LENGTH];
    memset_s(names, NAPI_UT_STR_LENGTH * 2, 0, NAPI_UT_STR_LENGTH * 2);
    auto ret = memcpy_s(names[0], strlen("strAttribute"), "strAttribute", strlen("strAttribute"));
    ASSERT_EQ(ret, EOK);
    ret = memcpy_s(names[1], strlen("numberAttribute"), "numberAttribute", strlen("numberAttribute"));
    ASSERT_EQ(ret, EOK);

    for (uint32_t i = 0; i < arrayLength; i++) {
        bool hasElement = false;
        ASSERT_CHECK_CALL(napi_has_element(env, propNames, i, &hasElement));

        napi_value propName = nullptr;
        ASSERT_CHECK_CALL(napi_get_element(env, propNames, i, &propName));
        ASSERT_CHECK_VALUE_TYPE(env, propName, napi_string);

        size_t testStrLength = NAPI_UT_STR_LENGTH;
        char testStrInner[NAPI_UT_STR_LENGTH + 1];
        size_t outStrLength = 0;
        memset_s(testStrInner, testStrLength + 1, 0, testStrLength + 1);
        ASSERT_CHECK_CALL(napi_get_value_string_utf8(env, propName, testStrInner, testStrLength, &outStrLength));

        int ret = strcmp(testStrInner, names[i]);
        ASSERT_EQ(ret, 0);
    }
}

HWTEST_F(NativeEngineTest, tag_object_Test, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };

    ASSERT_CHECK_CALL(napi_create_object(env, &object));

    ASSERT_CHECK_CALL(napi_type_tag_object(env, object, &typeTag));

    bool checkResult = false;

    ASSERT_CHECK_CALL(napi_check_object_type_tag(env, object, &typeTag, &checkResult));
    ASSERT_TRUE(checkResult);
}
#endif

HWTEST_F(NativeEngineTest, get_date_Test, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    napi_value createResult = nullptr;
    double time = 202110181203150;

    ASSERT_CHECK_CALL(napi_create_date(env, time, &createResult));

    double getTime = false;

    ASSERT_CHECK_CALL(napi_get_date_value(env, createResult, &getTime));
    bool result = false;
    if (time == getTime) {
        result = true;
    }
    ASSERT_TRUE(result);
}

HWTEST_F(NativeEngineTest, is_date_Test, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    napi_value createResult = nullptr;
    double time = 202110181203150;

    ASSERT_CHECK_CALL(napi_create_date(env, time, &createResult));

    bool result = false;

    ASSERT_CHECK_CALL(napi_is_date(env, createResult, &result));

    ASSERT_TRUE(result);
}

/**
 * @tc.name: ACE_napi_adjust_external_memory_test.
 * @tc.desc: Test napi_adjust_external_memory.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ACE_napi_adjust_external_memory_test, testing::ext::TestSize.Level0)
{
    HILOG_INFO("%{public}s", "ACE_napi_adjust_external_memory_test start");
    napi_env env = (napi_env)engine_;
    int64_t change_in_bytes = 32;
    int64_t adjusted_value = 32;
    napi_status ret = napi_adjust_external_memory(env, change_in_bytes, &adjusted_value);
    ASSERT_EQ(ret, napi_ok);
    HILOG_INFO("%{public}s", "ACE_napi_adjust_external_memory_test end");
}

/**
 * @tc.name: ACE_napi_async_init_Test.
 * @tc.desc: Test napi_async_init, napi_async_destroy.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ACE_napi_async_init_Test_001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("ACE_napi_async_init_Test_001 start");

    napi_env env = (napi_env)engine_;

    napi_value resourceName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "ACE_napi_async_init_Test_001",
        NAPI_AUTO_LENGTH, &resourceName));

    napi_async_context context = nullptr;
    napi_status ret = napi_async_init(env, nullptr, resourceName, &context);
    ASSERT_EQ(ret, napi_ok);
    EXPECT_NE(context, nullptr);

    ret = napi_async_destroy(env, context);
    ASSERT_EQ(ret, napi_ok);

    HILOG_INFO("ACE_napi_async_init_Test_001 end");
}

/**
 * @tc.name: ACE_napi_open_callback_scope_Test
 * @tc.desc: Test napi_open_callback_scope, napi_close_callback_scope.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ACE_napi_open_callback_scope_Test_001, testing::ext::TestSize.Level0)
{
    HILOG_INFO("ACE_napi_open_callback_scope_Test_001 start");

    napi_env env = (napi_env)engine_;

    auto callbackScopeManager = engine_->GetCallbackScopeManager();
    ASSERT_NE(callbackScopeManager, nullptr);

    int openCallbackScopesBefore = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthBefore = callbackScopeManager->GetAsyncCallbackScopeDepth();

    napi_value resourceName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &resourceName));

    napi_async_context context;
    NAPI_CALL_RETURN_VOID(env, napi_async_init(env, nullptr, resourceName, &context));

    napi_callback_scope scope = nullptr;
    napi_status ret = napi_open_callback_scope(env, NULL, context, &scope);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_NE(scope, nullptr);

    int openCallbackScopes = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepth = callbackScopeManager->GetAsyncCallbackScopeDepth();
    EXPECT_EQ(openCallbackScopes, (openCallbackScopesBefore + 1));
    EXPECT_EQ(asyncCallbackScopeDepth, (asyncCallbackScopeDepthBefore + 1));

    ret = napi_close_callback_scope(env, scope);
    EXPECT_EQ(ret, napi_ok);

    int openCallbackScopesAfter = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthAfter = callbackScopeManager->GetAsyncCallbackScopeDepth();
    EXPECT_EQ(openCallbackScopesAfter, openCallbackScopesBefore);
    EXPECT_EQ(asyncCallbackScopeDepthAfter, asyncCallbackScopeDepthBefore);

    NAPI_CALL_RETURN_VOID(env, napi_async_destroy(env, context));

    HILOG_INFO("ACE_napi_open_callback_scope_Test_001 end");
}

/**
 * @tc.name: ACE_napi_open_callback_scope_Test
 * @tc.desc: Test napi_open_callback_scope, napi_close_callback_scope.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, ACE_napi_open_callback_scope_Test_002, testing::ext::TestSize.Level0)
{
    HILOG_INFO("ACE_napi_open_callback_scope_Test_002 start");

    napi_env env = (napi_env)engine_;

    auto callbackScopeManager = engine_->GetCallbackScopeManager();
    ASSERT_NE(callbackScopeManager, nullptr);

    int openCallbackScopesBefore = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepthBefore = callbackScopeManager->GetAsyncCallbackScopeDepth();

    napi_value resourceName;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &resourceName));

    napi_async_context context;
    NAPI_CALL_RETURN_VOID(env, napi_async_init(env, nullptr, resourceName, &context));

    napi_callback_scope scope = nullptr;
    napi_status ret = napi_open_callback_scope(env, NULL, context, &scope);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_NE(scope, nullptr);

    int openCallbackScopes1 = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepth1 = callbackScopeManager->GetAsyncCallbackScopeDepth();

    // Open a internal callback scope
    auto scope2 = callbackScopeManager->Open(engine_);
    int openCallbackScopes2 = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepth2 = callbackScopeManager->GetAsyncCallbackScopeDepth();

    EXPECT_NE(scope2, nullptr);
    EXPECT_EQ(openCallbackScopes2, openCallbackScopes1);
    EXPECT_EQ(asyncCallbackScopeDepth2, (asyncCallbackScopeDepth1 + 1));
    
    callbackScopeManager->Close(scope2);
    int openCallbackScopes2After = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepth2After = callbackScopeManager->GetAsyncCallbackScopeDepth();

    EXPECT_EQ(openCallbackScopes2After, openCallbackScopes1);
    EXPECT_EQ(asyncCallbackScopeDepth2After, asyncCallbackScopeDepth1);

    ret = napi_close_callback_scope(env, scope);
    EXPECT_EQ(ret, napi_ok);

    int openCallbackScopes1After = callbackScopeManager->GetOpenCallbackScopes();
    int asyncCallbackScopeDepth1After = callbackScopeManager->GetAsyncCallbackScopeDepth();

    EXPECT_EQ(openCallbackScopes1After, openCallbackScopesBefore);
    EXPECT_EQ(asyncCallbackScopeDepth1After, asyncCallbackScopeDepthBefore);

    NAPI_CALL_RETURN_VOID(env, napi_async_destroy(env, context));

    HILOG_INFO("ACE_napi_open_callback_scope_Test_002 end");
}

static napi_value TestFatalException(napi_env env, napi_callback_info info)
{
    napi_value err;
    size_t argc = 1;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &err, nullptr, nullptr));
    NAPI_CALL(env, napi_fatal_exception(env, err));
    return nullptr;
}

/**
 * @tc.name: FatalException
 * @tc.desc: Test FatalException Func.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, FatalExceptionTest_001, testing::ext::TestSize.Level0)
{
    napi_env env = (napi_env)engine_;
    ASSERT_EQ(TestFatalException(env, nullptr), nullptr);
}

HWTEST_F(NativeEngineTest, add_finalizer_test_0100, testing::ext::TestSize.Level0)
{
    HILOG_INFO("add_finalizer_test_0100 start");
    napi_env env = (napi_env)engine_;

    napi_value object;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &object));

    static bool testValue = false;
    const char* testStr = "test";
    napi_ref ref = nullptr;
    napi_add_finalizer(
        env, object, (void*)testStr, [](napi_env env, void* data, void* hint) {
            testValue = true;
        }, nullptr, &ref);

    napi_delete_reference(env, ref);
    ASSERT_TRUE(testValue);
    HILOG_INFO("add_finalizer_test_0100 end");
}
