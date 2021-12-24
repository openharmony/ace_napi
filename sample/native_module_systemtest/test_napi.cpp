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
#define NAPI_VERSION 7
#ifndef NAPI_EXPERIMENTAL
#define NAPI_EXPERIMENTAL
#endif
#include <ctime>
#include <securec.h>
#ifdef FOR_JERRYSCRIPT_TEST
#include "jerryscript-core.h"
#endif
#include "js_native_api_types.h"
#define private public
#include "native_api.h"
#include "native_common.h"
#include "native_node_api.h"
#include "native_value.h"
#include "securec.h"
#include "test.h"
#include "utils/log.h"
#undef private

struct CallJsCbData {
    int32_t id = 0;
};
using CallJsCbData_t = struct CallJsCbData;

struct Final_Cb_Data {
    int32_t id = 0;
};
using FinalCbData_t = struct Final_Cb_Data;

struct ThreadData {
    napi_threadsafe_function tsfn = nullptr;
    napi_threadsafe_function_call_mode isMode = napi_tsfn_nonblocking;
};
using ThreadData_t = struct ThreadData;

struct Final_CbData {
    int32_t id;
};
using FinalCbData = struct Final_CbData;

struct Call_JsCbData_str {
    int32_t id;
    char strdata[12];
};
using CallJsCbData_str = struct Call_JsCbData_str;

static constexpr int32_t SEND_DATA = 10;
static constexpr int32_t CALL_JSCB_DATA = 20;
static constexpr int32_t FINAL_CB_DATA = 30;
static constexpr size_t MAX_COUNT = 128;
static constexpr size_t OVER_MAX_COUNT = 129;
static uv_thread_t g_uvThread;
static FinalCbData finalData;
static CallJsCbData jsData;
static CallJsCbData_str jsData_str;
static int g_callJSCallBackCount = 0;
static int g_callCount = 0;
static bool g_bFailFlag = false;
static bool g_bIsFinish = false;
static napi_env g_stEnv;
static int iBlockCallTimes = 20;
static int iNoneBlockCallTimes = 30;
static CallJsCbData_t g_jsData;
static FinalCbData_t g_finalData;
static int g_threadDataContent = 608;
static int g_threadDataContent2 = 708;
static int g_threadDataContent3 = 808;
static int g_threadDataContent4 = 908;
static bool g_callFinalizeEnd = false;

static int HOOK_ARG_ONE = 1;
static int HOOK_ARG_TWO = 2;
static int HOOK_ARG_THREE = 3;
static int HOOK_TAG = 0;
static int HOOK_TAGCP = 0;

static constexpr int32_t CALL_JS_CB_DATA_TEST_ID = 101;
static constexpr int32_t FINAL_CB_DATA_TEST_ID = 201;

#if  (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
    // jerryscript 2.3 do nothing
#else
    // jerryscript 2.4 or quickjs or V8
    static constexpr int UINT64_VALUE = 100;
    static constexpr int INT64_VALUE = 100;
#endif

static constexpr int INT_ZERO = 0;
static constexpr int INT_ONE = 1;
static constexpr int INT_TWO = 2;
static constexpr int INT_THREE = 3;
static constexpr int INT_FIVE = 5;
#if  (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
    // jerryscript 2.3 do nothing
#else
    static constexpr int INT_FOUR = 4;
#endif


static constexpr size_t ARRAYBUFFER_SIZE_NULL = 0;
static constexpr size_t ARRAYBUFFER_SIZE = 1024;
static constexpr int64_t CHANGE_IN_BYTES = 1024;
static constexpr int64_t ADJUSTED_VALUE = 0;
static constexpr double CREATE_DATE_TIME = 11.11;
static constexpr double DATE_TIME_VALUE = 0;
static constexpr double GET_DATE_TIME = 11;

static constexpr size_t BUFFER_OVERMAX_SIZE = 2147483648;
static char Text[] = "hello world";

static void GetFinalizeStatus()
{
    while (!g_callFinalizeEnd) {
        sleep(1);
    };
}
static void NewChildThreadMuti(void* data)
{
    GTEST_LOG_(INFO) << "NewChildThreadMuti called start";
    ThreadData_t* threadData = static_cast<ThreadData_t*>(data);
    int testCount = 20;
    for (int i = 0; i < testCount; i++) {
        auto status = napi_call_threadsafe_function(threadData->tsfn, (void*)&g_threadDataContent2, threadData->isMode);
        EXPECT_EQ(status, napi_ok);
    }
    GTEST_LOG_(INFO) << "NewChildThreadMuti called end";
}
static void NonBlockAndBlockNewChildThreadMuti(void* data)
{
    GTEST_LOG_(INFO) << "NonBlockAndBlockNewChildThreadMuti called start";
    ThreadData_t* threadData = static_cast<ThreadData_t*>(data);
    int testCount = 10;
    for (int i = 0; i < testCount; i++) {
        auto status = napi_call_threadsafe_function(threadData->tsfn, (void*)&g_threadDataContent3, napi_tsfn_blocking);
        EXPECT_EQ(status, napi_ok);
    }

    for (int i = 0; i < testCount; i++) {
        auto status =
            napi_call_threadsafe_function(threadData->tsfn, (void*)&g_threadDataContent3, napi_tsfn_nonblocking);
        EXPECT_EQ(status, napi_ok);
    }
    GTEST_LOG_(INFO) << "NonBlockAndBlockNewChildThreadMuti called end";
}

struct OneModeCallData {
    napi_threadsafe_function tsfn = nullptr;
    napi_threadsafe_function_call_mode mode = napi_tsfn_nonblocking;
    int callCount = 0;
};
using OneModeCallData_t = struct OneModeCallData;

static void OneModeCall(void* data)
{
    OneModeCallData_t* callData = static_cast<OneModeCallData_t*>(data);
    if (callData == nullptr) {
        return;
    }
    for (int i = 0; i < callData->callCount; i++) {
        auto status = napi_call_threadsafe_function(callData->tsfn, (void*)&g_threadDataContent4, callData->mode);
        EXPECT_EQ(status, napi_ok);
    }
}

static void MutiModeCallOne(void* data)
{
    GTEST_LOG_(INFO) << "MutiModeCallOne called start";
    OneModeCallData_t* callData = static_cast<OneModeCallData_t*>(data);
    callData->mode = napi_tsfn_nonblocking;
    int testCount = 10;
    callData->callCount = testCount;
    OneModeCall(data);

    callData->mode = napi_tsfn_blocking;
    OneModeCall(data);

    callData->mode = napi_tsfn_nonblocking;
    OneModeCall(data);

    GTEST_LOG_(INFO) << "MutiModeCallOne called end";
}
static void MutiModeCallTwo(void* data)
{
    GTEST_LOG_(INFO) << "MutiModeCallTwo called start";
    OneModeCallData_t* callData = static_cast<OneModeCallData_t*>(data);
    callData->mode = napi_tsfn_blocking;
    int testCount = 10;
    callData->callCount = testCount;
    OneModeCall(data);

    callData->mode = napi_tsfn_nonblocking;
    OneModeCall(data);

    callData->mode = napi_tsfn_blocking;
    OneModeCall(data);

    GTEST_LOG_(INFO) << "MutiModeCallTwo called end";
}
static void MutiModeCallThree(void* data)
{
    GTEST_LOG_(INFO) << "MutiModeCallThree called start";
    OneModeCallData_t* callData = static_cast<OneModeCallData_t*>(data);
    callData->mode = napi_tsfn_nonblocking;
    int testCount = 30;
    callData->callCount = testCount;
    OneModeCall(data);

    GTEST_LOG_(INFO) << "MutiModeCallThree called end";
}
static void MutiModeCallFour(void* data)
{
    GTEST_LOG_(INFO) << "MutiModeCallFour called start";
    OneModeCallData_t* callData = static_cast<OneModeCallData_t*>(data);
    callData->mode = napi_tsfn_blocking;
    int testCount = 30;
    callData->callCount = testCount;
    OneModeCall(data);

    GTEST_LOG_(INFO) << "MutiModeCallFour called end";
}
static void NonBlockFinalizeThreadCallBack(napi_env env, void* finalizeData, void* hint)
{
    GTEST_LOG_(INFO) << "NonBlockFinalizeThreadCallBack called";

    int CallJSCallCount = 40;
    EXPECT_EQ(g_callJSCallBackCount, CallJSCallCount);

    g_callFinalizeEnd = true;
}

static void CallJSCallBack(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    GTEST_LOG_(INFO) << "CallJSCallBack called";
    g_callJSCallBackCount++;

    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);
    GTEST_LOG_(INFO) << "CallJSCallBack param:data =" << *((int*)data);

    GTEST_LOG_(INFO) << "CallJSCallBack Count=" << g_callJSCallBackCount;
}

static void FinalizeThreadCallBack(napi_env env, void* finalizeData, void* hint)
{
    GTEST_LOG_(INFO) << "FinalizeThreadCallBack called start";

    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);

    int CallJSCallCount = 40;
    EXPECT_EQ(g_callJSCallBackCount, CallJSCallCount);

    g_callFinalizeEnd = true;
}

static void MutiModeFinalizeThreadCallBack(napi_env env, void* finalizeData, void* hint)
{
    GTEST_LOG_(INFO) << "MutiModeFinalizeThreadCallBack called start";

    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);

    int CallJSCallCount = 120;
    EXPECT_EQ(g_callJSCallBackCount, CallJSCallCount);

    g_callFinalizeEnd = true;
}
static void AllFinalizeThreadCallBack(napi_env env, void* finalizeData, void* hint)
{
    GTEST_LOG_(INFO) << "AllFinalizeThreadCallBack called start";

    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);

    int CallJSCallCount = 40;
    EXPECT_EQ(g_callJSCallBackCount, CallJSCallCount);

    g_callFinalizeEnd = true;
}
static void OtherFinalizeThreadCallBack(napi_env env, void* finalizeData, void* hint)
{
    GTEST_LOG_(INFO) << "OtherFinalizeThreadCallBack called start";

    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);

    int CallJSCallCount = 46;
    EXPECT_EQ(g_callJSCallBackCount, CallJSCallCount);

    g_callFinalizeEnd = true;
}
static void BufferFinalizer(napi_env env, void* data, void* hint)
{}

static void ExpectCheckCall(napi_status call)
{
    EXPECT_EQ(call, napi_ok);
}

static void AssertCheckValueType(napi_env env, napi_value value, napi_valuetype type)
{
    napi_valuetype valueType = napi_undefined;
    EXPECT_TRUE(value != nullptr);
    ExpectCheckCall(napi_typeof(env, value, &valueType));
    EXPECT_EQ(valueType, type);
}

static void NewChildRef(void* data)
{
    GTEST_LOG_(INFO) << "NewChildRef called ";
    napi_threadsafe_function tsFunc = (napi_threadsafe_function)data;
    auto status = napi_ref_threadsafe_function(g_stEnv, tsFunc);
    EXPECT_NE(status, napi_ok);
}
static void NewChildUnRef(void* data)
{
    GTEST_LOG_(INFO) << "NewChildUnRef called ";
    napi_threadsafe_function tsFunc = (napi_threadsafe_function)data;
    auto status = napi_unref_threadsafe_function(g_stEnv, tsFunc);
    EXPECT_NE(status, napi_ok);
}
static void NewChildThreadMutiCallBlocking(void* data)
{
    GTEST_LOG_(INFO) << "NewChildThreadMutiCallBlocking called start";
    napi_threadsafe_function tsFunc = (napi_threadsafe_function)data;

    for (int i = 0; i < iBlockCallTimes; i++) {
        auto status = napi_call_threadsafe_function(tsFunc, nullptr, napi_tsfn_blocking);
        EXPECT_EQ(status, napi_ok);
        g_callCount++;
    }
    GTEST_LOG_(INFO) << "NewChildThreadMutiCallBlocking called end";
}

static void NewChildThreadMutiCallNoneBlocking(void* data)
{
    GTEST_LOG_(INFO) << "NewChildThreadMutiCallNoneBlocking called start";
    napi_threadsafe_function tsFunc = (napi_threadsafe_function)data;
    int iFailTimes = 0;

    for (int i = 0; i < iNoneBlockCallTimes; i++) {
        auto status = napi_call_threadsafe_function(tsFunc, nullptr, napi_tsfn_nonblocking);
        if (napi_ok != status) {
            iFailTimes++;
        }
    }
    if (iFailTimes > 0) {
        g_bFailFlag = true;
    }
    GTEST_LOG_(INFO) << "none block call fail times" << iFailTimes;
    GTEST_LOG_(INFO) << "NewChildThreadMutiCallNoneBlocking called end";
}

static void CallJSSlowCallBack(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    GTEST_LOG_(INFO) << "CallJSSlowCallBack called";
    sleep(1);
    g_callJSCallBackCount++;
    GTEST_LOG_(INFO) << "CallJSSlowCallBack Count=" << g_callJSCallBackCount;
}

static void CallJSSlowCallBack4Block(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    GTEST_LOG_(INFO) << "CallJSSlowCallBack called";
    sleep(1);
    g_callJSCallBackCount++;
    while ((!g_bFailFlag) && (g_callJSCallBackCount > iBlockCallTimes)) {
        sleep(1);
    }
    GTEST_LOG_(INFO) << "CallJSSlowCallBack Count=" << g_callJSCallBackCount;
}

static void FinalCallBack(napi_env env, void* finalizeData, void* hint)
{
    GTEST_LOG_(INFO) << "FinalCallBack called";
    g_bIsFinish = true;
}

static void WaitForFinish()
{
    while (!g_bIsFinish) {
        sleep(1);
    }
}

static void ThreadSafeCallJs(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    GTEST_LOG_(INFO) << "ThreadSafeCallJs start";

    CallJsCbData* jsData = nullptr;
    jsData = (CallJsCbData*)context;
    GTEST_LOG_(INFO) << "context is" << context << "jsData->id is" << jsData->id;
    EXPECT_EQ(jsData->id, CALL_JSCB_DATA);
    int32_t* pData = nullptr;
    pData = (int32_t*)data;
    EXPECT_EQ((*pData), SEND_DATA);

    GTEST_LOG_(INFO) << "ThreadSafeCallJs end";
}

static void Threadfinalcb(napi_env env, void* finalizeData, void* context)
{
    GTEST_LOG_(INFO) << "Threadfinalcb called";

    uv_thread_join(&g_uvThread);
    GTEST_LOG_(INFO) << "context->id" << ((FinalCbData*)context)->id;
    CallJsCbData* jsData = nullptr;
    jsData = (CallJsCbData*)context;
    GTEST_LOG_(INFO) << "jsData->id" << jsData->id;
    EXPECT_EQ(jsData->id, CALL_JSCB_DATA);
    int32_t* pData = nullptr;
    pData = (int32_t*)finalizeData;
    EXPECT_EQ((*pData), FINAL_CB_DATA);
    g_callFinalizeEnd = true;
    GTEST_LOG_(INFO) << "Threadfinalcb end";
}

static void TsFuncDataSourceThread(void* data)
{
    GTEST_LOG_(INFO) << "TsFuncDataSourceThread called!";

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode is_blocking = napi_tsfn_nonblocking;
    int32_t sendData = SEND_DATA;
    napi_status callresult = napi_call_threadsafe_function(func, &sendData, is_blocking);
    GTEST_LOG_(INFO) << "napi_call_threadsafe_function finish!";
    EXPECT_EQ(callresult, napi_status::napi_ok);
    napi_status releaseresult = napi_release_threadsafe_function(func, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_ok);

    GTEST_LOG_(INFO) << "TsFuncDataSourceThread end!";
}

static void TsFuncDataSourceThread0200(void* data)
{
    GTEST_LOG_(INFO) << "TsFuncDataSourceThread0200 called!";

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode is_blocking = napi_tsfn_nonblocking;
    int32_t sendData = SEND_DATA;
    napi_status callresult = napi_call_threadsafe_function(func, &sendData, is_blocking);
    GTEST_LOG_(INFO) << "napi_call_threadsafe_function finish!";
    EXPECT_EQ(callresult, napi_status::napi_ok);
    for (size_t i = 0; i < MAX_COUNT; i++) {
        napi_release_threadsafe_function(func, napi_tsfn_release);
    }
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";

    GTEST_LOG_(INFO) << "TsFuncDataSourceThread0200 end!";
}

static void TsFuncreleaseThread(void* data)
{
    GTEST_LOG_(INFO) << "TsFuncreleaseThread called!";

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_status releaseresultone = napi_release_threadsafe_function(func, napi_tsfn_release);
    EXPECT_EQ(releaseresultone, napi_status::napi_ok);
    napi_status releaseresulttwo = napi_release_threadsafe_function(func, napi_tsfn_release);
    EXPECT_EQ(releaseresulttwo, napi_status::napi_generic_failure);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";

    GTEST_LOG_(INFO) << "TsFuncreleaseThread end!";
}

static void TsFuncabortThread(void* data)
{
    GTEST_LOG_(INFO) << "TsFuncabortThread called!";

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode is_blocking = napi_tsfn_nonblocking;
    int32_t sendData = SEND_DATA;
    napi_status callresultone = napi_call_threadsafe_function(func, &sendData, is_blocking);
    EXPECT_EQ(callresultone, napi_status::napi_ok);
    sleep(1);
    napi_status releaseresult = napi_release_threadsafe_function(func, napi_tsfn_abort);
    EXPECT_EQ(releaseresult, napi_status::napi_ok);
    napi_status callresulttwo = napi_call_threadsafe_function(func, &sendData, is_blocking);
    EXPECT_EQ(callresulttwo, napi_status::napi_closing);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";

    GTEST_LOG_(INFO) << "TsFuncabortThread end!";
}

static void TsFuncreleasefiveThread(void* data)
{
    GTEST_LOG_(INFO) << "TsFuncreleasefiveThread called!";

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_status releaseresult;
    size_t loopCount = 5;
    for (size_t i = 0; i < loopCount; i++) {
        releaseresult = napi_release_threadsafe_function(func, napi_tsfn_release);
        EXPECT_EQ(releaseresult, napi_status::napi_ok);
    }
    releaseresult = napi_release_threadsafe_function(func, napi_tsfn_release);
    EXPECT_EQ(releaseresult, napi_status::napi_generic_failure);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";

    GTEST_LOG_(INFO) << "TsFuncreleasefiveThread end!";
}

static void TsFuncErrReleaseThread(void* data)
{
    GTEST_LOG_(INFO) << "TsFuncErrReleaseThread called!";

    napi_status releaseresult = napi_release_threadsafe_function(nullptr, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "TsFuncErrReleaseThread end!";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0100
 * @tc.name      : Test the normal value of NAPi_CREATE_string_UTf16
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0100 start";

    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.!@#%中^&*()6666";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength + 1];
    size_t copied = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, buffer, testStrLength + 1, &copied));

    EXPECT_EQ(testStrLength, copied);
    for (size_t i = INT_ZERO; i < copied + 1; i++) {
        EXPECT_EQ(testStr[i], buffer[i]);
    }

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0200
 * @tc.name      : Tests whether napi_CREATE_string_UTf16 returns an exception if STR is empty
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0200 start";
    napi_env env = (napi_env)engine_;

    size_t buffer_size = INT_ONE;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ret = napi_create_string_utf16(env, nullptr, buffer_size, &result);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0300
 * @tc.name      : Test napi_CREATE_string_UTf16 when length is empty
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0300 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.abnormal.dd@#!#@$%999900";
    napi_value result = nullptr;
    napi_status status = napi_ok;
    status = napi_create_string_utf16(env, testStr, 0, &result);
    EXPECT_NE(status, napi_ok);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0400
 * @tc.name      : Test Napi_create_string_utf16 The string length is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0400 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.size_max.abnormal";
    napi_status ret = napi_ok;
    napi_value result = nullptr;

    ret = napi_create_string_utf16(env, testStr, (size_t)INT_MAX + 1, &result);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0500
 * @tc.name      : The napi_create_string_UTf16 returned object parameter is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0500 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.result.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    napi_status ret = napi_ok;

    ret = napi_create_string_utf16(env, testStr, testStrLength, nullptr);
    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0600
 * @tc.name      : The napi_CREATE_string_UTf16 environment parameter is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0600 start";
    char16_t testStr[] = u"system.test.content.env.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    napi_status ret = napi_ok;
    napi_value result = nullptr;

    ret = napi_create_string_utf16(nullptr, testStr, testStrLength, &result);
    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0700
 * @tc.name      : Test the special characters of NAPi_CREATE_string_UTf16
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0700, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0700 start";

    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.&*！@#￥%";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength + 1];
    size_t copied = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, buffer, testStrLength + 1, &copied));

    EXPECT_EQ(testStrLength, copied);
    for (size_t i = INT_ZERO; i < copied + 1; i++) {
        EXPECT_EQ(testStr[i], buffer[i]);
    }

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0800
 * @tc.name      : Test NAPi_CREATE_string_UTf16 space
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0800, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0800 start";

    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.     ";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength + 1];
    size_t copied = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, buffer, testStrLength + 1, &copied));

    EXPECT_EQ(testStrLength, copied);
    for (size_t i = INT_ZERO; i < copied + 1; i++) {
        EXPECT_EQ(testStr[i], buffer[i]);
    }

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0800 end";
}
/*
 * @tc.number    : ACE_Napi_Create_String_Utf16_0900
 * @tc.name      : Test NAPi_CREATE_string_UTf16 incoming Chinese characters
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_String_Utf16_0900, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0900 start";

    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.汉字输入";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength + 1];
    size_t copied = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, buffer, testStrLength + 1, &copied));

    EXPECT_EQ(testStrLength, copied);
    for (size_t i = INT_ZERO; i < copied + 1; i++) {
        EXPECT_EQ(testStr[i], buffer[i]);
    }

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_String_Utf16_0900 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0100
 * @tc.name      : Test the normal value of napi_get_value_string_utf16
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0100 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.&&&^^^^.中文9988";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength];
    size_t copied = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, buffer, testStrLength + 1, &copied));

    EXPECT_EQ(testStrLength, copied);
    for (size_t i = INT_ZERO; i < copied; i++) {
        EXPECT_EQ(testStr[i], buffer[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0200
 * @tc.name      : Test napi_get_value_string_UTf16 truncation
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0200 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.abnormal";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength];
    size_t copied;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, buffer, INT_FIVE + 1, &copied));

    for (int i = INT_ZERO; i < INT_FIVE; i++) {
        EXPECT_EQ(testStr[i], buffer[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0300
 * @tc.name      : Test napi_get_value_string_UTf16 The input buffer size is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0300 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.abnormal.!@#$%^&*123";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[] = u"12345";
    size_t buffer_size = INT_ZERO;
    size_t copied = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, buffer, buffer_size, &copied));

    EXPECT_EQ(copied, testStrLength);
    for (size_t i = INT_ZERO; i < INT_TWO; i++) {
        EXPECT_NE(buffer[i], testStr[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0400
 * @tc.name      : Test Napi_get_value_string_utf16 Invalid string passed
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Return value of function is napi_string_expected
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0400 start";
    napi_env env = (napi_env)engine_;

    char16_t buffer[INT_FIVE];
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(buffer));
    size_t copied;
    int64_t testValue = INT64_MAX;
    napi_value result = nullptr;
    napi_status ret = napi_ok;
    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    ret = napi_get_value_string_utf16(env, result, buffer, testStrLength, &copied);

    EXPECT_EQ(ret, napi_string_expected);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0500
 * @tc.name      : Test for invalid buffer for napi_get_value_string_UTf16 string
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0500 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.copied.$#@666";
    size_t testStrLength = static_cast<size_t>(std::char_traits<char16_t>::length(testStr));
    size_t copied = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ExpectCheckCall(napi_get_value_string_utf16(env, result, nullptr, testStrLength, &copied));

    EXPECT_EQ(testStrLength, copied);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0600
 * @tc.name      : The napi_get_value_string_UTf16 output length is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0600 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength];
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    ret = napi_get_value_string_utf16(env, result, buffer, testStrLength, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0700
 * @tc.name      : The napi_get_value_string_UTf16 environment parameter is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_string_utf16 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0700 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.env.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength];
    size_t copied;
    napi_value result = nullptr;
    napi_status ret = napi_ok;
    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));

    ret = napi_get_value_string_utf16(nullptr, result, buffer, testStrLength, &copied);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_String_Utf16_0800
 * @tc.name      : The napi_get_value_string_UTf16 value parameter is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_get_value_string_utf16 is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_String_Utf16_0800, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0800 start";
    napi_env env = (napi_env)engine_;

    char16_t testStr[] = u"system.test.content.result.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    char16_t buffer[testStrLength];
    size_t copied;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ret = napi_get_value_string_utf16(env, result, buffer, testStrLength, &copied);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_String_Utf16_0800 end";
}

#if  (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
    // jerryscript 2.3 do nothing
#else
// jerryscript 2.4 or quickjs or V8
/*
 * @tc.number    : ACE_Napi_Create_Bigint_Int64_0100
 * @tc.name      : Test the maximum value of napi_create_bigint_int64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Int64_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0100 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_MAX;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    int64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_int64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, INT64_MAX);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Int64_0200
 * @tc.name      : Test the minimum value of napi_create_bigint_int64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Int64_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0200 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_MIN;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    int64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_int64(env, result, &resultValue, &lossless));

    EXPECT_EQ(true, lossless);
    EXPECT_EQ(resultValue, INT64_MIN);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Int64_0300
 * @tc.name      : Test invalid environment for NAPi_CREATE_BIGINT_INT64
 * @tc.desc      : 1.Set test variables
 *                 2.The function of napi_create_bigint_int64 is called
 *                 3.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Int64_0300, testing::ext::TestSize.Level0)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0300 start";
    int64_t testValue = INT64_MAX;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ret = napi_create_bigint_int64(nullptr, testValue, &result);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Int64_0400
 * @tc.name      : Test the normal value of napi_create_bigint_int64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Int64_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0400 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_VALUE;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    int64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_int64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, testValue);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Int64_0500
 * @tc.name      : Test the napi_create_bigint_int64 function return value nullptr
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Int64_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0500 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_VALUE;
    napi_status ret = napi_ok;

    ret = napi_create_bigint_int64(env, testValue, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Int64_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Int64_0100
 * @tc.name      : Test the maximum value of napi_get_value_bigint_int64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Int64_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0100 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_MAX;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    int64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_int64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, INT64_MAX);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Int64_0200
 * @tc.name      : Test the minimum value of napi_get_value_bigint_int64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Int64_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0200 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_MIN;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    int64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_int64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, INT64_MIN);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Int64_0300
 * @tc.name      : Test passing in an object that is not Napi_bigint
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Return value of function is napi_bigint_expected
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Int64_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0300 start";
    napi_env env = (napi_env)engine_;

    napi_status ret = napi_ok;
    char16_t testStr[] = u"system.test.content.bigint.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));

    bool lossless = true;
    int64_t resultValue = INT_ZERO;
    ret = napi_get_value_bigint_int64(env, result, &resultValue, &lossless);

    EXPECT_EQ(ret, napi_bigint_expected);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Int64_0400
 * @tc.name      : Passed an invalid environment variable
 * @tc.desc      : 1.Set test variables
 *                 2.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Int64_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0400 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT_ZERO;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);
    bool lossless = true;
    int64_t resultValue = INT_ZERO;

    ret = napi_get_value_bigint_int64(nullptr, result, &resultValue, &lossless);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Int64_0500
 * @tc.name      : Test the normal value of napi_get_value_bigint_int64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Int64_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0500 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_VALUE;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    int64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_int64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, INT64_VALUE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Int64_0600
 * @tc.name      : The result value returned by test napi_get_value_bigint_int64 is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Int64_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0600 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_VALUE;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    ret = napi_get_value_bigint_int64(env, result, nullptr, &lossless);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Int64_0700
 * @tc.name      : Test napi_get_value_bigint_int64 returns JS big integer is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_int64 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Int64_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0700 start";
    napi_env env = (napi_env)engine_;

    int64_t testValue = INT64_VALUE;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ExpectCheckCall(napi_create_bigint_int64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    int64_t resultValue = INT_ZERO;
    ret = napi_get_value_bigint_int64(env, result, &resultValue, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Int64_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Uint64_0100
 * @tc.name      : Test the maximum value of napi_create_bigint_uint64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Uint64_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0100 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = UINT64_MAX;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = false;
    uint64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_uint64(env, result, &resultValue, &lossless));

    EXPECT_EQ(true, lossless);
    EXPECT_EQ(resultValue, testValue);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Uint64_0200
 * @tc.name      : Test the minimum value of napi_create_bigint_uint64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Uint64_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0200 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    uint64_t resultValue = INT_ONE;
    ExpectCheckCall(napi_get_value_bigint_uint64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, testValue);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Uint64_0300
 * @tc.name      : Passed an invalid environment variable
 * @tc.desc      : 1.Set test variables
 *                 2.The function of napi_create_bigint_uint64 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Uint64_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0300 start";
    uint64_t testValue = INT_ZERO;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ret = napi_create_bigint_uint64(nullptr, testValue, &result);
    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Uint64_0400
 * @tc.name      : Test napi_create_bigint_uint64 Normal value
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Uint64_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0400 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = UINT64_VALUE;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    uint64_t resultValue = INT_ONE;
    ExpectCheckCall(napi_get_value_bigint_uint64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, testValue);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Uint64_0500
 * @tc.name      : Test the napi_create_bigint_uint64 function return value nullptr
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Uint64_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0500 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = UINT64_VALUE;
    napi_status ret = napi_ok;

    ret = napi_create_bigint_uint64(env, testValue, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Uint64_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Uint64_0100
 * @tc.name      : Test the maximum value of napi_get_value_bigint_uint64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Uint64_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0100 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = UINT64_MAX;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    uint64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_uint64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, UINT64_MAX);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Uint64_0200
 * @tc.name      : Test the maximum value of napi_get_value_bigint_uint64
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Uint64_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0200 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = INT_ZERO;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    uint64_t resultValue = INT_ZERO;
    ExpectCheckCall(napi_get_value_bigint_uint64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, testValue);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Uint64_0300
 * @tc.name      : Pass in an object that is not BigInt
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_string_utf16 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Return value of function is napi_bigint_expected
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Uint64_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0300 start";
    napi_env env = (napi_env)engine_;

    napi_status ret = napi_ok;
    char16_t testStr[] = u"system.test.content.bigint.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));

    bool lossless = true;
    uint64_t resultValue = INT_ZERO;
    ret = napi_get_value_bigint_uint64(env, result, &resultValue, &lossless);

    EXPECT_EQ(ret, napi_bigint_expected);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Uint64_0400
 * @tc.name      : Passed an invalid environment variable
 * @tc.desc      : 1.Set test variables
 *                 2.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Uint64_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0400 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = INT_ZERO;
    napi_value result = nullptr;
    napi_status ret = napi_ok;
    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    uint64_t resultValue = INT_ZERO;
    ret = napi_get_value_bigint_uint64(nullptr, result, &resultValue, &lossless);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Uint64_0500
 * @tc.name      : Test napi_get_value_bigint_uint64 Normal value
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Uint64_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0500 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = UINT64_VALUE;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    uint64_t resultValue = INT_ONE;
    ExpectCheckCall(napi_get_value_bigint_uint64(env, result, &resultValue, &lossless));

    EXPECT_EQ(resultValue, (uint64_t)UINT64_VALUE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Uint64_0600
 * @tc.name      : The result value returned by test napi_get_value_bigint_uint64 is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Uint64_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0600 start";
    napi_env env = (napi_env)engine_;

    uint64_t testValue = UINT64_VALUE;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    bool lossless = true;
    ret = napi_get_value_bigint_uint64(env, result, nullptr, &lossless);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Uint64_0700
 * @tc.name      : Test napi_get_value_bigint_uint64 returns JS big integer is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_uint64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_uint64 is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Uint64_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0700 start";
    napi_env env = (napi_env)engine_;
    napi_status ret = napi_ok;
    uint64_t testValue = UINT64_VALUE;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_uint64(env, testValue, &result));
    AssertCheckValueType(env, result, napi_bigint);

    uint64_t resultValue = INT_ONE;
    ret = napi_get_value_bigint_uint64(env, result, &resultValue, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Uint64_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0100
 * @tc.name      : Test the normal value of NAPi_CREATE_BIGINT_WORDS and the character bit is 0
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0100 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)16, (uint64_t)128, (uint64_t)56, (uint64_t)226 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign = INT_ZERO;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, (size_t)INT_FOUR);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0200
 * @tc.name      : Test the normal value of NAPi_CREATE_BIGINT_WORDS and the character bit is 1
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0200 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)16, (uint64_t)64, (uint64_t)28, (uint64_t)8 };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign = INT_ZERO;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_FOUR);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0300
 * @tc.name      : Tests word_count for an invalid value of 0
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0300 start";
    napi_env env = (napi_env)engine_;

    int sign_bit = INT_ZERO;
    size_t word_count = INT_ZERO;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ret = napi_create_bigint_words(env, sign_bit, word_count, nullptr, &result);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0400
 * @tc.name      : Tests word_count for an invalid value of 0
 * @tc.desc      : 1.Set test variables
 *                 2.The function of napi_create_bigint_words is called
 *                 3.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0400 start";
    uint64_t words[] = { (uint64_t)128, (uint64_t)64, (uint64_t)66, (uint64_t)111 };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ret = napi_create_bigint_words(nullptr, sign_bit, word_count, words, &result);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0500
 * @tc.name      : Test napi_create_bigint_words Enter 0 and the character bit is 1
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0500 start";

    napi_env env = (napi_env)engine_;

#ifdef FOR_JERRYSCRIPT_TEST
    uint64_t words[] = { (uint64_t)5, (uint64_t)10, (uint64_t)25, (uint64_t)25 };
#else
    uint64_t words[] = { (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0 };
#endif
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);

#ifdef FOR_JERRYSCRIPT_TEST
    EXPECT_EQ(word_count, INT_FOUR);
#else
    EXPECT_EQ(word_count, INT_ONE);
#endif
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0600
 * @tc.name      : Test napi_create_bigint_words Combined data. The value is 1 character
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0600 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)1, (uint64_t)0, (uint64_t)0, (uint64_t)0 };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_ONE);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0700
 * @tc.name      : Test napi_create_bigint_words Combined data. The value is 1 character
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0700 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)1 };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_FOUR);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0800
 * @tc.name      : Tests the maximum number of words entered in napi_create_BIGINt_words and the sign bit is 1
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0800, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0800 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign = INT_ZERO;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_FOUR);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0800 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Bigint_Words_0900
 * @tc.name      : Test the normal value of NAPi_CREATE_BIGINT_WORDS,
 *                  the character bit is 0, and the return value is empty.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.Set test variables.
 *                 3.The function of napi_create_bigint_words is called.
 *                 4.Return value of function is napi_invalid_arg.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Bigint_Words_0900, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0900 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)16, (uint64_t)64, (uint64_t)28, (uint64_t)8 };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_status ret = napi_ok;

    ret = napi_create_bigint_words(env, sign_bit, word_count, words, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0900 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0100
 * @tc.name      : Test the normal value of napi_get_value_bigint_words and the character bit is 0
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0100 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { 8, 16, 34, 45, 66 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FIVE;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_FIVE);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0200
 * @tc.name      : Test the normal value of napi_get_value_bigint_words and the character bit is 1
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0200 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)12, (uint64_t)34, (uint64_t)56, 0x000000FF98765432 };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_FOUR);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0300
 * @tc.name      : Tests word_count for an invalid value of 1
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_int64 is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Return value of function is napi_object_expected
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0300 start";
    napi_env env = (napi_env)engine_;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;
    napi_status ret = napi_ok;
    char16_t testStr[] = u"system.test.content.bigint.abnormal";
    int testStrLength = static_cast<int>(std::char_traits<char16_t>::length(testStr));

    ExpectCheckCall(napi_create_string_utf16(env, testStr, testStrLength, &result));
    int sign;
    uint64_t wordsOut[word_count];

    ret = napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut);

    EXPECT_EQ(ret, napi_object_expected);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0400
 * @tc.name      : Passed an invalid environment variable
 * @tc.desc      : 1.Set test variables
 *                 2.The function of napi_create_bigint_words is called
 *                 3.Return value of function is napi_ok
 *                 4.The function of napi_get_value_bigint_words is called
 *                 5.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0400 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)13, (uint64_t)84, 0x00FF98765432, (uint64_t)254 };
    int sign_bit = INT_ONE;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;
    napi_status ret = napi_ok;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];

    ret = napi_get_value_bigint_words(nullptr, result, &sign, &word_count, wordsOut);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0500
 * @tc.name      : Test napi_get_value_bigint_words Enter 0 and the character bit is 1
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0500 start";
    napi_env env = (napi_env)engine_;
#ifdef FOR_JERRYSCRIPT_TEST
    uint64_t words[] = { (uint64_t)5, (uint64_t)10, (uint64_t)15, (uint64_t)20 };
#else
    uint64_t words[] = { (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0 };
#endif

    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(INT_ZERO, sign);
#ifdef FOR_JERRYSCRIPT_TEST
    EXPECT_EQ(word_count, INT_FOUR);
#else
    EXPECT_EQ(word_count, INT_ONE);
#endif

    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0600
 * @tc.name      : Test napi_get_value_bigint_words Combined data. The value is 0 character
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0600 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)1, (uint64_t)0, (uint64_t)0, (uint64_t)0 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_ONE);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0700
 * @tc.name      : Test napi_get_value_bigint_words Combined data. The value is 0 character
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0700 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)1 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_FOUR);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0800
 * @tc.name      : Tests the maximum number of words entered in napi_create_BIGINt_words and the sign bit is 0
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Check whether the obtained parameters meet requirements
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0800, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Bigint_Words_0800 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign = INT_ONE;
    uint64_t wordsOut[word_count];
    ExpectCheckCall(napi_get_value_bigint_words(env, result, &sign, &word_count, wordsOut));

    EXPECT_EQ(sign_bit, sign);
    EXPECT_EQ(word_count, INT_FOUR);
    for (size_t i = INT_ZERO; i < word_count; i++) {
        EXPECT_EQ(wordsOut[i], words[i]);
    }
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0800 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_0900
 * @tc.name      : Test napi_get_value_bigint_words parameter value input is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_0900, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0900 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)1, (uint64_t)20, (uint64_t)64, (uint64_t)88 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    napi_status ret = napi_get_value_bigint_words(env, nullptr, &sign, &word_count, wordsOut);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_0900 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_1000
 * @tc.name      : Test napi_get_value_bigint_words returns the sign parameter sign_bit is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_1000, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_1000 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)1, (uint64_t)20, (uint64_t)64, (uint64_t)88 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    uint64_t wordsOut[word_count];
    napi_status ret = napi_get_value_bigint_words(env, result, nullptr, &word_count, wordsOut);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_1000 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_1100
 * @tc.name      : Test that the sign parameter sign_bit returned by napi_get_value_bigint_words invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_1100, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_1100 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)1, (uint64_t)20, (uint64_t)64, (uint64_t)88 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    uint64_t wordsOut[word_count];
    napi_status ret = napi_get_value_bigint_words(env, result, &sign, nullptr, wordsOut);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_1100 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Value_Bigint_Words_1200
 * @tc.name      : Test that the parameter words of the data returned by napi_get_value_bigint_words invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_bigint_words is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_value_bigint_words is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Value_Bigint_Words_1200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_1200 start";
    napi_env env = (napi_env)engine_;

    uint64_t words[] = { (uint64_t)1, (uint64_t)20, (uint64_t)64, (uint64_t)88 };
    int sign_bit = INT_ZERO;
    size_t word_count = INT_FOUR;
    napi_value result = nullptr;

    ExpectCheckCall(napi_create_bigint_words(env, sign_bit, word_count, words, &result));

    int sign;
    napi_status ret = napi_get_value_bigint_words(env, result, &sign, &word_count, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Value_Bigint_Words_1200 end";
}
#endif // (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
/**
 * @tc.number    : ACE_Napi_Detach_Arraybuffer_0100
 * @tc.name      : The parameter is valid, the parameter is input to the buffer object, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. detach arraybuffer.
 *                 4. Verify napi_detach_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Detach_Arraybuffer_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0100 start";

    napi_env env = (napi_env)engine_;

    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    size_t compareSize = ARRAYBUFFER_SIZE_NULL;
    napi_status result = napi_get_arraybuffer_info(env, arrayBuffer, &arrayBufferPtr, &compareSize);

    EXPECT_EQ(compareSize, ARRAYBUFFER_SIZE);
    EXPECT_EQ(result, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0100 create arraybuffer complete";

    napi_status output = napi_detach_arraybuffer(env, arrayBuffer);

    bool out = false;
    napi_is_detached_arraybuffer(env, arrayBuffer, &out);
    EXPECT_TRUE(out);
    EXPECT_EQ(output, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Detach_Arraybuffer_0200
 * @tc.name      : The parameter is invalid, the parameter arraybuffer input non-buffer object, test and confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create buffer
 *                 3. detach buffer
 *                 4. Verify napi_detach_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Detach_Arraybuffer_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0200 start";

    napi_env env = (napi_env)engine_;

    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = ARRAYBUFFER_SIZE;
    napi_status verification = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(BufferPtr, nullptr);
    EXPECT_EQ(BufferSize, ARRAYBUFFER_SIZE);
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0200 create buffer complete";

    napi_status result = napi_detach_arraybuffer(env, Buffer);

    EXPECT_NE(BufferPtr, nullptr);
    EXPECT_EQ(result, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Detach_Arraybuffer_0300
 * @tc.name      : The parameter is invalid, the parameter arraybuffer input buffer object, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. detach arraybuffer.
 *                 4. Verify napi_detach_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Detach_Arraybuffer_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0300 start";

    napi_env env = (napi_env)engine_;

    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    size_t compareSize = ARRAYBUFFER_SIZE_NULL;
    napi_status result = napi_get_arraybuffer_info(env, arrayBuffer, &arrayBufferPtr, &compareSize);

    EXPECT_EQ(compareSize, ARRAYBUFFER_SIZE);
    EXPECT_EQ(result, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0300 create arraybuffer complete";

    napi_status output = napi_detach_arraybuffer(nullptr, arrayBuffer);

    EXPECT_EQ(arrayBufferSize, ARRAYBUFFER_SIZE);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Detach_Arraybuffer_0400
 * @tc.name      : The parameter is invalid, the parameter arraybuffer input date object, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. detach arraybuffer.
 *                 4. Verify napi_detach_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Detach_Arraybuffer_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0400 start";

    napi_env env = (napi_env)engine_;

    double time = CREATE_DATE_TIME;
    napi_value result = nullptr;
    napi_status verification = napi_create_date(env, time, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0400 create date complete";

    napi_status ret = napi_detach_arraybuffer(env, result);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Detach_Arraybuffer_0500
 * @tc.name      : The parameter is invalid, the parameter arraybuffer input is empty, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. detach arraybuffer.
 *                 3. Verify napi_detach_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Detach_Arraybuffer_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0500 start";

    napi_env env = (napi_env)engine_;
    napi_status result = napi_detach_arraybuffer(env, nullptr);

    EXPECT_EQ(result, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Detach_Arraybuffer_0600
 * @tc.name      : The parameter is invalid, the parameter arraybuffer input is Detached arraybuffer,
 *                 test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. detach arraybuffer.
 *                 3. Verify napi_detach_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Detach_Arraybuffer_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0600 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    size_t compareSize = ARRAYBUFFER_SIZE_NULL;
    napi_status result = napi_get_arraybuffer_info(env, arrayBuffer, &arrayBufferPtr, &compareSize);

    EXPECT_EQ(compareSize, ARRAYBUFFER_SIZE);
    EXPECT_EQ(result, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0100 create arraybuffer complete";

    napi_status output = napi_detach_arraybuffer(env, arrayBuffer);

    bool out = false;
    napi_status verificationCp = napi_is_detached_arraybuffer(env, arrayBuffer, &out);

    EXPECT_EQ(verificationCp, napi_ok);
    EXPECT_TRUE(out);
    EXPECT_EQ(output, napi_ok);

    napi_status outcompare = napi_detach_arraybuffer(env, arrayBuffer);
    EXPECT_EQ(outcompare, napi_detachable_arraybuffer_expected);

    GTEST_LOG_(INFO) << "ACE_Napi_Detach_Arraybuffer_0600 start";
}

/**
 * @tc.number    : ACE_Napi_Is_Detached_Arraybuffer_0100
 * @tc.name      : The parameter is valid, the parameter arraybuffer input buffer object, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. detach arraybuffer.
 *                 4. Verify napi_is_detached_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Detached_Arraybuffer_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0100 start";

    napi_env env = (napi_env)engine_;

    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(arrayBufferPtr, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0100 create arraybuffer complete";

    napi_status verificationCp = napi_detach_arraybuffer(env, arrayBuffer);
    EXPECT_EQ(verificationCp, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0100 detached arraybuffer complete";

    bool result = false;
    napi_status out = napi_is_detached_arraybuffer(env, arrayBuffer, &result);

    EXPECT_EQ(out, napi_ok);
    EXPECT_TRUE(result);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Detached_Arraybuffer_0200
 * @tc.name      : The parameter is valid, the parameter arraybuffer input non-buffer object, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. Verify napi_is_detached_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Detached_Arraybuffer_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0200 start";

    napi_env env = (napi_env)engine_;
    double time = CREATE_DATE_TIME;
    napi_value date = nullptr;
    napi_status verification = napi_create_date(env, time, &date);
    
    EXPECT_EQ(verification, napi_ok);

    bool result = false;
    napi_status ret = napi_is_detached_arraybuffer(env, date, &result);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Detached_Arraybuffer_0300
 * @tc.name      : The parameter is invalid, the parameter arraybuffer input buffer object, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. detach arraybuffer.
 *                 4. Verify napi_is_detached_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Detached_Arraybuffer_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0300 start";

    napi_env env = (napi_env)engine_;

    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(arrayBufferPtr, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0300 create arraybuffer complete";

    napi_status verificationCp = napi_detach_arraybuffer(env, arrayBuffer);

    EXPECT_EQ(verificationCp, napi_ok);

    bool result = true;
    napi_status ret = napi_is_detached_arraybuffer(nullptr, arrayBuffer, &result);

    EXPECT_TRUE(result);
    EXPECT_EQ(arrayBufferSize, ARRAYBUFFER_SIZE);
    EXPECT_NE(arrayBuffer, nullptr);
    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Detached_Arraybuffer_0400
 * @tc.name      : The parameter is invalid, the parameter result input is empty, test confirmation.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. detach arraybuffer.
 *                 4. Verify napi_is_detached_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Detached_Arraybuffer_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0400 start";

    napi_env env = (napi_env)engine_;

    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0400 create arraybuffer complete";

    napi_status verificationCp = napi_detach_arraybuffer(env, arrayBuffer);

    EXPECT_EQ(verificationCp, napi_ok);

    napi_status ret = napi_is_detached_arraybuffer(env, arrayBuffer, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);
    EXPECT_EQ(arrayBufferSize, ARRAYBUFFER_SIZE);
    EXPECT_NE(arrayBuffer, nullptr);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Detached_Arraybuffer_0500
 * @tc.name      : The parameter is invalid, the parameter arraybuffer input is empty, test to confirm
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Verify napi_is_detached_arraybuffer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Detached_Arraybuffer_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0500 start";

    napi_env env = (napi_env)engine_;

    bool result = true;
    napi_status ret = napi_is_detached_arraybuffer(env, nullptr, &result);

    EXPECT_TRUE(result);
    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Detached_Arraybuffer_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Date_0100
 * @tc.name      : The parameter is valid,
 *                 and the parameter value is entered in the napi_date object,
 *                 and the test is performed to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3. Verify napi_is_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Date_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0100 start";

    napi_env env = (napi_env)engine_;

    double time = CREATE_DATE_TIME;
    napi_value result = nullptr;
    napi_status verification = napi_create_date(env, time, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);

    double jsTime = 0;
    napi_status output = napi_get_date_value(env, result, &jsTime);

    EXPECT_EQ(output, napi_ok);
    EXPECT_EQ(jsTime, GET_DATE_TIME);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0100 create date complete";

    bool ret = false;
    napi_status out = napi_is_date(env, result, &ret);

    EXPECT_TRUE(ret);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Date_0200
 * @tc.name      : The parameter is invalid,
 *                 the value of the parameter is not a napi_date object,
 *                 and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. Verify napi_is_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Date_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0200 start";

    napi_env env = (napi_env)engine_;

    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(arrayBufferPtr, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0200 create arraybuffer complete";

    size_t compareSize = ARRAYBUFFER_SIZE_NULL;
    napi_status result = napi_get_arraybuffer_info(env, arrayBuffer, &arrayBufferPtr, &compareSize);

    EXPECT_EQ(compareSize, ARRAYBUFFER_SIZE);
    EXPECT_EQ(result, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0200 get arraybuffer info complete";

    bool ret = true;
    napi_status out = napi_is_date(env, arrayBuffer, &ret);

    EXPECT_FALSE(ret);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Date_0300
 * @tc.name      : The parameter is invalid,
 *                 the parameter value is entered in the napi_date object,
 *                 and the test is confirmed
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3. Verify napi_is_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Date_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0300 start";

    napi_env env = (napi_env)engine_;

    double time = CREATE_DATE_TIME;
    napi_value result = nullptr;
    napi_status verification = napi_create_date(env, time, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);

    double jsTime = 0;
    napi_status out = napi_get_date_value(env, result, &jsTime);

    EXPECT_EQ(out, napi_ok);
    EXPECT_EQ(jsTime, GET_DATE_TIME);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0300 create date complete";

    bool ret = false;
    napi_status output = napi_is_date(nullptr, result, &ret);

    EXPECT_FALSE(ret);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Date_0400
 * @tc.name      : The parameter is invalid,
 *                 and the parameter value is entered as the date object that failed to be created,
 *                 and the test is performed to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Verify napi_is_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Date_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0400 start";

    napi_env env = (napi_env)engine_;

    bool ret = false;
    napi_status out = napi_is_date(env, nullptr, &ret);

    EXPECT_FALSE(ret);
    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Date_0500
 * @tc.name      : The parameter is invalid. Enter 0 for the parameter value to confirm the test.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3. Verify napi_is_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Date_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0500 start";

    napi_env env = (napi_env)engine_;

    double time = DATE_TIME_VALUE;
    napi_value result = nullptr;
    napi_status out = napi_create_date(env, time, &result);

    EXPECT_EQ(out, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0500 create date value complete";

    napi_status output = napi_is_date(env, result, nullptr);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Is_Date_0600
 * @tc.name      : The parameter is valid, input the character string ("汉字测试") for
 *                 the parameter value, and perform the test to confirm.
 * @tc.desc      : 1. Install the app.
 *                 2. Start the application.
 *                 3. Call napi_create_string_utf8.
 *                 4. Call napi_is_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Date_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0600 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_string_utf8(env, "汉字测试", NAPI_AUTO_LENGTH, &result);

    EXPECT_EQ(verification, napi_ok);

    bool ret = true;
    napi_status out = napi_is_date(env, result, &ret);

    EXPECT_EQ(out, napi_ok);
    EXPECT_FALSE(ret);

    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0600 end";
}

/**
 * @tc.number    : ACE_Napi_Create_Date_0100
 * @tc.name      : The parameter is valid,
 *                 enter the double value for the parameter time, and perform the test to confirm
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3.Verify napi_create_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Date_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0100 start";

    napi_env env = (napi_env)engine_;

    time_t date = time(nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Is_Date_0100 time = " << date;

    napi_value result = nullptr;
    napi_status ret = napi_create_date(env, date, &result);

    double jsTime = 0;
    napi_status out = napi_get_date_value(env, result, &jsTime);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(date, jsTime);
    
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0100 end";
}


/**
 * @tc.number    : ACE_Napi_Create_Date_0200
 * @tc.name      : The parameter is invalid. Enter a double value for the time parameter to confirm the test.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3.Verify napi_create_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Date_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0200 start";

    double time = CREATE_DATE_TIME;
    napi_value result = nullptr;
    napi_status ret = napi_create_date(nullptr, time, &result);

    EXPECT_EQ(result, nullptr);
    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Create_Date_0300
 * @tc.name      : The parameter is invalid, the parameter time is 0, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3.Verify napi_create_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Date_0300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0300 start";

    napi_env env = (napi_env)engine_;

    double time = DATE_TIME_VALUE;
    napi_value result = nullptr;
    napi_status ret = napi_create_date(env, time, &result);

    double jsTime = 1;
    napi_status out = napi_get_date_value(env, result, &jsTime);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(time, jsTime);
    
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Create_Date_0400
 * @tc.name      : The parameter is invalid, the parameter time is char, test to confirm
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3.Verify napi_create_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Date_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0400 start";

    napi_env env = (napi_env)engine_;

    char time = 'a';
    napi_value result = nullptr;
    napi_status ret = napi_create_date(env, time, &result);

    double jsTime = 0;
    napi_status out = napi_get_date_value(env, result, &jsTime);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(time, jsTime);
    
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Create_Date_0500
 * @tc.name      : The parameter is invalid, the parameter time is empty, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3.Verify napi_create_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Date_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0500 start";

    napi_env env = (napi_env)engine_;

    double time = CREATE_DATE_TIME;
    napi_status ret = napi_create_date(env, time, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Create_Date_0600
 * @tc.name      : The parameter is invalid, the parameter time is -1, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3.Verify napi_create_date.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Date_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0600 start";

    napi_env env = (napi_env)engine_;

    double time = -1;
    napi_value result = nullptr;
    napi_status ret = napi_create_date(env, time, &result);

    double jsTime = 0;
    napi_status out = napi_get_date_value(env, result, &jsTime);
    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0600 time = " << jsTime;

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(jsTime, -1);

    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Create_Date_0600 end";
}

/**
 * @tc.number    : ACE_Napi_Get_Date_Value_0100
 * @tc.name      : The parameter is valid,
 *                 and the parameter value is entered in the napi_date object, and the test is performed to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3. get date value.
 *                 4.Verify napi_get_date_value.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Date_Value_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0100 start";

    napi_env env = (napi_env)engine_;

    double time = CREATE_DATE_TIME;
    napi_value result = nullptr;
    napi_status verification = napi_create_date(env, time, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0100 create date complete";

    double ret = DATE_TIME_VALUE;
    napi_status out = napi_get_date_value(env, result, &ret);

    EXPECT_EQ(ret, GET_DATE_TIME);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Get_Date_Value_0200
 * @tc.name      : Valid environment variable env, parameter value input non-napi_date object.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create arraybuffer.
 *                 3. get date value.
 *                 4.Verify napi_get_date_value.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Date_Value_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0200 start";

    napi_env env = (napi_env)engine_;

    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(arrayBufferPtr, nullptr);
    EXPECT_NE(arrayBuffer, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0200 create arraybuffer complete";

    double ret = DATE_TIME_VALUE;
    napi_status out = napi_get_date_value(env, arrayBuffer, &ret);

    EXPECT_EQ(ret, DATE_TIME_VALUE);
    EXPECT_EQ(out, napi_date_expected);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Get_Date_Value_0300
 * @tc.name      : Invalid environment variable env, parameter value input napi_date object.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3. get date value.
 *                 4.Verify napi_get_date_value.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Date_Value_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0300 start";

    napi_env env = (napi_env)engine_;

    double time = CREATE_DATE_TIME;
    napi_value result = nullptr;
    napi_status verification = napi_create_date(env, time, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0300 create date complete";

    double ret = DATE_TIME_VALUE;
    napi_status out = napi_get_date_value(nullptr, result, &ret);

    EXPECT_EQ(ret, DATE_TIME_VALUE);
    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Get_Date_Value_0400
 * @tc.name      : The parameter is invalid, the parameter return is empty, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3. get date value.
 *                 4.Verify napi_get_date_value.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Date_Value_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0400 start";

    napi_env env = (napi_env)engine_;

    double time = CREATE_DATE_TIME;
    napi_value result = nullptr;
    napi_status verification = napi_create_date(env, time, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0400 create date complete";

    napi_status out = napi_get_date_value(env, result, nullptr);

    EXPECT_NE(result, nullptr);
    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Get_Date_Value_0500
 * @tc.name      : The parameter is invalid, the parameter value is the date when the creation failed, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. create date.
 *                 3. get date value.
 *                 4. Verify napi_get_date_value.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Date_Value_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0500 start";

    napi_env env = (napi_env)engine_;

    double ret = DATE_TIME_VALUE;
    napi_status out = napi_get_date_value(env, nullptr, &ret);

    EXPECT_EQ(ret, DATE_TIME_VALUE);
    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_Date_Value_0500 end";
}
#if  (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
    // jerryscript 2.3 do nothing
#else
// jerryscript 2.4 or quickjs or V8
/**
 * @tc.number    : ACE_Napi_Type_Tag_Object_0100
 * @tc.name      : The parameter is valid,
 *                 and the parameter js_object js_object is entered in the js object
 *                 that is not associated with type_tag, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Verify napi_type_tag_object.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Type_Tag_Object_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0100 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0100 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status out = napi_type_tag_object(env, result, &typeTag);

    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Type_Tag_Object_0200
 * @tc.name      : The parameter is valid,
 *                 the parameter js_object js_object enters the js object
 *                 associated with type_tag, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 3. Mark object
 *                 4. Verify napi_type_tag_object.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Type_Tag_Object_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0200 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0200 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status ret = napi_type_tag_object(env, result, &typeTag);

    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0200 type tag obiect complete";

    const napi_type_tag typeTagcopy = { 12312, 12 };
    napi_status out = napi_type_tag_object(env, result, &typeTagcopy);

    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Type_Tag_Object_0300
 * @tc.name      : The parameter is invalid,
 *                 the parameter js_object js_object is entered as a js object
 *                 that is not associated with type_tag, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Verify napi_type_tag_object.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Type_Tag_Object_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0300 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0200 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status out = napi_type_tag_object(nullptr, result, &typeTag);

    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Type_Tag_Object_0400
 * @tc.name      : The parameter is invalid,
 *                 the parameter js_object js_object object input is empty, test to confirm.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Mark object
 *                 3. Verify napi_type_tag_object.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Type_Tag_Object_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0400 start";

    napi_env env = (napi_env)engine_;

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status out = napi_type_tag_object(env, nullptr, &typeTag);

    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Type_Tag_Object_0500
 * @tc.name      : The parameter is invalid,
 *                 the parameter napi_type_tag* object input is empty, test confirmation.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Verify napi_type_tag_object.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Type_Tag_Object_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0500 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0500 create obiect complete";

    napi_status out = napi_type_tag_object(env, result, nullptr);

    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Type_Tag_Object_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Check_Object_Type_Tag_0100
 * @tc.name      : The parameter is valid,
 *                 the js_object object enters the js object
 *                 that is not associated with type_tag, and the parameter type_tag
 *                 enters a valid parameter, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Verify napi_check_object_type_tag.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Check_Object_Type_Tag_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0100 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0100 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status ret = napi_type_tag_object(env, result, &typeTag);

    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0100 type tag obiect obiect complete";

    bool out = false;
    napi_status verificationCp = napi_check_object_type_tag(env, result, &typeTag, &out);

    EXPECT_EQ(verificationCp, napi_ok);
    EXPECT_TRUE(out);

    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Check_Object_Type_Tag_0200
 * @tc.name      : The parameter is valid,
 *                 the js_object object enters the js object
 *                 that is not associated with type_tag, and the parameter type_tag
 *                 enters a valid parameter, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Verify napi_check_object_type_tag.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Check_Object_Type_Tag_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0200 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0100 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    bool out = true;
    napi_status verificationCp = napi_check_object_type_tag(env, result, &typeTag, &out);

    EXPECT_EQ(verificationCp, napi_ok);
    EXPECT_FALSE(out);

    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Check_Object_Type_Tag_0300
 * @tc.name      : The parameter is invalid,
 *                 the js_object object enters a js object that is not associated with type_tag,
 *                 and the parameter type_tag enters a valid parameter, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Verify napi_check_object_type_tag.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Check_Object_Type_Tag_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0300 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0300 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status ret = napi_type_tag_object(env, result, &typeTag);

    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0300 type tag obiect obiect complete";

    bool out = false;
    napi_status output = napi_check_object_type_tag(nullptr, result, &typeTag, &out);

    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Check_Object_Type_Tag_0400
 * @tc.name      : The parameter is invalid,
 *                 the js_object object enters the js object associated with type_tag,
 *                 and the parameter type_tag enters a valid parameter, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Create object.
 *                 5. Verify napi_check_object_type_tag.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Check_Object_Type_Tag_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0400 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0400 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status ret = napi_type_tag_object(env, result, &typeTag);

    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0400 type tag obiect obiect complete";

    napi_value resultcopy = nullptr;
    napi_status verifi =napi_create_object(env, &resultcopy);

    EXPECT_EQ(verifi, napi_ok);
    EXPECT_NE(resultcopy, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0400 create objectcopy complete";

    bool out = true;
    napi_status verificationCp = napi_check_object_type_tag(env, resultcopy, &typeTag, &out);

    EXPECT_EQ(verificationCp, napi_ok);
    EXPECT_FALSE(out);

    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Check_Object_Type_Tag_0500
 * @tc.name      : The parameter is invalid,
 *                 the input of the napi_type_tag* object is empty, test confirmation.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Verify napi_check_object_type_tag.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Check_Object_Type_Tag_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0500 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0500 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status ret = napi_type_tag_object(env, result, &typeTag);

    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0400 type tag obiect obiect complete";

    bool out = false;
    napi_status output = napi_check_object_type_tag(env, result, nullptr, &out);

    EXPECT_FALSE(out);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Check_Object_Type_Tag_0600
 * @tc.name      : Valid environment env, parameter type_tag object input has been associated with type_tag js object,
 *                 parameter type_tag input is valid, the returned comparison result is invalid (nullptr).
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Create object.
 *                 3. Mark object
 *                 4. Verify napi_check_object_type_tag.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Check_Object_Type_Tag_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0600 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0600 create obiect complete";

    const napi_type_tag typeTag = { 0xFFFFFFFFFFFFFFFF, 34ULL };
    napi_status ret = napi_type_tag_object(env, result, &typeTag);

    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0600 type tag obiect obiect complete";

    napi_status out = napi_check_object_type_tag(env, result, &typeTag, nullptr);

    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Check_Object_Type_Tag_0600 end";
}
#endif // (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0100
 * @tc.name      : The necessary parameters of napi_add_finalizer are valid, and callback functions can be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0100 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    char16_t nativeObj[] = u"napiaddfinalizer";
    int finalizeHine = 0;
    static bool testValue = false;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_ref result = nullptr;
    napi_status ret = napi_add_finalizer(env, jsObj, &nativeObj, [](napi_env env, void* data, void* hint) {
        testValue = true;
    }, &finalizeHine, &result);

    EXPECT_EQ(ret, napi_ok);

    napi_status verifi = napi_delete_reference(env, result);

    EXPECT_TRUE(testValue);
    EXPECT_EQ(verifi, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0200
 * @tc.name      : The JS object parameter of napi_add_finalizer that binds local data is invalid,
 *                 and the callback function cannot be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0200 start";

    napi_env env = (napi_env)engine_;

    char16_t nativeObj[] = u"napiaddfinalizer";
    int finalizeHine = 0;
    static bool testValue = false;

    napi_ref result = nullptr;
    napi_status ret = napi_add_finalizer(env, nullptr, &nativeObj, [](napi_env env, void* data, void* hint) {
        testValue = true;
    }, &finalizeHine, &result);

    EXPECT_EQ(ret, napi_invalid_arg);
    EXPECT_FALSE(testValue);

    napi_status output = napi_delete_reference(env, result);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0300
 * @tc.name      : The callback function parameter of napi_add_finalizer used to release local data is invalid,
 *                 and the callback function cannot be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0300 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    char16_t nativeObj[] = u"napiaddfinalizer";
    int finalizeHine = 0;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_ref result = nullptr;
    napi_status ret = napi_add_finalizer(env, jsObj, &nativeObj, nullptr, &finalizeHine, &result);

    EXPECT_EQ(ret, napi_invalid_arg);

    napi_status output = napi_delete_reference(env, result);
    EXPECT_EQ(output, napi_invalid_arg);
    
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0400
 * @tc.name      : The environment parameter of napi_add_finalizer is invalid,
 *                 the callback function cannot be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0400 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    char16_t nativeObj[] = u"napiaddfinalizer";
    int finalizeHine = 0;
    static bool testValue = false;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_ref result = nullptr;
    napi_status ret = napi_add_finalizer(nullptr, jsObj, &nativeObj, [](napi_env env, void* data, void* hint) {
        testValue = true;
    }, &finalizeHine, &result);

    EXPECT_EQ(ret, napi_invalid_arg);
    EXPECT_FALSE(testValue);

    napi_status output = napi_delete_reference(env, result);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0500
 * @tc.name      : The necessary parameters of napi_add_finalizer are valid,
 *                 the local data bound to the JS object is valid, and callback functions can be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0500 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    char16_t nativeObj[] = u"napiaddfinalizer";
    int finalizeHine = 0;
    static bool testValue = false;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_status ret = napi_add_finalizer(env, jsObj, &nativeObj, [](napi_env env, void* data, void* hint) {
        testValue = true;
    }, &finalizeHine, nullptr);

    EXPECT_EQ(ret, napi_ok);
    EXPECT_FALSE(testValue);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0600
 * @tc.name      : The necessary parameters of napi_add_finalizer are valid,
 *                 and the local data bound to the JS object is invalid. Callback functions can be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0600 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    char16_t nativeObj[] = u"napiaddfinalizer";
    static bool testValue = false;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_status ret = napi_add_finalizer(env, jsObj, &nativeObj, [](napi_env env, void* data, void* hint) {
        testValue = true;
    }, nullptr, nullptr);

    EXPECT_EQ(ret, napi_ok);
    EXPECT_FALSE(testValue);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0600 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0700
 * @tc.name      : The necessary parameters of napi_add_finalizer are valid,
 *                 the callback function parameter is valid, and the callback function can be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0700 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    int finalizeHine = 0;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_status ret = napi_add_finalizer(env, jsObj, nullptr, nullptr, &finalizeHine, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0700 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0800
 * @tc.name      : The necessary parameters of napi_add_finalizer are valid,
 *                 and the callback function parameter is invalid. Callback function can be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0800, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0800 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    int finalizeHine = 0;
    static bool testValue = false;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_ref result = nullptr;
    napi_status ret = napi_add_finalizer(env, jsObj, nullptr, [](napi_env env, void* data, void* hint) {
        testValue = true;
    }, &finalizeHine, &result);

    EXPECT_EQ(ret, napi_ok);

    napi_status output = napi_delete_reference(env, result);
    EXPECT_EQ(output, napi_ok);
    EXPECT_TRUE(testValue);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0800 end";
}

/**
 * @tc.number    : ACE_Napi_Add_Finalizer_0900
 * @tc.name      : The necessary parameters of napi_add_finalizer are valid,
 *                 the reference of the returned JS object is valid, and the callback function can be added.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Finalizer_0900, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0900 start";

    napi_env env = (napi_env)engine_;

    napi_value jsObj = nullptr;
    int finalizeHine = 0;

    napi_status verification = napi_create_object(env, &jsObj);

    EXPECT_EQ(verification, napi_ok);

    napi_ref result = nullptr;
    napi_status ret = napi_add_finalizer(env, jsObj, nullptr, nullptr, &finalizeHine, &result);

    EXPECT_EQ(ret, napi_invalid_arg);

    napi_status output = napi_delete_reference(env, result);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Add_Finalizer_0900 end";
}

/**
 * @tc.number    : ACE_Napi_Adjust_External_Memory_0100
 * @tc.name      : The necessary parameters of napi_adjust_external_memory
 *                 are valid and the function can be called successfully.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Adjust_External_Memory_0100, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Adjust_External_Memory_0100 start";

    napi_env env = (napi_env)engine_;

    int64_t change_in_bytes = CHANGE_IN_BYTES;
    int64_t adjusted_value = ADJUSTED_VALUE;

    napi_status ret = napi_adjust_external_memory(env, change_in_bytes, &adjusted_value);

    EXPECT_EQ(ret, napi_ok);

    GTEST_LOG_(INFO) << "ACE_Napi_Adjust_External_Memory_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Adjust_External_Memory_0200
 * @tc.name      : The size parameter of napi_adjust_external_memory's return
 *                 instance allocated memory is invalid, the function is called.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Adjust_External_Memory_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Adjust_External_Memory_0200 start";

    napi_env env = (napi_env)engine_;

    int64_t change_in_bytes = CHANGE_IN_BYTES;

    napi_status ret = napi_adjust_external_memory(env, change_in_bytes, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Adjust_External_Memory_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Adjust_External_Memory_0300
 * @tc.name      : The environment parameter of napi_adjust_external_memory is invalid, the function is called.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_add_finalizer.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Adjust_External_Memory_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Adjust_External_Memory_0300 start";

    int64_t change_in_bytes = CHANGE_IN_BYTES;
    int64_t adjusted_value = ADJUSTED_VALUE;

    napi_status ret = napi_adjust_external_memory(nullptr, change_in_bytes, &adjusted_value);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Adjust_External_Memory_0300 end";
}
#if  (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
    // jerryscript 2.3 do nothing
#else
// jerryscript 2.4 or quickjs or V8
/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0100
 * @tc.name      : The parameter is valid,
 *                 the parameter object enters the js object with the defined properties,
 *                 the parameter key_mode enters napi_key_include_prototypes,
 *                 the parameter key_filter enters napi_key_all_properties,
 *                 the parameter key_conversion enters napi_key_keep_numbers, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_get_array_length.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0100 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_status verifiint = napi_create_int32(env, testNumber, &numberAttribute);

    EXPECT_EQ(verifiint, napi_ok);

    napi_status verifiname = napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    EXPECT_EQ(verifiname, napi_ok);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    napi_status ret = napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames);

    EXPECT_EQ(ret, napi_ok);

    uint32_t length = 0;
    napi_status verifilen = napi_get_array_length(env, propNames, &length);

    EXPECT_EQ(verifilen, napi_ok);
    EXPECT_EQ(length, (size_t)1);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0100 end";
}

/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0200
 * @tc.name      : The parameter is invalid,
 *                 the parameter object enters the js object with defined properties,
 *                 the parameter key_mode enters napi_key_own_only,
 *                 the parameter key_filter enters napi_key_all_properties,
 *                 the parameter key_conversion enters napi_key_keep_numbers, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_get_array_length.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0200 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_status verifiint = napi_create_int32(env, testNumber, &numberAttribute);

    EXPECT_EQ(verifiint, napi_ok);

    napi_status verifiname = napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    EXPECT_EQ(verifiname, napi_ok);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    napi_status ret = napi_get_all_property_names(nullptr, result, keyMode, keyFilter, keyConversion, &propNames);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0200 end";
}

/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0300
 * @tc.name      : The parameter is invalid,
 *                 the parameter object enters the js object with the defined properties,
 *                 the parameter key_mode enters napi_key_own_only,
 *                 the parameter key_filter enters napi_key_all_properties,
 *                 the parameter key_conversion enters napi_key_keep_numbers,
 *                 the parameter result enters empty, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_get_array_length.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0300 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_status verifiint =napi_create_int32(env, testNumber, &numberAttribute);

    EXPECT_EQ(verifiint, napi_ok);

    napi_status verifiname =napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    EXPECT_EQ(verifiname, napi_ok);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_status ret = napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, nullptr);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0300 end";
}

/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0400
 * @tc.name      : The parameter is invalid,
 *                 the parameter object is empty,
 *                 the parameter key_mode is entered napi_key_own_only,
 *                 the parameter key_filter is entered napi_key_all_properties,
 *                 the parameter key_conversion is entered napi_key_keep_numbers,
 *                 and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_get_array_length.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0400 start";

    napi_env env = (napi_env)engine_;

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    napi_status ret = napi_get_all_property_names(env, nullptr, keyMode, keyFilter, keyConversion, &propNames);

    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0400 end";
}

/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0500
 * @tc.name      : The parameter is invalid,
 *                 the parameter object enters the js object with the defined properties,
 *                 the parameter key_mode enters napi_key_include_prototypes,
 *                 the parameter key_filter enters napi_key_numbers_to_strings,
 *                 the parameter key_conversion enters napi_key_keep_numbers.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_get_array_length.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0500 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_status verifiint = napi_create_int32(env, testNumber, &numberAttribute);

    EXPECT_EQ(verifiint, napi_ok);

    napi_status verifiname = napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    EXPECT_EQ(verifiname, napi_ok);

    napi_key_collection_mode keyMode = napi_key_include_prototypes;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_numbers_to_strings;
    napi_value propNames = nullptr;
    napi_status ret = napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames);

    EXPECT_EQ(ret, napi_ok);

    uint32_t length = 0;
    napi_status verifilen = napi_get_array_length(env, propNames, &length);

    EXPECT_EQ(verifilen, napi_ok);
    EXPECT_EQ(length, (size_t)1);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0500 end";
}

/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0600
 * @tc.name      : The parameter is valid,
 *                 the parameter object enters the js object with the defined properties,
 *                 the parameter key_mode enters napi_key_include_prototypes,
 *                 the parameter key_filter enters napi_key_writable,
 *                 the parameter key_conversion enters napi_key_keep_numbers, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_get_array_length.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0600 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_status verifiint = napi_create_int32(env, testNumber, &numberAttribute);

    EXPECT_EQ(verifiint, napi_ok);

    napi_status verifiname = napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    EXPECT_EQ(verifiname, napi_ok);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_writable;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    napi_status ret = napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames);

    EXPECT_EQ(ret, napi_ok);

    uint32_t length = 0;
    napi_status verifilen = napi_get_array_length(env, propNames, &length);

    EXPECT_EQ(verifilen, napi_ok);
    EXPECT_EQ(length, (size_t)1);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0600 end";
}

/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0700
 * @tc.name      : The parameter is valid,
 *                 the parameter object enters the js object with the defined properties,
 *                 the parameter key_mode enters napi_key_include_prototypes,
 *                 the parameter key_filter enters napi_key_enumerable,
 *                 the parameter key_conversion enters napi_key_keep_numbers, and the test is confirmed.
 * @tc.desc      : 1. Set up the env environment.
 *                 2. Configuration parameter.
 *                 3. Verify napi_get_array_length.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0700, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0700 start";

    napi_env env = (napi_env)engine_;

    napi_value result = nullptr;
    napi_status verification = napi_create_object(env, &result);

    EXPECT_EQ(verification, napi_ok);

    int32_t testNumber = 12345;
    napi_value numberAttribute = nullptr;
    napi_status verifiint = napi_create_int32(env, testNumber, &numberAttribute);

    EXPECT_EQ(verifiint, napi_ok);

    napi_status verifiname = napi_set_named_property(env, result, "numberAttribute", numberAttribute);

    EXPECT_EQ(verifiname, napi_ok);

    napi_key_collection_mode keyMode = napi_key_own_only;
    napi_key_filter keyFilter = napi_key_enumerable;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    napi_status ret = napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames);

    EXPECT_EQ(ret, napi_ok);

    uint32_t length = 0;
    napi_status verifilen = napi_get_array_length(env, propNames, &length);

    EXPECT_EQ(verifilen, napi_ok);
    EXPECT_EQ(length, (size_t)1);

    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0700 end";
}

/**
 * @tc.number    : ACE_Napi_Get_All_Property_Names_0800
 * @tc.name      : Invalid parameter, invalid parameter object input (non-object object, for example: string),
 *                 parameter key_mode input napi_key_include_prototypes,
 *                 parameter key_filter input napi_key_all_properties,
 *                 parameter key_conversion input napi_key_keep_numbers, test confirmation.
 * @tc.desc      : 1. Install the app.
 *                 2. Start the application.
 *                 3. Call napi_create_string_utf8 to create a string.
 *                 4. Call napi_get_all_property_names.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_All_Property_Names_0800, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0800 start";

    napi_env env = (napi_env)engine_;
    napi_value result = nullptr;
    napi_status verification =napi_create_string_utf8(env, "napi_get_all_property_namesTest",
        NAPI_AUTO_LENGTH, &result);

    EXPECT_EQ(verification, napi_ok);

    napi_key_collection_mode keyMode = napi_key_include_prototypes;
    napi_key_filter keyFilter = napi_key_all_properties;
    napi_key_conversion keyConversion = napi_key_keep_numbers;
    napi_value propNames = nullptr;
    napi_status ret = napi_get_all_property_names(env, result, keyMode, keyFilter, keyConversion, &propNames);

    EXPECT_EQ(ret, napi_object_expected);
    GTEST_LOG_(INFO) << "ACE_Napi_Get_All_Property_Names_0800 end";
}
#endif // (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
/*
 * @tc.number    : ACE_Napi_Create_Buffer_0100
 * @tc.name      : The parameters of napi_create_buffer are valid,
 *                 The parameter of buffer size is the minimum value (1),
 *                 A node::Buffer object can be created
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is called
 *                 4.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0100 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    GTEST_LOG_(INFO) << "BufferPtr is" << BufferPtr;
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    EXPECT_NE(BufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_0200
 * @tc.name      : The parameter of buffer size is the over maxnum value (2G),
 *                 A node::Buffer object can  not be created
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = BUFFER_OVERMAX_SIZE;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    GTEST_LOG_(INFO) << "BufferPtr is" << BufferPtr;
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(BufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_0300
 * @tc.name      : The parameter of buffer size is invalid(-1),
 *                 A node::Buffer object can  not be created
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = -1;

    GTEST_LOG_(INFO) << "BufferSize is" << BufferSize;
    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    GTEST_LOG_(INFO) << "BufferPtr is" << BufferPtr;
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(BufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_0400
 * @tc.name      : The parameter of buffer size is invalid(0),
 *                 A node::Buffer object can  not be created
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 0;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    GTEST_LOG_(INFO) << "BufferPtr is" << BufferPtr;
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(BufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_0500
 * @tc.name      : Raw pointer to the underlying buffer is invalid(nullptr),
 *                 A node::Buffer object can  not be created
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_0600
 * @tc.name      : The parameter of result is invalid(nullptr),
 *                 A node::Buffer object can  not be created
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0600 start";

    napi_env env = (napi_env)engine_;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, nullptr);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_0700
 * @tc.name      : The parameter of environment  is invalid(nullptr),
 *                 A node::Buffer object can  not be created
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0700 start";

    napi_env env = nullptr;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Is_Buffer_0100
 * @tc.name      : The parameters of napi_is_buffer are valid,
 *                 The buffer is created by napi_create_buffer function,
 *                 Checks if the Object passed in is a buffer.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is used to create a new buffer
 *                 4.The function of napi_is_buffer is called
 *                 5.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Buffer_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0100 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    bool isBuffer = false;
    napi_status isresult = napi_is_buffer(env, Buffer, &isBuffer);
    EXPECT_EQ(isresult, napi_status::napi_ok);
    EXPECT_TRUE(isBuffer);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Is_Buffer_0200
 * @tc.name      : The parameters of napi_is_buffer are valid,
 *                 The buffer is created by napi_create_buffer_copy function,
 *                 Checks if the Object passed in is a buffer.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is used to create a new buffer
 *                 4.The function of napi_is_buffer is called
 *                 5.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Buffer_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    bool isBuffer = false;
    napi_status isresult = napi_is_buffer(env, Buffer, &isBuffer);
    EXPECT_EQ(isresult, napi_status::napi_ok);
    EXPECT_TRUE(isBuffer);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Is_Buffer_0300
 * @tc.name      : The parameters of napi_is_buffer are valid,
 *                 The buffer is created by napi_create_external_buffer function,
 *                 Checks if the Object passed in is a buffer.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_external_buffer is used to create a new buffer
 *                 4.The function of napi_is_buffer is called
 *                 5.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Buffer_0300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_external_buffer(env, BufferSize, Text, BufferFinalizer, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    bool isBuffer = false;
    napi_status isresult = napi_is_buffer(env, Buffer, &isBuffer);
    EXPECT_EQ(isresult, napi_status::napi_ok);
    EXPECT_TRUE(isBuffer);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Is_Buffer_0400
 * @tc.name      : The JavaScript value to check is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is used to create a invalid buffer
 *                 4.The function of napi_is_buffer is called
 *                 5.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Buffer_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = -1;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    GTEST_LOG_(INFO) << "BufferPtr is" << BufferPtr;
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(BufferPtr, nullptr);
    bool isBuffer = false;
    napi_status isresult = napi_is_buffer(env, Buffer, &isBuffer);
    EXPECT_EQ(isresult, napi_status::napi_invalid_arg);
    EXPECT_FALSE(isBuffer);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Is_Buffer_0500
 * @tc.name      : The out parameter of result is invalid(nullptr)
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is used to create a valid buffer
 *                 4.The function of napi_is_buffer is called
 *                 5.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Buffer_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    napi_status isresult = napi_is_buffer(env, Buffer, nullptr);
    EXPECT_EQ(isresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Is_Buffer_0600
 * @tc.name      : The parameter of environment is invalid(nullptr)
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is used to create a valid buffer
 *                 4.The function of napi_is_buffer is called
 *                 5.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Is_Buffer_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0600 start";

    napi_env envone = (napi_env)engine_;
    napi_env envtwo = nullptr;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(envone, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    bool isBuffer = false;
    napi_status isresult = napi_is_buffer(envtwo, Buffer, &isBuffer);
    EXPECT_EQ(isresult, napi_status::napi_invalid_arg);
    EXPECT_FALSE(isBuffer);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Is_Buffer_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Buffer_Info_0100
 * @tc.name      : The parameters of napi_is_buffer are valid,
 *                 The buffer is created by napi_create_buffer function,
 *                 Checks if the Object passed in is a buffer.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer is used to create a new buffer
 *                 4.The function of napi_get_buffer_info is called
 *                 5.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Buffer_Info_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0100 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    EXPECT_EQ(BufferPtr, tmpBufferPtr);
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, ""), 0);
    EXPECT_EQ(BufferSize, BufferLength);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Buffer_Info_0200
 * @tc.name      : The parameter of value is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is used to create a invalid buffer
 *                 4.The function of napi_get_buffer_info is called
 *                 5.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Buffer_Info_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, "hello world"), 0);
    EXPECT_EQ(result_data, tmpBufferPtr);
    EXPECT_EQ(BufferSize, BufferLength);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Buffer_Info_0300
 * @tc.name      : The parameter of value is invalid(nullptr)
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_get_buffer_info is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Buffer_Info_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;

    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(nullptr, tmpBufferPtr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Buffer_Info_0400
 * @tc.name      : The parameters of data buffer and length are invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_get_buffer_info is called
 *                 4.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Buffer_Info_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, nullptr, &BufferLength);
    GTEST_LOG_(INFO) << "BufferLength" << BufferLength;
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    EXPECT_EQ(BufferLength, BufferSize);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Buffer_Info_0500
 * @tc.name      : The parameters of length is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_get_buffer_info is called
 *                 4.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Buffer_Info_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(env, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, nullptr);
    GTEST_LOG_(INFO) << "tmpBufferPtr" << tmpBufferPtr;
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, ""), 0);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    EXPECT_EQ(tmpBufferPtr, BufferPtr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Get_Buffer_Info_0600
 * @tc.name      : The parameters of environment is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_get_buffer_info is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Buffer_Info_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0600 start";

    napi_env envone = (napi_env)engine_;
    napi_env envtwo = nullptr;
    napi_value Buffer = nullptr;
    void* BufferPtr = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer(envone, BufferSize, &BufferPtr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(envtwo, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Buffer_Info_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0100
 * @tc.name      : The data of raw pointer to the underlying buffer to copy from is invalid.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0100, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0100 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = 1;

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, nullptr, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(tmpBufferPtr, nullptr);
    EXPECT_EQ(BufferLength, (size_t)0);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0200
 * @tc.name      : The size of buffer is over maxnum.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;

    size_t BufferSize = BUFFER_OVERMAX_SIZE;

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, &Buffer);

    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(tmpBufferPtr, nullptr);
    EXPECT_EQ(BufferLength, size_t(0));

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0300
 * @tc.name      : The parameters of size(BUFFER_OVERMAX_SIZE) and data are invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = BUFFER_OVERMAX_SIZE;

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, nullptr, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(tmpBufferPtr, nullptr);
    EXPECT_EQ(BufferLength, (size_t)0);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0400
 * @tc.name      : The parameters of size(-1) and data are invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = -1;

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(tmpBufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0500
 * @tc.name      : The parameter of size(0) is invalid
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = 0;

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(tmpBufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0600
 * @tc.name      : The parameters of napi_create_buffer_copy are valid,
 *                 The buffer is created by napi_create_buffer_copy function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is used to create a new buffer
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_buffer_info is called
 *                 6.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0600 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* Bufferptr = nullptr;
    size_t BufferSize = sizeof(Text) - 2;

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &Bufferptr, &Buffer);
    GTEST_LOG_(INFO) << "*Bufferptr"<<(char*)Bufferptr;
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(memcmp((char*)tmpBufferPtr, "hello worl", 10), 0);
    EXPECT_EQ(tmpBufferPtr, Bufferptr);
    EXPECT_EQ(BufferLength, sizeof(Text) - 2);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0700
 * @tc.name      : The parameter of pointer to the new Buffer's underlying data buffer is invalid,
 *                 The buffer is created by napi_create_buffer_copy function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is used to create a new buffer
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_buffer_info is called
 *                 6.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0700, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0700 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, "hello world"), 0);
    EXPECT_NE(tmpBufferPtr, nullptr);
    EXPECT_EQ(BufferLength, BufferSize);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0800
 * @tc.name      : The parameter of result is invalid,
 *                 The buffer can not be created by napi_create_buffer_copy function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is used to create a new buffer
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_get_buffer_info is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0800, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0800 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, nullptr);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(tmpBufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0800 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_0900
 * @tc.name      : The parameter of environment is invalid,
 *                 The buffer can not be created by napi_create_buffer_copy function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_buffer_copy is used to create a new buffer
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_get_buffer_info is called
 *                 6.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_0900, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0900 start";

    napi_env envinfo = (napi_env)engine_;
    napi_env envcopy = nullptr;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_buffer_copy(envcopy, BufferSize, Text, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(envinfo, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_invalid_arg);
    EXPECT_EQ(tmpBufferPtr, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_0900 end";
}

/*
 * @tc.number    : ACE_Napi_Create_Buffer_Copy_1000
 * @tc.name      : The parameter of environment is invalid,
 *                 The buffer can not be created by napi_create_buffer_copy function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.Set test variables.
 *                 3.The function of napi_create_buffer_copy is used to create
 *                   a new buffer when the BufferSize is larger then buffer.
 *                 4.Return value of function is napi_ok.
 *                 5.The function of napi_get_buffer_info is called.
 *                 6.Return value of function is napi_ok.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Buffer_Copy_1000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_1000 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    void* result_data = nullptr;
    size_t BufferSize = 1024;

    napi_status creatresult = napi_create_buffer_copy(env, BufferSize, Text, &result_data, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, "hello world"), 0);
    EXPECT_NE(tmpBufferPtr, nullptr);
    EXPECT_EQ(BufferLength, (size_t)1024);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Buffer_Copy_1000 end";
}

/*
 * @tc.number    : ACE_Napi_Create_External_Buffer_0100
 * @tc.name      : The parameters of napi_create_external_buffer are valid,
 *                 The buffer is created by napi_create_external_buffer function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_external_buffer is used to create a new buffer
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_buffer_info is called
 *                 6.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_External_Buffer_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0100 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    size_t BufferSize = 1;

    napi_status creatresult = napi_create_external_buffer(env, BufferSize, Text, BufferFinalizer, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, "hello world"), 0);
    EXPECT_NE(tmpBufferPtr, nullptr);
    EXPECT_EQ(BufferLength, BufferSize);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Create_External_Buffer_0200
 * @tc.name      : The parameter of buffer size is over maxnum,
 *                 The buffer can not be created by napi_create_external_buffer function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_external_buffer is used to create a new buffer
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_External_Buffer_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    size_t BufferSize = BUFFER_OVERMAX_SIZE;

    napi_status creatresult = napi_create_external_buffer(env, BufferSize, Text, BufferFinalizer, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Create_External_Buffer_0300
 * @tc.name      : The parameter of result is invalid,
 *                 The buffer can not be created by napi_create_external_buffer function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_external_buffer is used to create a new buffer
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_External_Buffer_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    size_t BufferSize = 0;

    napi_status creatresult = napi_create_external_buffer(env, BufferSize, Text, BufferFinalizer, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Create_External_Buffer_0400
 * @tc.name      : The parameters of finalize_cb and finalize_hint are invalid,
 *                 The buffer is created by napi_create_external_buffer function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_external_buffer is used to create a new buffer
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_get_buffer_info is called
 *                 6.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_External_Buffer_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_external_buffer(env, BufferSize, Text, nullptr, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    void* tmpBufferPtr = nullptr;
    size_t BufferLength = 0;
    napi_status getinforesult = napi_get_buffer_info(env, Buffer, &tmpBufferPtr, &BufferLength);
    EXPECT_EQ(getinforesult, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "*tmpBufferPtr"<<(char*)tmpBufferPtr;
    EXPECT_EQ(strcmp((char*)tmpBufferPtr, "hello world"), 0);
    EXPECT_NE(tmpBufferPtr, nullptr);
    EXPECT_EQ(BufferLength, size_t(12));

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Create_External_Buffer_0500
 * @tc.name      : The parameter of buffer size is invalid,
 *                 The buffer can not be created by napi_create_external_buffer function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_external_buffer is used to create a new buffer
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_External_Buffer_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value Buffer = nullptr;
    size_t BufferSize = -1;

    napi_status creatresult = napi_create_external_buffer(env, BufferSize, Text, BufferFinalizer, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Create_External_Buffer_0600
 * @tc.name      : The parameter of environment is invalid,
 *                 The buffer can not be created by napi_create_external_buffer function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_create_external_buffer is used to create a new buffer
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_External_Buffer_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0600 start";

    napi_env env = nullptr;
    napi_value Buffer = nullptr;
    size_t BufferSize = sizeof(Text);

    napi_status creatresult = napi_create_external_buffer(env, BufferSize, Text, BufferFinalizer, nullptr, &Buffer);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_External_Buffer_0600 end";
}

#if  (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
    // jerryscript 2.3 do nothing
#else
// jerryscript 2.4 or quickjs or V8
/*
 * @tc.number    : ACE_Napi_Object_Freeze_0100
 * @tc.name      : The parameters of napi_object_freeze are valid,
 *                 The object is freeze by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a object
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_set_element is called
 *                 6.The element can not be added
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0100 start";
    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_freeze(env, object);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_number_expected);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 0);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Freeze_0200
 * @tc.name      : The parameters of napi_object_freeze are valid,
 *                 The object is freeze by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a object
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_delete_element is called
 *                 6.The element can not be deleted
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    napi_value afterfreezeresult;
    napi_value namestr;
    int32_t numresultone;
    int32_t numresulttwo;
    int32_t afternumresulttwo;
    bool deleteresult = false;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    napi_status creatresult = napi_object_freeze(env, object);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    status = napi_create_string_utf8(env, "freezeresult", strlen("freezeresult"), &namestr);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_delete_property(env, object, namestr, &deleteresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &afterfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, afterfreezeresult, &afternumresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "afterfreezeresult is" << afternumresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Freeze_0300
 * @tc.name      : The parameters of napi_object_freeze are valid,
 *                 The object is freeze by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a object
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_set_element is called
 *                 6.The element can not be changed
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_freeze(env, object);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 10);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Freeze_0400
 * @tc.name      : The parameter of the object to freeze is invalid,
 *                 The object can not be freezed by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be added
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_freeze(env, nullptr);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Freeze_0500
 * @tc.name      : The parameter of environment is invalid,
 *                 The object can not be freezed by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be added
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_freeze(nullptr, object);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Freeze_0600
 * @tc.name      : The parameter of environment is invalid,
 *                 The object can not be freezed by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be deleted
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0600 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    napi_value afterfreezeresult;
    napi_value namestr;
    int32_t numresultone;
    int32_t numresulttwo;
    int32_t afternumresulttwo;
    bool deleteresult = false;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    napi_status creatresult = napi_object_freeze(nullptr, object);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_create_string_utf8(env, "freezeresult", strlen("freezeresult"), &namestr);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_delete_property(env, object, namestr, &deleteresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    EXPECT_EQ(deleteresult, true);
    status = napi_get_named_property(env, object, "freezeresult", &afterfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, afterfreezeresult, &afternumresulttwo);
    EXPECT_EQ(status, napi_status::napi_number_expected);
    GTEST_LOG_(INFO) << "afterfreezeresult is" << afternumresulttwo;
    EXPECT_EQ(afternumresulttwo, 0);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Freeze_0700
 * @tc.name      : The parameter of environment is invalid,
 *                 The object can not be freezed by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be changed
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0700 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_freeze(nullptr, object);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_set_named_property(env, object, "int32result", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Freeze_0800
 * @tc.name      : The parameter of environment is invalid,
 *                 The object can not be freezed by napi_object_freeze function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_freeze is used to freeze a number
 *                 4.Return value of function is napi_object_expected
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Freeze_0800, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0800 start";

    napi_env env = (napi_env)engine_;
    napi_value int32result;
    auto numone = static_cast<int32_t>(10);

    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    napi_status creatresult = napi_object_freeze(env, int32result);
    EXPECT_EQ(creatresult, napi_status::napi_object_expected);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Freeze_0800 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0100
 * @tc.name      : The parameters of napi_object_seal are valid,
 *                 The object is sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a object
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_set_element is called
 *                 6.The element can not be added
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0100 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_seal(env, object);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_number_expected);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 0);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0200
 * @tc.name      : The parameters of napi_object_seal are valid,
 *                 The object is sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a object
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_delete_element is called
 *                 6.The element can be deleted
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    napi_value afterfreezeresult;
    napi_value namestr;
    int32_t numresultone;
    int32_t numresulttwo;
    int32_t afternumresulttwo;
    bool deleteresult = false;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    napi_status creatresult = napi_object_seal(env, object);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    status = napi_create_string_utf8(env, "freezeresult", strlen("freezeresult"), &namestr);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_delete_property(env, object, namestr, &deleteresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &afterfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, afterfreezeresult, &afternumresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "afterfreezeresult is" << afternumresulttwo;
    EXPECT_EQ(afternumresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0300
 * @tc.name      : The parameters of napi_object_seal are valid,
 *                 The object is sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a object
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_set_element is called
 *                 6.The element can be changed
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_seal(env, object);
    EXPECT_EQ(creatresult, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0400
 * @tc.name      : The parameter of the object to seal is invalid,
 *                 The object can not be sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be added
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_seal(env, nullptr);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0500
 * @tc.name      : The parameter of environment is invalid,
 *                 The object can not be sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be added
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_seal(nullptr, object);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0600
 * @tc.name      : The parameter of environment is invalid,
 *                 The object can not be sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be deleted
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0600, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0600 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    napi_value afterfreezeresult;
    napi_value namestr;
    int32_t numresultone;
    int32_t numresulttwo;
    int32_t afternumresulttwo;
    bool deleteresult = false;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    status = napi_set_named_property(env, object, "freezeresult", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "freezeresult", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    napi_status creatresult = napi_object_seal(nullptr, object);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_create_string_utf8(env, "freezeresult", strlen("freezeresult"), &namestr);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_delete_property(env, object, namestr, &deleteresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    EXPECT_EQ(deleteresult, true);
    status = napi_get_named_property(env, object, "freezeresult", &afterfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, afterfreezeresult, &afternumresulttwo);
    EXPECT_EQ(status, napi_status::napi_number_expected);
    GTEST_LOG_(INFO) << "afterfreezeresult is" << afternumresulttwo;
    EXPECT_EQ(afternumresulttwo, 0);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0700
 * @tc.name      : The parameter of environment is invalid,
 *                 The object can not be sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a object
 *                 4.Return value of function is napi_invalid_arg
 *                 5.The function of napi_set_element is called
 *                 6.The element can be changed
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0700 start";

    napi_env env = (napi_env)engine_;
    napi_value object = nullptr;
    napi_value int32result;
    napi_value newuint32result;
    napi_value freezeresult;
    napi_value newfreezeresult;
    int32_t numresultone;
    int32_t numresulttwo;
    auto numone = static_cast<int32_t>(10);
    auto numtwo = static_cast<int32_t>(20);

    napi_create_object(env, &object);
    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_create_int32(env, numtwo, &freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_set_named_property(env, object, "int32result", int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newuint32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newuint32result, &numresultone);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newuint32result is" << numresultone;
    napi_status creatresult = napi_object_seal(nullptr, object);
    EXPECT_EQ(creatresult, napi_status::napi_invalid_arg);
    status = napi_set_named_property(env, object, "int32result", freezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_named_property(env, object, "int32result", &newfreezeresult);
    EXPECT_EQ(status, napi_status::napi_ok);
    status = napi_get_value_int32(env, newfreezeresult, &numresulttwo);
    EXPECT_EQ(status, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "newfreezeresult is" << numresulttwo;
    EXPECT_EQ(numresulttwo, 20);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Object_Seal_0800
 * @tc.name      : The parameters of napi_object_seal are valid,
 *                 The object is sealed by napi_object_seal function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_object_seal is used to seal a number
 *                 4.Return value of function is napi_object_expected
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Object_Seal_0800, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0800 start";

    napi_env env = (napi_env)engine_;
    napi_value int32result;
    auto numone = static_cast<int32_t>(10);

    napi_status status = napi_create_int32(env, numone, &int32result);
    EXPECT_EQ(status, napi_status::napi_ok);
    napi_status creatresult = napi_object_seal(env, int32result);
    EXPECT_EQ(creatresult, napi_status::napi_object_expected);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Object_Seal_0800 end";
}
#endif // (defined(FOR_JERRYSCRIPT_TEST)) &&  (JERRY_API_MINOR_VERSION <= 3)
/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0100
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function is called in blocking mode.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.calls the napi_call_threadsafe_function function in the test thread and child threads.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0100 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, &g_finalData,
        FinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    ThreadData_t threadData = { .tsfn = tsFunc, .isMode = napi_tsfn_blocking };
    if (uv_thread_create(&newChildTid, NewChildThreadMuti, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join Failed!";
    }
    int testCount = 20;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, (void*)&g_threadDataContent, napi_tsfn_blocking);
        EXPECT_EQ(status, napi_ok);
    }

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);

    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0100 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0200
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function is called in nonblocking mode.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.calls the napi_call_threadsafe_function function in the test thread and child threads.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0200 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 0, 1, &g_finalData,
        NonBlockFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    ThreadData_t threadData = { .tsfn = tsFunc, .isMode = napi_tsfn_nonblocking };
    if (uv_thread_create(&newChildTid, NewChildThreadMuti, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join Failed!";
    }
    int testCount = 20;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, &g_threadDataContent, napi_tsfn_nonblocking);
        EXPECT_EQ(status, napi_ok);
    }

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();

    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0200 called end";
}
/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0300
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function in blocking mode.
 *                 4.calls the napi_call_threadsafe_function function in non-blocking mode in the created child thread.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0300 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 50, 1, &g_finalData,
        FinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    ThreadData_t threadData = { .tsfn = tsFunc, .isMode = napi_tsfn_blocking };
    if (uv_thread_create(&newChildTid, NewChildThreadMuti, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join Failed!";
    }

    int testCount = 20;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, &g_threadDataContent, napi_tsfn_nonblocking);
        EXPECT_EQ(status, napi_ok);
    }

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0300 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0400
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function in non-blocking mode.
 *                 4.calls the napi_call_threadsafe_function function in blocking mode in the created child thread.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0400 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 50, 1, &g_finalData,
        FinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    ThreadData_t threadData = { .tsfn = tsFunc, .isMode = napi_tsfn_nonblocking };
    if (uv_thread_create(&newChildTid, NewChildThreadMuti, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join Failed!";
    }

    int testCount = 20;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, &g_threadDataContent, napi_tsfn_blocking);
        EXPECT_EQ(status, napi_ok);
    }

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0400 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0500
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function in blocking and non-blocking
 *                  modes.
 *                 4.calls the napi_call_threadsafe_function function in blocking and non-blocking modes in the created
 *                  child thread.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0500 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 50, 1, &g_finalData,
        AllFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    ThreadData_t threadData = { .tsfn = tsFunc };
    if (uv_thread_create(&newChildTid, NonBlockAndBlockNewChildThreadMuti, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join Failed!";
    }

    int testCount = 10;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, &g_threadDataContent3, napi_tsfn_blocking);
        EXPECT_EQ(status, napi_ok);
    }

    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, &g_threadDataContent3, napi_tsfn_nonblocking);
        EXPECT_EQ(status, napi_ok);
    }
    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0500 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0600
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function in blocking and non-blocking
 *                   modes.
 *                 4.calls the napi_call_threadsafe_function function in blocking and non-blocking modes in the created
 *                  child thread.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0600 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 50, 1, &g_finalData,
        OtherFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    uv_thread_t newChildTid2;
    ThreadData_t threadData = { .tsfn = tsFunc, .isMode = napi_tsfn_nonblocking };
    if (uv_thread_create(&newChildTid, NonBlockAndBlockNewChildThreadMuti, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }

    if (uv_thread_create(&newChildTid2, NonBlockAndBlockNewChildThreadMuti, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid2) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid2) Failed!";
    }
    int testCount = 3;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, &g_threadDataContent3, napi_tsfn_blocking);
        EXPECT_EQ(status, napi_ok);
    }
    testCount = 3;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, &g_threadDataContent3, napi_tsfn_nonblocking);
        EXPECT_EQ(status, napi_ok);
    }
    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);

    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0600 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0700
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function [nonblock call 10 times,
 *                   block call 10 times, nonblock call 10 times].
 *                 4.In the three child threads created, calls the napi_call_threadsafe_function
 *                   function[nonblock call 10 times, block call 10 times, onblock call 10 times].
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0700, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0700 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 200, 1, &g_finalData,
        MutiModeFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid, newChildTid2, newChildTid3;
    OneModeCallData_t threadData = { .tsfn = tsFunc, .mode = napi_tsfn_nonblocking, .callCount = 10 };
    if (uv_thread_create(&newChildTid, MutiModeCallOne, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_create(&newChildTid) Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }
    if (uv_thread_create(&newChildTid2, MutiModeCallOne, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_create(&newChildTid2) Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid2) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid2) Failed!";
    }
    if (uv_thread_create(&newChildTid3, MutiModeCallOne, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_create(&newChildTid3) Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid3) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid3) Failed!";
    }

    threadData.mode = napi_tsfn_nonblocking;
    threadData.callCount = 10;
    OneModeCall(&threadData);

    threadData.mode = napi_tsfn_blocking;
    OneModeCall(&threadData);

    threadData.mode = napi_tsfn_nonblocking;
    OneModeCall(&threadData);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0700 called end";
}
/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0800
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function [block call 10 times,
 *                   nonblock call 10 times, block call 10 times].
 *                 4.In the three child threads created, calls the napi_call_threadsafe_function
 *                   function[block call 10 times, nonblock call 10 times, block call 10 times].
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0800, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0800 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 200, 1, &g_finalData,
        MutiModeFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid, newChildTid2, newChildTid3;
    OneModeCallData_t threadData = { .tsfn = tsFunc, .mode = napi_tsfn_nonblocking, .callCount = 10 };
    if (uv_thread_create(&newChildTid, MutiModeCallTwo, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }
    if (uv_thread_create(&newChildTid2, MutiModeCallTwo, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid2) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid2) Failed!";
    }
    if (uv_thread_create(&newChildTid3, MutiModeCallTwo, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid3) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid3) Failed!";
    }

    threadData.mode = napi_tsfn_blocking;
    threadData.callCount = 10;
    OneModeCall(&threadData);

    threadData.mode = napi_tsfn_nonblocking;
    OneModeCall(&threadData);

    threadData.mode = napi_tsfn_blocking;
    OneModeCall(&threadData);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0800 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_0900
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function [block call 10 times,
 *                   nonblock call 10 times, block call 10 times].
 *                 4.In the first child thread created, calls the napi_call_threadsafe_function 
 *                   function[nonblock call 10 times, block call 10 times, nonblock call 10 times].
 *                 5.In the second child thread created, calls the napi_call_threadsafe_function
 *                   function[block call 10 times, nonblock call 10 times, block call 10 times].
 *                 6.In the third child thread created, calls the napi_call_threadsafe_function
 *                   function[nonblock call 10 times, block call 10 times, nonblock call 10 times].
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_0900, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0900 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 200, 1, &g_finalData,
        MutiModeFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid, newChildTid2, newChildTid3;
    OneModeCallData_t threadData = { .tsfn = tsFunc, .mode = napi_tsfn_nonblocking, .callCount = 10 };
    if (uv_thread_create(&newChildTid, MutiModeCallOne, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }
    if (uv_thread_create(&newChildTid2, MutiModeCallTwo, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid2) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid2) Failed!";
    }
    if (uv_thread_create(&newChildTid3, MutiModeCallOne, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid3) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid3) Failed!";
    }

    threadData.mode = napi_tsfn_blocking;
    threadData.callCount = 10;
    OneModeCall(&threadData);

    threadData.mode = napi_tsfn_nonblocking;
    OneModeCall(&threadData);

    threadData.mode = napi_tsfn_blocking;
    OneModeCall(&threadData);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);

    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_0900 called end";
}
/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_1000
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function [nonblock call 30 times].
 *                 4.In the one child thread created, calls the napi_call_threadsafe_function 
 *                   function[nonblock call 30 times].
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_1000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1000 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 200, 1, &g_finalData,
        MutiModeFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid, newChildTid2, newChildTid3;
    OneModeCallData_t threadData = { .tsfn = tsFunc, .mode = napi_tsfn_nonblocking, .callCount = 10 };
    if (uv_thread_create(&newChildTid, MutiModeCallThree, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }
    if (uv_thread_create(&newChildTid2, MutiModeCallThree, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid2) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid2) Failed!";
    }
    if (uv_thread_create(&newChildTid3, MutiModeCallThree, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid3) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid3) Failed!";
    }

    threadData.mode = napi_tsfn_nonblocking;
    threadData.callCount = 30;
    OneModeCall(&threadData);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1000 called end";
}
/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_1100
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                  calls napi_call_threadsafe_function.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.napi_create_threadsafe_function creates a queue.
 *                 3.In the test thread, calls the napi_call_threadsafe_function function [block call 30 times].
 *                 4.In the three child threads created, calls the napi_call_threadsafe_function
 *                   function[block call 30 times].
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_1100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1100 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_callJSCallBackCount = 0;
    g_callFinalizeEnd = false;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 200, 1, &g_finalData,
        MutiModeFinalizeThreadCallBack, &g_jsData, CallJSCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid, newChildTid2, newChildTid3;
    OneModeCallData_t threadData = { .tsfn = tsFunc, .mode = napi_tsfn_nonblocking, .callCount = 10 };
    if (uv_thread_create(&newChildTid, MutiModeCallFour, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid)!= 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }
    if (uv_thread_create(&newChildTid2, MutiModeCallFour, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid2) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid2) Failed!";
    }
    if (uv_thread_create(&newChildTid3, MutiModeCallFour, (void*)&threadData) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid3) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid3) Failed!";
    }

    threadData.mode = napi_tsfn_blocking;
    threadData.callCount = 30;
    OneModeCall(&threadData);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1100 called end";
}
/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_1200
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full .
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue with size 10.
 *                3.In the child thread, calls the napi_call_threadsafe_function function in non-blocking modes for 30
 *                  times. 
 *                4.Normally for each CallJSSlowCallBack takes at least 1 second,
 *                  the created queue would be full quickly.
 *                5.Check if napi_call_threadsafe_function failure happens.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_1200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1200 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    g_callJSCallBackCount = 0;
    g_callCount = 0;
    g_bFailFlag = false;
    g_bIsFinish = false;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 10, 1, &g_finalData,
        FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    if (uv_thread_create(&newChildTid, NewChildThreadMutiCallNoneBlocking, tsFunc) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }
    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);

    WaitForFinish();
    EXPECT_EQ(g_bFailFlag, true);
    GTEST_LOG_(INFO) << "CallBack Count= " << g_callJSCallBackCount;
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1200 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_1300
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue with size 10.
 *                3.In the child thread, calls the napi_call_threadsafe_function 
 *                  function in blocking mode for 20 times.
 *                4.Normally for each CallJSSlowCallBack takes at least 1 second, the created queue would be full
 *                  quickly. 5.Check if napi_call_threadsafe_function failure happens.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_1300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1300 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    g_callJSCallBackCount = 0;
    g_callCount = 0;
    g_bIsFinish = false;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 10, 1, &g_finalData,
        FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    if (uv_thread_create(&newChildTid, NewChildThreadMutiCallBlocking, tsFunc) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();
    EXPECT_EQ(g_callCount, g_callJSCallBackCount);
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1300 called end";
}

/*
 * @tc.number    : ACE_Napi_Call_Threadsafe_Function_1400
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function blocking mode with the queue full .
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue with size 10.
 *                3.In the child thread, calls the napi_call_threadsafe_function 
 *                  function in blocking mode for 20 times.
 *                4.In the test thread, calls the napi_call_threadsafe_function function in non-blocking mode for 20
 *                  times.
 *                5.Normally for each CallJSSlowCallBack takes at least 1 second, the created queue would be full
 *                  quickly.
 *                6.Check if napi_call_threadsafe_function failure happens and the callback times
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Call_Threadsafe_Function_1400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1400 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    g_callJSCallBackCount = 0;
    g_callCount = 0;
    g_bFailFlag = false;
    g_bIsFinish = false;
    int iFailTimes = 0;

    auto status = napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    EXPECT_EQ(status, napi_ok);

    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName, 10, 1, &g_finalData,
        FinalCallBack, &g_jsData, CallJSSlowCallBack4Block, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    if (uv_thread_create(&newChildTid, NewChildThreadMutiCallBlocking, tsFunc) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    int testCount = 20;
    for (int i = 0; i < testCount; i++) {
        status = napi_call_threadsafe_function(tsFunc, nullptr, napi_tsfn_nonblocking);
        if (napi_ok != status) {
            iFailTimes++;
        }
    }
    if (0 < iFailTimes) {
        g_bFailFlag = true;
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid) Failed!";
    }

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();
    EXPECT_EQ(g_bFailFlag, true);
    EXPECT_TRUE(g_callJSCallBackCount >= iBlockCallTimes);
    GTEST_LOG_(INFO) << "ACE_Napi_Call_Threadsafe_Function_1400 called end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0100
 * @tc.name    : If all parameters are normal, call napi_create_threadsafe_function to create a safe thread
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function
 *               3.Call uv_thread_create to create the thread
 *               4.Jscallback is triggered by napi_call_threadsafe_function
 *               5.Call napi_release_threadsafe_function to free the thread
 *               6.Trigger Threadfinalcb
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0100 start";

    napi_env env = (napi_env)engine_;
    g_callFinalizeEnd = false;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    EXPECT_NE(result, nullptr);
    if (uv_thread_create(&g_uvThread, TsFuncDataSourceThread, result) != 0) {
        GTEST_LOG_(INFO) << "Failed to create uv thread!";
    }
    GetFinalizeStatus();
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0100 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0200
 * @tc.name    : Call napi_create_threadsafe_function when the number of initial threads is the maximum
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initial threads is the
 *                  maximum
 *               3.Call uv_thread_create to create the thread
 *               4.Jscallback is triggered by napi_call_threadsafe_function
 *               5.Call napi_release_threadsafe_function to free the thread
 *               6.Trigger Threadfinalcb
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0200 start";

    napi_env env = (napi_env)engine_;
    g_callFinalizeEnd = false;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(env, nullptr, nullptr, resource_name,
        0, MAX_COUNT, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    EXPECT_NE(result, nullptr);
    if (uv_thread_create(&g_uvThread, TsFuncDataSourceThread0200, result) != 0) {
        GTEST_LOG_(INFO) << "Failed to create uv thread!";
    }
    GetFinalizeStatus();

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0200 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0300
 * @tc.name    : Call napi_create_threadsafe_function when the number of initialization threads exceeds the maximum
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initialization threads
 *                  exceeds the maximum
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0300 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(env, nullptr, nullptr, resource_name, 0, OVER_MAX_COUNT,
        &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0300 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0400
 * @tc.name    : Call napi_create_threadsafe_function when JSCallback is nullptr
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when JSCallback is nullptr
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0400 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, &finalData, Threadfinalcb, &jsData, nullptr, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0400 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0500
 * @tc.name    : Call napi_create_threadsafe_function when finalcallback is nullptr
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when finalcallback is nullptr
 *               3.Call uv_thread_create to create the thread
 *               4.Jscallback is triggered by napi_call_threadsafe_function
 *               5.Call napi_release_threadsafe_function to free the thread
 *               6.Trigger Threadfinalcb
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0500 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, &finalData, nullptr, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    EXPECT_NE(result, nullptr);
    if (uv_thread_create(&g_uvThread, TsFuncDataSourceThread, result) != 0) {
        GTEST_LOG_(INFO) << "Failed to create uv thread!";
    }
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0500 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0600
 * @tc.name    : Call napi_create_threadsafe_function when JSCallback and finalcallback is nullptr
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when JSCallback and finalcallback is nullptr
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0600 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, &finalData, nullptr, &jsData, nullptr, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0600 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0700
 * @tc.name    : Call napi_create_threadsafe_function when the number of initialization threads is 0
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initialization threads is 0
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0700 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 0, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0700 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0800
 * @tc.name    : Call napi_create_threadsafe_function when the number of initialization threads is -1
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initialization threads is -1
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0800, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0800 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, -1, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0800 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_0900
 * @tc.name    : Call napi_create_threadsafe_function when the JS identifier name is empty
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the JS identifier name is empty
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_0900, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0900 start";

    napi_env env = (napi_env)engine_;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, nullptr, 0, 1, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_0900 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_1000
 * @tc.name    : Call napi_create_threadsafe_function when the output parameter result is empty
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the output parameter result is empty
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_1000, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_1000 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, nullptr);
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_1000 end";
}

/*
 * @tc.number  : ACE_Napi_Create_Threadsafe_Function_1100
 * @tc.name    : Call napi_create_threadsafe_function when the env environment variable is empty
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the env environment variable is empty
 *               3.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Create_Threadsafe_Function_1100, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_1100 start";

    napi_env env = nullptr;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Create_Threadsafe_Function_1100 end";
}

/*
 * @tc.number  : ACE_Napi_Get_Threadsafe_Function_Context_0100
 * @tc.name    : Call napi_get_threadsafe_function_context to get the context value passed by creat
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function and the value passed in context is number
 *               3.Call napi_get_threadsafe_function_context to get context
 *               4.Call napi_release_threadsafe_function to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Threadsafe_Function_Context_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0100 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    jsData.id = CALL_JSCB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status getcontextresult = napi_get_threadsafe_function_context(result, &contextresult);
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    EXPECT_EQ(getcontextresult, napi_status::napi_ok);
    CallJsCbData* cbdata = nullptr;
    cbdata = (CallJsCbData*)contextresult;
    EXPECT_EQ(cbdata->id, CALL_JSCB_DATA);
    napi_status releaseresult = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_ok);
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0100 end";
}

/*
 * @tc.number  : ACE_Napi_Get_Threadsafe_Function_Context_0200
 * @tc.name    : Call napi_get_threadsafe_function_context to get the context value passed by creat
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function
 *               3.Call napi_get_threadsafe_function_context to get context when the contextresult is
 *                  nullptr
 *               4.return napi_invalid_arg 5.Call napi_release_threadsafe_function to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Threadsafe_Function_Context_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0200 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status getcontextresult = napi_get_threadsafe_function_context(result, nullptr);
    EXPECT_EQ(getcontextresult, napi_status::napi_invalid_arg);
    napi_status releaseresult = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_ok);
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0200 end";
}

/*
 * @tc.number  : ACE_Napi_Get_Threadsafe_Function_Context_0300
 * @tc.name    : Call napi_get_threadsafe_function_context to get the context value passed by creat
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function
 *               3.Call napi_get_threadsafe_function_context to get context when the result is nullptr
 *               4.return napi_invalid_arg
 *               5.Call napi_release_threadsafe_function to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Threadsafe_Function_Context_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0300 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    jsData.id = CALL_JSCB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status getcontextresult = napi_get_threadsafe_function_context(nullptr, &contextresult);
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    EXPECT_EQ(getcontextresult, napi_status::napi_invalid_arg);
    napi_status releaseresult = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_ok);
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0300 end";
}

/*
 * @tc.number  : ACE_Napi_Get_Threadsafe_Function_Context_0400
 * @tc.name    : Call napi_get_threadsafe_function_context to get the context value passed by creat
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function and the value passed in context is a string
 *               3.Call napi_get_threadsafe_function_context to get context
 *               4.Call napi_release_threadsafe_function to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Threadsafe_Function_Context_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0400 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    jsData_str.id = CALL_JSCB_DATA;
    if (strcpy_s(jsData_str.strdata, strlen("contextdata") + 1, "contextdata") != 0) {
        return;
    }
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, &jsData_str, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status getcontextresult = napi_get_threadsafe_function_context(result, &contextresult);
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    EXPECT_EQ(getcontextresult, napi_status::napi_ok);
    CallJsCbData_str* cbdata = nullptr;
    cbdata = (CallJsCbData_str*)contextresult;
    GTEST_LOG_(INFO) << "cbdata is " << cbdata;
    GTEST_LOG_(INFO) << "cbdata->id is " << cbdata->id;
    EXPECT_EQ(cbdata->id, CALL_JSCB_DATA);
    GTEST_LOG_(INFO) << "cbdata->strdata is " << cbdata->strdata;
    EXPECT_EQ(strcmp(cbdata->strdata, "contextdata"), 0);
    napi_status releaseresult = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_ok);
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0400 end";
}

/*
 * @tc.number  : ACE_Napi_Get_Threadsafe_Function_Context_0500
 * @tc.name    : Call napi_get_threadsafe_function_context to get the context value passed by creat
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function and the value passed in context is a special
 *                  string
 *               3.Call napi_get_threadsafe_function_context to get context 4.Call napi_release_threadsafe_function to
 *                  free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Threadsafe_Function_Context_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0500 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    jsData_str.id = CALL_JSCB_DATA;
    if (strcpy_s(jsData_str.strdata, strlen("~!@#$%^&*( ") + 1, "~!@#$%^&*( ") != 0) {
        return;
    }
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, &jsData_str, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status getcontextresult = napi_get_threadsafe_function_context(result, &contextresult);
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    EXPECT_EQ(getcontextresult, napi_status::napi_ok);
    CallJsCbData_str* cbdata = nullptr;
    cbdata = (CallJsCbData_str*)contextresult;
    GTEST_LOG_(INFO) << "cbdata is " << cbdata;
    GTEST_LOG_(INFO) << "cbdata->id is " << cbdata->id;
    EXPECT_EQ(cbdata->id, CALL_JSCB_DATA);
    GTEST_LOG_(INFO) << "cbdata->strdata is " << cbdata->strdata;
    EXPECT_EQ(strcmp(cbdata->strdata, "~!@#$%^&*( "), 0);
    napi_status releaseresult = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_ok);
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0500 end";
}

/*
 * @tc.number  : ACE_Napi_Get_Threadsafe_Function_Context_0600
 * @tc.name    : Call napi_get_threadsafe_function_context to get the context value passed by creat
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function and the value of the incoming context is a
 *                  Chinese character
 *               3.Call napi_get_threadsafe_function_context to get context 4.Call napi_release_threadsafe_function
 *                  to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Get_Threadsafe_Function_Context_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0600 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    jsData_str.id = CALL_JSCB_DATA;
    if (strcpy_s(jsData_str.strdata, strlen("大家好!!") + 1, "大家好!!") != 0) {
        return;
    }
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, &jsData_str, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status getcontextresult = napi_get_threadsafe_function_context(result, &contextresult);
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    EXPECT_EQ(getcontextresult, napi_status::napi_ok);
    CallJsCbData_str* cbdata = nullptr;
    cbdata = (CallJsCbData_str*)contextresult;
    GTEST_LOG_(INFO) << "cbdata is " << cbdata;
    GTEST_LOG_(INFO) << "cbdata->id is " << cbdata->id;
    EXPECT_EQ(cbdata->id, CALL_JSCB_DATA);
    GTEST_LOG_(INFO) << "cbdata->strdata is " << cbdata->strdata;
    EXPECT_EQ(strcmp(cbdata->strdata, "大家好!!"), 0);
    napi_status releaseresult = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";
    EXPECT_EQ(releaseresult, napi_status::napi_ok);
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Get_Threadsafe_Function_Context_0600 end";
}

/*
 * @tc.number  : ACE_Napi_Acquire_Threadsafe_Function_0100
 * @tc.name    : Call napi_acquire_threadsafe_function after passing the parameters correctly
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function
 *               3.Call napi_acquire_threadsafe_function
 *               4.Call twice  napi_release_threadsafe_function to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Acquire_Threadsafe_Function_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Acquire_Threadsafe_Function_0100 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, nullptr, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status acquireresult = napi_acquire_threadsafe_function(result);
    GTEST_LOG_(INFO) << "acquireresult is " << acquireresult;
    EXPECT_EQ(acquireresult, napi_status::napi_ok);
    napi_status releaseresultone = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresultone is " << releaseresultone;
    EXPECT_EQ(releaseresultone, napi_status::napi_ok);
    napi_status releaseresulttwo = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresulttwo is " << releaseresulttwo;
    EXPECT_EQ(releaseresulttwo, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Acquire_Threadsafe_Function_0100 end";
}

/*
 * @tc.number  : ACE_Napi_Acquire_Threadsafe_Function_0200
 * @tc.name    : Call napi_acquire_threadsafe_function after passing parameter error
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function
 *               3.Call napi_acquire_threadsafe_function after the result value is nullptr
 *               4.Call napi_release_threadsafe_function to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Acquire_Threadsafe_Function_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Acquire_Threadsafe_Function_0200 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, nullptr, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status acquireresult = napi_acquire_threadsafe_function(nullptr);
    GTEST_LOG_(INFO) << "acquireresult is " << acquireresult;
    EXPECT_EQ(acquireresult, napi_status::napi_invalid_arg);
    napi_status releaseresultone = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresult is " << releaseresultone;
    EXPECT_EQ(releaseresultone, napi_status::napi_ok);
    napi_status releaseresulttwo = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresult is " << releaseresulttwo;
    EXPECT_EQ(releaseresulttwo, napi_status::napi_generic_failure);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Acquire_Threadsafe_Function_0200 end";
}

/*
 * @tc.number  : ACE_Napi_Acquire_Threadsafe_Function_0300
 * @tc.name    : Call napi_acquire_threadsafe_function after passing the parameters correctly
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function
 *               3.Call twice napi_acquire_threadsafe_function
 *               4.Call thrice napi_release_threadsafe_function to free the thread
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Acquire_Threadsafe_Function_0300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Acquire_Threadsafe_Function_0300 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    void* contextresult = nullptr;
    GTEST_LOG_(INFO) << "contextresult is " << contextresult;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, nullptr, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    napi_status acquireresultone = napi_acquire_threadsafe_function(result);
    GTEST_LOG_(INFO) << "acquireresult is " << acquireresultone;
    EXPECT_EQ(acquireresultone, napi_status::napi_ok);
    napi_status acquireresulttwo = napi_acquire_threadsafe_function(result);
    GTEST_LOG_(INFO) << "acquireresult is " << acquireresulttwo;
    EXPECT_EQ(acquireresulttwo, napi_status::napi_ok);
    napi_status releaseresultone = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresultone is " << releaseresultone;
    EXPECT_EQ(releaseresultone, napi_status::napi_ok);
    napi_status releaseresulttwo = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresulttwo is " << releaseresulttwo;
    EXPECT_EQ(releaseresulttwo, napi_status::napi_ok);
    napi_status releaseresultthree = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresultthree is " << releaseresultthree;
    EXPECT_EQ(releaseresultthree, napi_status::napi_ok);
    GTEST_LOG_(INFO) << "napi_release_threadsafe_function finish!";

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Acquire_Threadsafe_Function_0300 end";
}

/*
 * @tc.number  : ACE_Napi_Release_Threadsafe_Function_0100
 * @tc.name    : Call napi_release_threadsafe_function after passing the parameters correctly
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initial threads is one
 *               3.Call uv_thread_create to create thread
 *               4.Call napi_release_threadsafe_function
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Release_Threadsafe_Function_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0100 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, nullptr, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    EXPECT_NE(result, nullptr);
    if (uv_thread_create(&g_uvThread, TsFuncreleaseThread, result) != 0) {
        GTEST_LOG_(INFO) << "Failed to create uv thread!";
    }
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0100 end";
}

/*
 * @tc.number  : ACE_Napi_Release_Threadsafe_Function_0200
 * @tc.name    : Call napi_release_threadsafe_function after passing parameter error
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initial threads is one
 *               3.Call uv_thread_create to create thread
 *               4.Call napi_release_threadsafe_function when the incoming result is nullptr
 *               5.return napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Release_Threadsafe_Function_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0200 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 1, nullptr, nullptr, nullptr, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    EXPECT_NE(result, nullptr);
    if (uv_thread_create(&g_uvThread, TsFuncErrReleaseThread, result) != 0) {
        GTEST_LOG_(INFO) << "Failed to create uv thread!";
    }
    sleep(1);
    napi_status releaselastresult = napi_release_threadsafe_function(result, napi_tsfn_release);
    GTEST_LOG_(INFO) << "releaseresultthree is " << releaselastresult;
    EXPECT_EQ(releaselastresult, napi_status::napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0200 end";
}

/*
 * @tc.number  : ACE_Napi_Release_Threadsafe_Function_0300
 * @tc.name    : Call napi_release_threadsafe_function after passing the parameters correctly
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initial threads is five
 *               3.Call uv_thread_create to create thread
 *               4.Call Five times napi_release_threadsafe_function
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Release_Threadsafe_Function_0300, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0300 start";

    napi_env env = (napi_env)engine_;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 5, nullptr, nullptr, nullptr, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    EXPECT_NE(result, nullptr);
    if (uv_thread_create(&g_uvThread, TsFuncreleasefiveThread, result) != 0) {
        GTEST_LOG_(INFO) << "Failed to create uv thread!";
    }
    sleep(1);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0300 end";
}

/*
 * @tc.number  : ACE_Napi_Release_Threadsafe_Function_0400
 * @tc.name    : Call napi_release_threadsafe_function after passing the parameters correctly
 * @tc.desc    : 1.Declare each parameter correctly
 *               2.Call napi_create_threadsafe_function when the number of initial threads is two
 *               3.Call uv_thread_create to create thread
 *               4.Call napi_release_threadsafe_function when the incoming mode is napi_tsfn_abort
 *               5.Call uv_thread_create to create thread return napi_closing
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Release_Threadsafe_Function_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0400 start";

    napi_env env = (napi_env)engine_;
    g_callFinalizeEnd = false;
    size_t strsize = 13;
    napi_value resource_name;
    napi_threadsafe_function result = nullptr;
    jsData.id = CALL_JSCB_DATA;
    finalData.id = FINAL_CB_DATA;
    napi_create_string_utf8(env, "JSstringTest", strsize, &resource_name);
    napi_status threadresult = napi_create_threadsafe_function(
        env, nullptr, nullptr, resource_name, 0, 2, &finalData, Threadfinalcb, &jsData, ThreadSafeCallJs, &result);
    GTEST_LOG_(INFO) << "result is " << result;
    GTEST_LOG_(INFO) << "threadresult is " << threadresult;
    EXPECT_EQ(threadresult, napi_status::napi_ok);
    EXPECT_NE(result, nullptr);
    if (uv_thread_create(&g_uvThread, TsFuncabortThread, result) != 0) {
        GTEST_LOG_(INFO) << "Failed to create uv thread!";
    }
    GetFinalizeStatus();

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Release_Threadsafe_Function_0400 end";
}

static void Cleanup(void* arg)
{
    GTEST_LOG_(INFO) << "Cleanup(void* arg) start";

    HOOK_TAG += INT_ONE;
    if (arg != nullptr) {
        GTEST_LOG_(INFO) << "cleanup(" << *(int*)(arg) << ")";
    }
    GTEST_LOG_(INFO) << "Cleanup(void* arg) end";
}

static void CleanupCopy(void* arg)
{
    GTEST_LOG_(INFO) << "CleanupCopy(void* arg) start";

    HOOK_TAGCP += INT_ONE;
    if (arg != nullptr) {
        GTEST_LOG_(INFO) << "CleanupCopy(" << *(int*)(arg) << ")";
    }
    GTEST_LOG_(INFO) << "CleanupCopy(void* arg) end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0100
 * @tc.name      : Tests whether napi_add_env_cleanup_hook is called properly when the environment exits
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call RunCleanup to trigger napi_add_env_cleanup_hook
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0100 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0200
 * @tc.name      : Test napi_add_env_cleanup_hook if the arg parameter is null
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call RunCleanup to trigger napi_add_env_cleanup_hook
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0200 start";
    napi_env env = (napi_env)engine_;
    napi_status ret = napi_invalid_arg;
    ret = napi_add_env_cleanup_hook(env, Cleanup, nullptr);
    engine_->RunCleanup();
    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0300
 * @tc.name      : Invalid registered function for napi_add_env_cleanup_hook test
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0300 start";
    napi_env env = (napi_env)engine_;
    napi_status ret = napi_ok;
    ret = napi_add_env_cleanup_hook(env, nullptr, &HOOK_ARG_ONE);

    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0400
 * @tc.name      : Test napi_add_env_cleanup_hook passing an invalid env
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0400 start";
    napi_status ret = napi_ok;
    ret = napi_add_env_cleanup_hook(nullptr, Cleanup, &HOOK_ARG_ONE);
    engine_->RunCleanup();
    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0500
 * @tc.name      : Release after testing napi_add_env_cleanup_hook call, call again
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_remove_env_cleanup_hook is called
 *                 6.Release napi_add_env_cleanup_hook
 *                 7.The function of napi_add_env_cleanup_hook is called
 *                 8.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0500 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_remove_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0600
 * @tc.name      : Test the normal operation of napi_add_env_cleanup_hook
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Then call the function of napi_add_env_cleanup_hook again
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0600 start";
    HOOK_TAG = INT_ZERO;
    napi_env env = (napi_env)engine_;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_TWO);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0700
 * @tc.name      : Test whether an exception is returned when the parameters of
 *                 napi_add_env_cleanup_hook are the same
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0700 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Env_Cleanup_Hook_0800
 * @tc.name      : Test the normal operation of napi_add_env_cleanup_hook
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Env_Cleanup_Hook_0800, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0800 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_THREE));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_THREE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Env_Cleanup_Hook_0800 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Env_Cleanup_Hook_0100
 * @tc.name      : Test napi_remove_env_cleanup_hook Delete napi_add_env_cleanup_hook
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call RunCleanup to trigger napi_add_env_cleanup_hook
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Env_Cleanup_Hook_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0100 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    ExpectCheckCall(napi_remove_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Env_Cleanup_Hook_0200
 * @tc.name      : Test napi_remove_env_cleanup_hook  the arg parameter is null
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_remove_env_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Env_Cleanup_Hook_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0200 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    napi_status ret = napi_invalid_arg;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    ret = napi_remove_env_cleanup_hook(env, Cleanup, nullptr);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_TWO);
    EXPECT_EQ(ret, napi_ok);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Env_Cleanup_Hook_0300
 * @tc.name      : Test Invalid registered function for napi_remove_env_cleanup_hook
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_remove_env_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Env_Cleanup_Hook_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0300 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    napi_status ret = napi_ok;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    ret = napi_remove_env_cleanup_hook(env, nullptr, &HOOK_ARG_TWO);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_TWO);
    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Env_Cleanup_Hook_0400
 * @tc.name      : Test napi_remove_env_cleanup_hook passing an invalid env
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_remove_env_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Env_Cleanup_Hook_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0400 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    napi_status ret = napi_ok;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    ret = napi_remove_env_cleanup_hook(nullptr, Cleanup, &HOOK_ARG_TWO);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_TWO);
    EXPECT_EQ(ret, napi_invalid_arg);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Env_Cleanup_Hook_0500
 * @tc.name      : Test napi_remove_env_cleanup_hook Delete
 *                 napi_add_env_cleanup_hook abnormal parameter transmission
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call RunCleanup to trigger napi_add_env_cleanup_hook
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Env_Cleanup_Hook_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0500 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    HOOK_TAGCP = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, CleanupCopy, &HOOK_ARG_TWO));
    ExpectCheckCall(napi_remove_env_cleanup_hook(env, Cleanup, &HOOK_ARG_TWO));
    ExpectCheckCall(napi_remove_env_cleanup_hook(env, CleanupCopy, &HOOK_ARG_ONE));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    EXPECT_EQ(HOOK_TAGCP, INT_ONE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Env_Cleanup_Hook_0600
 * @tc.name      : Test napi_remove_env_cleanup_hook Delete napi_add_env_cleanup_hook
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_env_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call RunCleanup to trigger napi_add_env_cleanup_hook
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Env_Cleanup_Hook_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0600 start";
    napi_env env = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    HOOK_TAGCP = INT_ZERO;
    ExpectCheckCall(napi_add_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_add_env_cleanup_hook(env, CleanupCopy, &HOOK_ARG_TWO));
    ExpectCheckCall(napi_remove_env_cleanup_hook(env, Cleanup, &HOOK_ARG_ONE));
    ExpectCheckCall(napi_remove_env_cleanup_hook(env, CleanupCopy, &HOOK_ARG_TWO));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ZERO);
    EXPECT_EQ(HOOK_TAGCP, INT_ZERO);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Env_Cleanup_Hook_0600 end";
}

static void MustNotCall(napi_async_cleanup_hook_handle hook, void* arg)
{
    EXPECT_EQ(1, 0);
}

struct AsyncData {
    uv_async_t async;
    napi_env env;
    napi_async_cleanup_hook_handle handle;
};

static struct AsyncData* CreateAsyncData()
{
    GTEST_LOG_(INFO) << "CreateAsyncData";
    struct AsyncData* data = (struct AsyncData*)malloc(sizeof(struct AsyncData));
    if (data == nullptr) {
        return nullptr;
    }
    data->handle = nullptr;
    return data;
}

static void AfterCleanupHookTwo(uv_handle_t* handle)
{
    GTEST_LOG_(INFO) << "AfterCleanupHookTwo start";
    struct AsyncData* data = (struct AsyncData*)handle->data;
    ExpectCheckCall(napi_remove_async_cleanup_hook(data->handle));
    HOOK_TAG += INT_ONE;
    free(data);
    GTEST_LOG_(INFO) << "AfterCleanupHookTwo end";
}

static void AfterCleanupHookOne(uv_async_t* async)
{
    GTEST_LOG_(INFO) << "AfterCleanupHookOne start";
    uv_close((uv_handle_t*)async, AfterCleanupHookTwo);
    GTEST_LOG_(INFO) << "AfterCleanupHookOne end";
}

static void AsyncCleanupHook(napi_async_cleanup_hook_handle handle, void* arg)
{
    GTEST_LOG_(INFO) << "AsyncCleanupHook start";
    struct AsyncData* data = (struct AsyncData*)arg;
    uv_loop_t* loop;
    ExpectCheckCall(napi_get_uv_event_loop(data->env, &loop));
    int err = uv_async_init(loop, &data->async, AfterCleanupHookOne);
    EXPECT_EQ(err, 0);

    data->async.data = data;
    data->handle = handle;
    uv_async_send(&data->async);
    GTEST_LOG_(INFO) << "AsyncCleanupHook end";
    sleep(1);
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0100
 * @tc.name      : Test napi_add_async_cleanup_hook to pass the normal environment
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call the incoming function normally after the environment exits
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0100 start";

    napi_env env = (napi_env)engine_;
    struct AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    HOOK_TAG = INT_ZERO;
    data->env = env;
    napi_status ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    engine_->RunCleanup();
    EXPECT_EQ(ret, napi_ok);
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0100 end";
    sleep(1);
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0200
 * @tc.name      : Test napi_remove_env_cleanup_hook passing an invalid remove_handle
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call the incoming function normally after the environment exits
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0200 start";

    napi_env env = (napi_env)engine_;
    struct AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    HOOK_TAG = INT_ZERO;
    data->env = env;
    napi_status ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, nullptr);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    EXPECT_EQ(ret, napi_ok);
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0300
 * @tc.name      : Test napi_add_async_cleanup_hook passing an invalid arg
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call the incoming function normally after the environment exits
 *                 6.The function of napi_remove_async_cleanup_hook is called
 *                 7.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0300 start";

    napi_env env = (napi_env)engine_;
    napi_async_cleanup_hook_handle must_not_call_handle;
    HOOK_TAG = INT_ZERO;
    ExpectCheckCall(napi_add_async_cleanup_hook(env, MustNotCall, nullptr, &must_not_call_handle));
    ExpectCheckCall(napi_remove_async_cleanup_hook(must_not_call_handle));
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ZERO);
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0300 start";
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0400
 * @tc.name      : Test napi_add_async_cleanup_hook passing an invalid env
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0400 start";

    napi_status ret = napi_ok;
    napi_async_cleanup_hook_handle must_not_call_handle;
    HOOK_TAG = INT_ZERO;
    ret = napi_add_async_cleanup_hook(nullptr, MustNotCall, nullptr, &must_not_call_handle);
    engine_->RunCleanup();
    EXPECT_EQ(ret, napi_invalid_arg);
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0400 start";
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0500
 * @tc.name      : Test napi_add_async_cleanup_hook passing an invalid hook
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0500 start";

    napi_env env = (napi_env)engine_;
    napi_status ret = napi_ok;
    napi_async_cleanup_hook_handle must_not_call_handle;
    ret = napi_add_async_cleanup_hook(env, nullptr, nullptr, &must_not_call_handle);
    engine_->RunCleanup();
    EXPECT_EQ(ret, napi_invalid_arg);
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0500 start";
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0600
 * @tc.name      : Test napi_add_async_cleanup_hook, after registering the function,
 *                 call remove to delete the registered function, and then re-register the function.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call the incoming function normally after the environment exits
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0600, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0600 start";

    napi_env env = (napi_env)engine_;
    struct AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    data->env = env;
    HOOK_TAG = INT_ZERO;
    napi_status ret = napi_invalid_arg;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    ASSERT_EQ(ret, napi_ok);
    ret = napi_remove_async_cleanup_hook(data->handle);
    ASSERT_EQ(ret, napi_ok);
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    ASSERT_EQ(ret, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ONE);
    EXPECT_EQ(ret, napi_ok);
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0600 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0700
 * @tc.name      : Test napi_add_async_cleanup_hook to pass the normal environment
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call the incoming function normally after the environment exits
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0700, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0700 start";

    napi_env env = (napi_env)engine_;
    napi_env envtwo = (napi_env)engine_;
    HOOK_TAG = INT_ZERO;
    struct AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    data->env = env;
    napi_status ret = napi_invalid_arg;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    EXPECT_EQ(ret, napi_ok);
    struct AsyncData* datatwo = CreateAsyncData();
    if (datatwo == nullptr) {
        return;
    }
    datatwo->env = envtwo;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, datatwo, &datatwo->handle);
    EXPECT_EQ(ret, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_TWO);
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Add_Async_Cleanup_Hook_0800
 * @tc.name      : Test napi_add_async_cleanup_hook to pass the normal environment
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.Call the incoming function normally after the environment exits
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Add_Async_Cleanup_Hook_0800, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0800 start";

    napi_env env = (napi_env)engine_;
    napi_env envtwo = (napi_env)engine_;
    napi_env envthree = (napi_env)engine_;
    struct AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    HOOK_TAG = INT_ZERO;
    data->env = env;
    napi_status ret = napi_invalid_arg;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    EXPECT_EQ(ret, napi_ok);

    struct AsyncData* datatwo = CreateAsyncData();
    if (datatwo == nullptr) {
        return;
    }
    datatwo->env = envtwo;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, datatwo, &datatwo->handle);
    EXPECT_EQ(ret, napi_ok);

    struct AsyncData* datathree = CreateAsyncData();
    datathree->env = envthree;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, datathree, &datathree->handle);
    EXPECT_EQ(ret, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_THREE);
    sleep(2);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Add_Async_Cleanup_Hook_0800 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Async_Cleanup_Hook_0100
 * @tc.name      : Test the normal call of napi_remove_async_cleanup_hook
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_remove_async_cleanup_hook is called
 *                 6.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Async_Cleanup_Hook_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Async_Cleanup_Hook_0100 start";

    napi_env env = (napi_env)engine_;
    struct AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    napi_status ret = napi_invalid_arg;
    HOOK_TAG = INT_ZERO;
    data->env = env;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, &data->handle);
    EXPECT_EQ(ret, napi_ok);
    ret = napi_remove_async_cleanup_hook(data->handle);
    EXPECT_EQ(ret, napi_ok);
    engine_->RunCleanup();
    EXPECT_EQ(HOOK_TAG, INT_ZERO);
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Async_Cleanup_Hook_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Async_Cleanup_Hook_0200
 * @tc.name      : Test napi_remove_async_cleanup_hook passing an invalid remove_handle
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_remove_async_cleanup_hook is called
 *                 4.Return value of function is napi_invalid_arg
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Async_Cleanup_Hook_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Async_Cleanup_Hook_0200 start";

    napi_status ret = napi_ok;
    ret = napi_remove_async_cleanup_hook(nullptr);
    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Async_Cleanup_Hook_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Remove_Async_Cleanup_Hook_0300
 * @tc.name      : Test that napi_add_async_cleanup_hook is registered twice,
 *                 release one of them, and the other is triggered normally
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables
 *                 3.The function of napi_add_async_cleanup_hook is called
 *                 4.Return value of function is napi_ok
 *                 5.The function of napi_remove_async_cleanup_hook is called
 *                 6.Return value of function is napi_ok
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Remove_Async_Cleanup_Hook_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Async_Cleanup_Hook_0300 start";

    napi_env env = (napi_env)engine_;
    napi_env envtwo = (napi_env)engine_;
    struct AsyncData* data = CreateAsyncData();
    if (data == nullptr) {
        return;
    }
    napi_status ret = napi_invalid_arg;
    data->env = env;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, data, nullptr);
    EXPECT_EQ(ret, napi_ok);
    struct AsyncData* datatwo = CreateAsyncData();
    if (datatwo == nullptr) {
        return;
    }
    datatwo->env = envtwo;
    ret = napi_add_async_cleanup_hook(env, AsyncCleanupHook, datatwo, &datatwo->handle);
    EXPECT_EQ(ret, napi_ok);
    ret = napi_remove_async_cleanup_hook(datatwo->handle);
    EXPECT_EQ(ret, napi_ok);
    engine_->RunCleanup();
    sleep(1);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Remove_Async_Cleanup_Hook_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Init_0100
 * @tc.name      : The parameter is valid, the parameter async_resource input the defined date,
 *                 the parameter async_resource_name input string, the parameter result input result.
 * @tc.desc      : 1.Valid environment variable env
 *                 2.parameter async_resource input defined date
 *                 3.parameter async_resource_name input string
 *                 4.parameter result input result
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Init_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0100 start";

    napi_env env = (napi_env)engine_;
    napi_value asyncResource = nullptr;
    double time = 2731123.12;
    napi_value asyncResourceName = nullptr;
    napi_async_context result = nullptr;

    napi_status status = napi_create_date(env, time, &asyncResource);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Async_Init_0100", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, asyncResource, asyncResourceName, &result);

    EXPECT_EQ(ret, napi_ok);
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Init_0200
 * @tc.name      : The parameter is valid, the parameter async_resource is empty,
 *                 the parameter async_resource_name is string,
 *                 and the parameter result is result.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.the parameter async_resource is empty.
 *                 3.the parameter async_resource_name is string.
 *                 4.the parameter result is result.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Init_0200, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0200 start";

    napi_env env = (napi_env)engine_;
    napi_value asyncResource = nullptr;
    napi_value asyncResourceName = nullptr;
    napi_async_context result = nullptr;

    napi_status status = napi_create_string_utf8(env, "ACE_Napi_Async_Init_0200", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, asyncResource, asyncResourceName, &result);
    EXPECT_EQ(ret, napi_ok);
    
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Init_0300
 * @tc.name      : The parameter is invalid, env is empty, the parameter async_resource enters the defined date,
 *                 the parameter async_resource_name enters string, and the parameter result enters result.
 * @tc.desc      : 1.Invalid environment variable env.
 *                 2.parameter async_resource input defined date.
 *                 3.parameter async_resource_name input string.
 *                 4.parameter result input result.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Init_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0300 start";

    napi_env env = (napi_env)engine_;
    napi_value asyncResource = nullptr;
    double time = 2731123.12;
    napi_value asyncResourceName = nullptr;
    napi_async_context result = nullptr;

    napi_status status = napi_create_date(env, time, &asyncResource);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Async_Init_0300", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(nullptr, asyncResource, asyncResourceName, &result);
    EXPECT_EQ(ret, napi_invalid_arg);
    
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Init_0400
 * @tc.name      : The parameter is invalid, the parameter async_resource enters the defined date,
 *                 the parameter async_resource_name enters empty,
 *                 the parameter result enters result.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.the parameter async_resource enters the defined date.
 *                 3.the parameter async_resource_name enters empty.
 *                 4.the parameter result enters result.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Init_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0400 start";

    napi_env env = (napi_env)engine_;
    napi_value asyncResource = nullptr;
    double time = 2731123.12;
    napi_async_context result = nullptr;

    napi_status status = napi_create_date(env, time, &asyncResource);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, asyncResource, nullptr, &result);
    EXPECT_EQ(ret, napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Init_0500
 * @tc.name      : The parameter is invalid, the parameter async_resource input the defined date,
 *                 the parameter async_resource_name input string,the parameter result input is empty.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.the parameter async_resource enters the defined date.
 *                 3.the parameter async_resource_name input string.
 *                 4.the parameter result input is empty.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Init_0500, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0500 start";

    napi_env env = (napi_env)engine_;
    napi_value asyncResource = nullptr;
    double time = 2731123.12;
    napi_value asyncResourceName = nullptr;

    napi_status status = napi_create_date(env, time, &asyncResource);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Async_Init_0500", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, asyncResource, asyncResourceName, nullptr);
    EXPECT_EQ(ret, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Init_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Destroy_0100
 * @tc.name      : The parameter is valid, and the parameter async_context enters the initialized object.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.parameter async_context input initialized object.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Destroy_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0100 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_value asyncResourceName = nullptr;
    napi_async_context result = nullptr;

    napi_status status = napi_create_string_utf8(env, "ACE_Napi_Async_Destroy_0100",
        NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &result);

    EXPECT_EQ(ret, napi_ok);
    EXPECT_NE(result, nullptr);

    napi_status out = napi_async_destroy(env, result);
    EXPECT_EQ(out, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Destroy_0200
 * @tc.name      : The parameter is invalid, the parameter async_context input is empty.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.parameter async_context input is empty.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Destroy_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0200 start";

    napi_env env = (napi_env)engine_;
    napi_async_context result = nullptr;

    napi_status out = napi_async_destroy(env, result);
    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Destroy_0300
 * @tc.name      : The parameter is invalid, the parameter async_context input is empty.
 * @tc.desc      : 1.Invalid environment variable env.
 *                 2.parameter async_context input initialized object.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Destroy_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0300 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_value asyncResourceName = nullptr;
    napi_async_context result = nullptr;

    napi_status status = napi_create_string_utf8(env, "ACE_Napi_Async_Destroy_0300",
        NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0300 napi_create_string_utf8 successful";

    napi_status verification = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);

    EXPECT_EQ(verification, napi_ok);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0300 napi_create_arraybuffer successful";

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &result);
    EXPECT_EQ(ret, napi_ok);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0300 napi_async_init successful";

    napi_status out = napi_async_destroy(nullptr, result);

    EXPECT_EQ(out, napi_invalid_arg);
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Async_Destroy_0400
 * @tc.name      : The parameter is valid, env is empty, and the parameter async_context input is empty
 * @tc.desc      : 1.Invalid environment variable env
 *                 2.parameter async_context input is empty
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Async_Destroy_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0400 start";

    napi_async_context result = nullptr;

    napi_status out = napi_async_destroy(nullptr, result);
    EXPECT_EQ(out, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Async_Destroy_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Open_callback_Scope_0100
 * @tc.name      : The parameter is valid, the resource_object object enters a valid arrayBuffer object,
 *                 the parameter context enters a valid parameter connext,
 *                 the parameter result enters a valid parameter, and the test is confirmed
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.resource_object object input valid arrayBuffer object.
 *                 3.parameter context input valid parameter connext.
 *                 4.parameter result input valid parameter.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Open_callback_Scope_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0100 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_async_context connext = nullptr;
    napi_value asyncResourceName = nullptr;
    napi_callback_scope result = nullptr;

    napi_status status = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Open_callback_Scope_0100", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &connext);
    EXPECT_EQ(ret, napi_ok);

    napi_status out = napi_open_callback_scope(env, arrayBuffer, connext, &result);
    EXPECT_EQ(out, napi_ok);
    EXPECT_NE(result, nullptr);

    status = napi_async_destroy(env, connext);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Open_callback_Scope_0200
 * @tc.name      : The parameter is invalid, the resource_object object is input a valid arrayBuffer object,
 *                 the parameter context is input the valid parameter connext,
 *                 the parameter result is empty, and the test is confirmed.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.the resource_object object is input a valid arrayBuffer object.
 *                 3.parameter context input valid parameter connext.
 *                 4.parameter result input valid parameter.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Open_callback_Scope_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0200 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_async_context connext = nullptr;
    napi_value asyncResourceName = nullptr;

    napi_status status = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Open_callback_Scope_0200", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &connext);
    EXPECT_EQ(ret, napi_ok);

    napi_status out = napi_open_callback_scope(env, arrayBuffer, connext, nullptr);
    EXPECT_EQ(out, napi_invalid_arg);

    status = napi_async_destroy(env, connext);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Open_callback_Scope_0300
 * @tc.name      : The parameter is invalid,  the resource_object object enters a valid arrayBuffer object,
 *                 the parameter context enters a valid parameter connext,
 *                 the parameter result enters a valid parameter, and the test is confirmed.
 * @tc.desc      : 1.Invalid environment variable env.
 *                 2.resource_object object input valid arrayBuffer object.
 *                 3.parameter context input valid parameter connext.
 *                 4.parameter result input valid parameter.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Open_callback_Scope_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0300 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_async_context connext = nullptr;
    napi_value asyncResourceName = nullptr;
    napi_callback_scope result = nullptr;

    napi_status status = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Open_callback_Scope_0300", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &connext);

    EXPECT_EQ(ret, napi_ok);

    napi_status out = napi_open_callback_scope(nullptr, arrayBuffer, connext, &result);
    EXPECT_EQ(out, napi_invalid_arg);
    EXPECT_EQ(result, nullptr);

    status = napi_async_destroy(env, connext);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Open_callback_Scope_0400
 * @tc.name      : The parameter is invalid, the resource_object object input is empty,
 *                 the parameter context input valid parameter connext,
 *                 the parameter result input valid parameter, and the test is confirmed.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.the resource_object object input is empty.
 *                 3.parameter context input valid parameter connext.
 *                 4.parameter result input valid parameter.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Open_callback_Scope_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0400 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_async_context connext = nullptr;
    napi_value asyncResourceName = nullptr;
    napi_callback_scope result = nullptr;

    napi_status status = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Open_callback_Scope_0400", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &connext);
    EXPECT_EQ(ret, napi_ok);

    napi_status out = napi_open_callback_scope(env, nullptr, connext, &result);
    EXPECT_EQ(out, napi_ok);
    EXPECT_NE(result, nullptr);

    status = napi_async_destroy(env, connext);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Open_callback_Scope_0500
 * @tc.name      : The parameter is invalid, the resource_object object is input a valid arrayBuffer object,
 *                 the parameter context is empty, and the parameter result is a valid parameter,
 *                 and the test is confirmed.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.resource_object object input valid arrayBuffer object.
 *                 3.the parameter result is a valid parameter.
 *                 4.parameter result input valid parameter.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Open_callback_Scope_0500, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0500 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_async_context connext = nullptr;
    napi_value asyncResourceName = nullptr;
    napi_callback_scope result = nullptr;

    napi_status status = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Open_callback_Scope_0500", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &connext);
    EXPECT_EQ(ret, napi_ok);

    napi_status out = napi_open_callback_scope(env, arrayBuffer, nullptr, &result);
    EXPECT_EQ(out, napi_ok);
    EXPECT_NE(result, nullptr);

    status = napi_async_destroy(env, connext);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Open_callback_Scope_0500 end";
}

/*
 * @tc.number    : ACE_Napi_Close_callback_Scope_0100
 * @tc.name      : The parameter is valid, and the scope object enters the newly created associated scope.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.scope object enters the newly created associated scope.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Close_callback_Scope_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0100 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_async_context connext = nullptr;
    napi_value asyncResourceName = nullptr;
    napi_callback_scope result = nullptr;

    napi_status status = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Close_callback_Scope_0100", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &connext);
    EXPECT_EQ(ret, napi_ok);

    napi_status out = napi_open_callback_scope(env, arrayBuffer, connext, &result);
    EXPECT_EQ(out, napi_ok);
    EXPECT_NE(result, nullptr);

    napi_status output = napi_close_callback_scope(env, result);
    EXPECT_EQ(output, napi_ok);

    status = napi_async_destroy(env, connext);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0100 end";
}

/*
 * @tc.number    : ACE_Napi_Close_callback_Scope_0200
 * @tc.name      : The parameter is invalid, the scope object input is empty.
 * @tc.desc      : 1.Valid environment variable env.
 *                 2.scope object input is empty.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Close_callback_Scope_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0200 start";

    napi_env env = (napi_env)engine_;

    napi_callback_scope result = nullptr;

    napi_status output = napi_close_callback_scope(env, result);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0200 end";
}

/*
 * @tc.number    : ACE_Napi_Close_callback_Scope_0300
 * @tc.name      : The parameter is invalid, env is empty, and the scope object enters the newly created associated
 *                  scope.
 * @tc.desc      : 1.Invalid environment variable env.
 *                 2.scope object enters the newly created associated scope.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Close_callback_Scope_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0300 start";

    napi_env env = (napi_env)engine_;
    void* arrayBufferPtr = nullptr;
    napi_value arrayBuffer = nullptr;
    size_t arrayBufferSize = ARRAYBUFFER_SIZE;
    napi_async_context connext = nullptr;
    napi_value asyncResourceName = nullptr;
    napi_callback_scope result = nullptr;

    napi_status status = napi_create_arraybuffer(env, arrayBufferSize, &arrayBufferPtr, &arrayBuffer);
    EXPECT_EQ(status, napi_ok);

    status = napi_create_string_utf8(env, "ACE_Napi_Close_callback_Scope_0300", NAPI_AUTO_LENGTH, &asyncResourceName);
    EXPECT_EQ(status, napi_ok);

    napi_status ret = napi_async_init(env, arrayBuffer, asyncResourceName, &connext);

    EXPECT_EQ(ret, napi_ok);

    napi_status out = napi_open_callback_scope(env, arrayBuffer, connext, &result);
    EXPECT_EQ(out, napi_ok);
    EXPECT_NE(result, nullptr);

    napi_status output = napi_close_callback_scope(nullptr, result);
    EXPECT_EQ(output, napi_invalid_arg);

    status = napi_async_destroy(env, connext);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0300 end";
}

/*
 * @tc.number    : ACE_Napi_Close_callback_Scope_0400
 * @tc.name      : The parameter is invalid, env is empty, scope object input is empty.
 * @tc.desc      : 1.Invalid environment variable env
 *                 2.scope object input is empty
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Close_callback_Scope_0400, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0400 start";

    napi_status output = napi_close_callback_scope(nullptr, nullptr);
    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Close_callback_Scope_0400 end";
}

/*
 * @tc.number    : ACE_Napi_Fatal_Exception_0700
 * @tc.name      : napi_fatal_exception is passed to the JS uncaughtException error message is invalid nullptr,
 *                 triggering uncaughtException exception failure.
 * @tc.desc      : 1.The environment engine is created.
 *                 2.Set test variables.
 *                 3.The function of napi_fatal_exception is called.
 *                 4.Return value of function is napi_invalid_arg.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Fatal_Exception_0700, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Fatal_Exception_0700 start";

    napi_env env = (napi_env)engine_;

    napi_status output = napi_fatal_exception(env, nullptr);

    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Fatal_Exception_0700 end";
}

/*
 * @tc.number    : ACE_Napi_Fatal_Exception_0800
 * @tc.name      : The environment parameter of napi_fatal_exception nullptr,
 *                 triggering uncaughtException failed.
 * @tc.desc      : 1.The environment engine is created
 *                 2.Set test variables.
 *                 3.The function of napi_fatal_exception is called.
 *                 4.Return value of function is napi_invalid_arg.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Fatal_Exception_0800, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Fatal_Exception_0800 start";

    napi_env env = (napi_env)engine_;

    napi_value err = nullptr;
    size_t sizeLen = sizeof("uncaughtException");
    napi_status verification = napi_create_string_utf8(env, "uncaughtException", sizeLen, &err);

    EXPECT_EQ(verification, napi_ok);
    napi_status output = napi_fatal_exception(nullptr, err);

    EXPECT_EQ(output, napi_invalid_arg);

    GTEST_LOG_(INFO) << "NativeEngineTest ACE_Napi_Fatal_Exception_0800 end";
}

/*
 * @tc.number    : ACE_Napi_Ref_Threadsafe_Function_0100
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue
 *                3.Call napi_ref_threadsafe_function to set ref flag
 *                4.Call uv_has_ref to check if ref flag hasbeen set.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Ref_Threadsafe_Function_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0100 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_bIsFinish = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr, resourceName,
        0, 1, &g_finalData, FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_ref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(tsFunc);

    auto ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->asyncHandler_)));
    EXPECT_EQ(ret, 1);
    ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->idleHandler_)));
    EXPECT_EQ(ret, 1);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();

    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0100 called end";
}

/*
 * @tc.number    : ACE_Napi_Ref_Threadsafe_Function_0200
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue
 *                3.Call napi_ref_threadsafe_function with invalid param
 *                4.Check if napi_ref_threadsafe_function fails
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Ref_Threadsafe_Function_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0200 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_bIsFinish = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr,
        resourceName, 0, 1, &g_finalData, FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_ref_threadsafe_function(env, nullptr);
    EXPECT_NE(status, napi_ok);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();

    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0200 called end";
}

/*
 * @tc.number    : ACE_Napi_Ref_Threadsafe_Function_0300
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue
 *                3.Create child thread napi_ref_threadsafe_function to set ref flag
 *                4.Check if ref flag has been set.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Ref_Threadsafe_Function_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0300 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_bIsFinish = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr,
        resourceName, 0, 1, &g_finalData, FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_unref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    g_stEnv = env;
    if (uv_thread_create(&newChildTid, NewChildRef, tsFunc) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join Failed!";
    }

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(tsFunc);

    auto ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->asyncHandler_)));
    EXPECT_EQ(ret, 0);
    ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->idleHandler_)));
    EXPECT_EQ(ret, 0);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();

    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0300 called end";
}

/*
 * @tc.number    : ACE_Napi_Ref_Threadsafe_Function_0400
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue
 *                3.Call uv_has_ref to check if ref flag hasbeen set.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Ref_Threadsafe_Function_0400, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0400 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_bIsFinish = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr,
        resourceName, 0, 1, &g_finalData, FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(tsFunc);
    auto ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->asyncHandler_)));
    EXPECT_EQ(ret, 1);
    ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->idleHandler_)));
    EXPECT_EQ(ret, 1);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();

    GTEST_LOG_(INFO) << "ACE_Napi_Ref_Threadsafe_Function_0400 called end";
}

/*
 * @tc.number    : ACE_Napi_Unref_Threadsafe_Function_0100
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue
 *                3.Call napi_unref_threadsafe_function to set ref flag
 *                4.Call uv_has_ref to check if ref flag hasbeen set.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Unref_Threadsafe_Function_0100, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Unref_Threadsafe_Function_0100 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_bIsFinish = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr,
       resourceName, 0, 1, &g_finalData, FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_unref_threadsafe_function(env, tsFunc);
    EXPECT_EQ(status, napi_ok);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(tsFunc);

    auto ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->asyncHandler_)));
    EXPECT_EQ(ret, 0);
    ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->idleHandler_)));
    EXPECT_EQ(ret, 0);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();

    GTEST_LOG_(INFO) << "ACE_Napi_Unref_Threadsafe_Function_0100 called end";
}

/*
 * @tc.number    : ACE_Napi_Unref_Threadsafe_Function_0200
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue
 *                3.Call napi_unref_threadsafe_function with invalid param
 *                4.Check if napi_unref_threadsafe_function fails
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Unref_Threadsafe_Function_0200, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Unref_Threadsafe_Function_0200 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_bIsFinish = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr,
        resourceName, 0, 1, &g_finalData, FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    status = napi_unref_threadsafe_function(env, nullptr);
    EXPECT_NE(status, napi_ok);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();

    GTEST_LOG_(INFO) << "ACE_Napi_Unref_Threadsafe_Function_0200 called end";
}

/*
 * @tc.number    : ACE_Napi_Unref_Threadsafe_Function_0300
 * @tc.name      : napi_create_threadsafe_function creates a queue and
 *                 test napi_call_threadsafe_function none blocking mode with the queue full.
 * @tc.desc      :1.The environment engine is created.
 *                2.napi_create_threadsafe_function creates a queue
 *                3.Create child thread napi_unref_threadsafe_function to set ref flag
 *                4.Check if ref flag has been set.
 */
HWTEST_F(NativeEngineTest, ACE_Napi_Unref_Threadsafe_Function_0300, testing::ext::TestSize.Level2)
{
    GTEST_LOG_(INFO) << "ACE_Napi_Unref_Threadsafe_Function_0300 called start";
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;
    g_bIsFinish = false;

    auto status = napi_create_threadsafe_function(env, nullptr, nullptr,
        resourceName, 0, 1, &g_finalData, FinalCallBack, &g_jsData, CallJSSlowCallBack, &tsFunc);
    EXPECT_EQ(status, napi_ok);

    uv_thread_t newChildTid;
    g_stEnv = env;
    if (uv_thread_create(&newChildTid, NewChildUnRef, tsFunc) != 0) {
        GTEST_LOG_(INFO) << "NewChildThreadMuti Failed to create uv thread!";
    }
    if (uv_thread_join(&newChildTid) != 0) {
        GTEST_LOG_(INFO) << "uv_thread_join(&newChildTid)!";
    }

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(tsFunc);

    auto ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->asyncHandler_)));
    EXPECT_EQ(ret, 1);
    ret = uv_has_ref(reinterpret_cast<uv_handle_t*>(&(safeAsyncWork->idleHandler_)));
    EXPECT_EQ(ret, 1);

    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
    WaitForFinish();

    GTEST_LOG_(INFO) << "ACE_Napi_Unref_Threadsafe_Function_0300 called end";
}