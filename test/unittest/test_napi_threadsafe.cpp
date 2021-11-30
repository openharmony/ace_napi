/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http:// www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "test.h"

#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"
#include "utils/log.h"

struct CallJsCbData_t {
    int32_t id = 0;
    bool secondaryThread = false;
    napi_threadsafe_function_call_mode blockOnFull = napi_tsfn_nonblocking;
};

struct FinalCbData_t {
    int32_t id = 0;
};

static constexpr int32_t SEND_DATA_TEST = 11;
static constexpr int32_t CALL_JS_CB_DATA_TEST_ID = 101;
static constexpr int32_t FINAL_CB_DATA_TEST_ID = 1001;
static constexpr int32_t SEND_DATAS_LENGTH = 10;
static constexpr int32_t THREAD_COUNT = 2;
static constexpr int32_t THREAD_COUNT_FOUR = 4;
static constexpr int32_t MAX_QUEUE_SIZE = 3;
static constexpr int32_t SUCCESS_COUNT_JS_FOUR = 4;

static pid_t g_mainTid = 0;
static CallJsCbData_t g_jsData;
static CallJsCbData_t g_jsDataInternal;
static FinalCbData_t g_finalData;
static int32_t g_sendData = 0;
static uv_thread_t g_uvThread;
static uv_thread_t g_uvThreadTest5;
static uv_thread_t g_uvThreadTest6;
static uv_thread_t g_uvThreadTest7;
static uv_thread_t g_uvThreadSecondary;
static uv_thread_t g_uvTheads2;
static uv_thread_t g_uvTheads3;
static int32_t g_sendDatas[SEND_DATAS_LENGTH];
static int32_t  callSuccessCount = 0;
static int32_t  callSuccessCountJS = 0;
static int32_t  callSuccessCountJSFour = 0;
bool  acquireFlag = false;

static void TsFuncCallJs(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    HILOG_INFO("TsFuncCallJs called");

    EXPECT_EQ(gettid(), g_mainTid);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    // expect data equal
    int* pData = (int32_t*)data;
    EXPECT_EQ((*pData), SEND_DATA_TEST);
}
static void TsFuncCallJsTwo(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    HILOG_INFO("TsFuncCallJsTwo called");
    TsFuncCallJs(env, tsfn_cb, context, data);
    callSuccessCountJS++;
}
static void TsFuncCallJsFour(napi_env env, napi_value tsfn_cb, void* context, void* data)
{
    HILOG_INFO("TsFuncCallJsFour called");

    TsFuncCallJs(env, tsfn_cb, context, data);
    callSuccessCountJSFour++;
}
static void TsFuncCallJsMulti(napi_env env,
                              napi_value tsfn_cb,
                              void* context,
                              void* data)
{
    HILOG_INFO("TsFuncCallJsMulti called");

    EXPECT_EQ(gettid(), g_mainTid);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    int* pData = ((int32_t*)data);

    HILOG_INFO("TsFuncCallJsMulti data %d", (*pData));
}

static void TsFuncFinal(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinal called");
    
    // expect thread id equal
    EXPECT_EQ(gettid(), g_mainTid);

    // wait data source thread
    uv_thread_join(&g_uvThread);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)hint)->id, CALL_JS_CB_DATA_TEST_ID);

    // expect finalize data equal
    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);
}
static void TsFuncFinalTest5(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinalTest5 called");
    
    // expect thread id equal
    EXPECT_EQ(gettid(), g_mainTid);

    // wait data source thread
    uv_thread_join(&g_uvThreadTest5);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)hint)->id, CALL_JS_CB_DATA_TEST_ID);

    // expect finalize data equal
    EXPECT_EQ(((FinalCbData_t*)finalizeData)->id, FINAL_CB_DATA_TEST_ID);
}
static void TsFuncFinalTotal(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinalTotal called");
    uv_thread_join(&g_uvThreadTest6);
    // when add thread,repair  callSuccessCountJS eq  SUCCESS_COUNT_JS_TWO
    EXPECT_EQ(callSuccessCountJS, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("TsFuncFinalTotal end");
}
static void TsFuncFinalTotalFour(napi_env env, void* finalizeData, void* hint)
{
    HILOG_INFO("TsFuncFinalTotalFour called");
    uv_thread_join(&g_uvThreadTest7);
    EXPECT_EQ(callSuccessCountJSFour, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("TsFuncFinalTotalFour end");
}

static void TsFuncFinalJoinThread(napi_env env, void* data, void* hint)
{
    HILOG_INFO("TsFuncFinalJoinThread called");

    uv_thread_t *uvThread = reinterpret_cast<uv_thread_t*>(data);
    CallJsCbData_t *jsData = reinterpret_cast<CallJsCbData_t*>(hint);

    uv_thread_join(uvThread);

    if (jsData->secondaryThread) {
        uv_thread_join(&g_uvThreadSecondary);
    }
}

static void TsFuncSecondaryThread(void* data)
{
    HILOG_INFO("TsFuncSecondaryThread called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;

    auto status = napi_release_threadsafe_function(func, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
}

static void TsFuncDataSourceThread(void* data)
{
    HILOG_INFO("TsFuncDataSourceThread called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode blockMode = napi_tsfn_nonblocking;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    // set send data
    g_sendData = SEND_DATA_TEST;

    // As main thread has set initial_thread_count to 1 and only this one secondary thread,
    // so no need to call `napi_acquire_threadsafe_function()`.
    status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
    EXPECT_EQ(status, napi_ok);

    status = napi_release_threadsafe_function(func, napi_tsfn_release);
    EXPECT_EQ(status, napi_ok);
}
static void TsFuncDataSourceThreadAbort(void* data)
{
    HILOG_INFO("TsFuncDataSourceThreadAbort called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode blockMode = napi_tsfn_nonblocking;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);

    // set send data
    g_sendData = SEND_DATA_TEST;

    status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
    EXPECT_EQ(status, napi_closing);
}

static void TsFuncDataSourceThreadCountTotal(void* data)
{
    HILOG_INFO("TsFuncDataSourceThreadCountTotal called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func = (napi_threadsafe_function)data;
    napi_threadsafe_function_call_mode blockMode = napi_tsfn_nonblocking;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);

    // expect context equal
    EXPECT_EQ(((CallJsCbData_t*)context)->id, CALL_JS_CB_DATA_TEST_ID);
    // set send data
    g_sendData = SEND_DATA_TEST;
    if (acquireFlag) {
        std::cout<<"acquireFlag  is true"<<std::endl;
        status = napi_acquire_threadsafe_function(func);
        EXPECT_EQ(status, napi_ok);
        status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
        if (status == napi_ok) {
            callSuccessCount++;
        }
        status = napi_release_threadsafe_function(func, napi_tsfn_release);
    } else {
        status = napi_call_threadsafe_function(func, &g_sendData, blockMode);
        if (status == napi_ok) {
            callSuccessCount++;
        }
    }
    status = napi_release_threadsafe_function(func, napi_tsfn_release);
}

static void TsFuncDataSourceThreadMulti(void* data)
{
    HILOG_INFO("TsFuncDataSourceThreadMulti called");

    // expect thread id not equal
    EXPECT_NE(gettid(), g_mainTid);

    napi_threadsafe_function func =  (napi_threadsafe_function)data;
    void* context = nullptr;

    auto status = napi_get_threadsafe_function_context(func, &context);
    EXPECT_EQ(status, napi_ok);
    CallJsCbData_t* jsData = nullptr;
    jsData = (CallJsCbData_t*)context;

    if (jsData->secondaryThread) {
        status = napi_acquire_threadsafe_function(func);
        EXPECT_EQ(status, napi_ok);

        if (uv_thread_create(&g_uvThreadSecondary, TsFuncSecondaryThread, func) != 0) {
            HILOG_ERROR("Failed to create uv thread!");
        }
    }

    bool queueClosing = false;
    bool queueFull = false;
    int32_t index = 0;
    for (index = SEND_DATAS_LENGTH - 1; index > -1 && !queueClosing; index--) {
        g_sendDatas[index] = index;
        status = napi_call_threadsafe_function(func, &g_sendDatas[index], jsData->blockOnFull);
        HILOG_INFO("napi_call_threadsafe_function index %d status %d", index, status);

        switch (status) {
            case napi_queue_full:
                queueFull = true;
                index++;
                [[fallthrough]];
            case napi_ok:
                continue;
            case napi_closing:
                queueClosing = true;
                break;
            default:
                HILOG_ERROR("Failed to call napi_call_threadsafe_function!");
        }
    }

    if (!queueClosing && (napi_release_threadsafe_function(func, napi_tsfn_release) != napi_ok)) {
        HILOG_ERROR("Failed to call napi_release_threadsafe_function!");
    }
}

static void TsFuncThreadInternal(napi_env env,
                                 napi_threadsafe_function_call_js cb,
                                 uv_thread_t& uvThread,
                                 bool secondary,
                                 bool block)
{
    HILOG_INFO("TsFuncThreadInternal start cb %p secondary %d block %d", cb, secondary, block);

    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();

    g_jsDataInternal.id = CALL_JS_CB_DATA_TEST_ID;
    g_jsDataInternal.secondaryThread = (secondary ? true : false);
    g_jsDataInternal.blockOnFull = (block ? napi_tsfn_blocking : napi_tsfn_nonblocking);

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  MAX_QUEUE_SIZE,
                                                  THREAD_COUNT,
                                                  &uvThread,
                                                  TsFuncFinalJoinThread,
                                                  &g_jsDataInternal,
                                                  cb,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);

    if (uv_thread_create(&uvThread, TsFuncDataSourceThreadMulti, tsFunc) != 0) {
        HILOG_ERROR("Failed to create uv thread!");
    }

    HILOG_INFO("TsFuncThreadInternal end");
}

/**
 * @tc.name: ThreadsafeTest
 * @tc.desc: Test LoadModule Func.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, Threadsafe_Test_0100, testing::ext::TestSize.Level0)
{
    HILOG_INFO("Threadsafe_Test_0100 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  1,
                                                  &g_finalData,
                                                  TsFuncFinal,
                                                  &g_jsData,
                                                  TsFuncCallJs,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);

    if (uv_thread_create(&g_uvThread, TsFuncDataSourceThread, tsFunc) != 0) {
        HILOG_ERROR("Failed to create uv thread!");
    }

    HILOG_INFO("Threadsafe_Test_0100 end");
}

/**
 * @tc.name: ThreadsafeTest
 * @tc.desc: Test LoadModule Func.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, Threadsafe_Test_0200, testing::ext::TestSize.Level0)
{
    HILOG_INFO("Threadsafe_Test_0200 start");

    // start secondary thread, block on full
    TsFuncThreadInternal((napi_env)engine_, TsFuncCallJsMulti, g_uvTheads2, true, true);

    HILOG_INFO("Threadsafe_Test_0200 end");
}

/**
 * @tc.name: ThreadsafeTest
 * @tc.desc: Test threadsafe Func, no js.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, Threadsafe_Test_0300, testing::ext::TestSize.Level0)
{
    HILOG_INFO("Threadsafe_Test_0300 start");

    // secondary thread, not block
    TsFuncThreadInternal((napi_env)engine_, TsFuncCallJsMulti, g_uvTheads3, false, false);

    HILOG_INFO("Threadsafe_Test_0300 end");
}

/**
 * @tc.name: ThreadsafeTest
 * @tc.desc: Test napi_release_threadsafe_function, napi_tsfn_abort.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, Threadsafe_Test_0400, testing::ext::TestSize.Level0)
{
    HILOG_INFO("Threadsafe_Test_0400 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;

    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  10,
                                                  &g_finalData,
                                                  TsFuncFinalTest5,
                                                  &g_jsData,
                                                  TsFuncCallJs,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);
    status = napi_release_threadsafe_function(tsFunc, napi_tsfn_abort);
    EXPECT_EQ(status, napi_ok);
    if (uv_thread_create(&g_uvThreadTest5, TsFuncDataSourceThreadAbort, tsFunc) != 0) {
        HILOG_ERROR("Failed to create uv thread!");
    }

    HILOG_INFO("Threadsafe_Test_0400 end");
}

/**
 * @tc.name: ThreadsafeTest
 * @tc.desc: Test initial_thread_count not enough.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, Threadsafe_Test_0500, testing::ext::TestSize.Level0)
{
    HILOG_INFO("Threadsafe_Test_0500 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    callSuccessCountJS=0;
    callSuccessCount=0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  2,
                                                  &g_finalData,
                                                  TsFuncFinalTotal,
                                                  &g_jsData,
                                                  TsFuncCallJsTwo,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);
    int threadCount = THREAD_COUNT_FOUR;
    acquireFlag = false;

    for (int i = 0; i < threadCount; i++) {
        if (uv_thread_create(&g_uvThreadTest6, TsFuncDataSourceThreadCountTotal, tsFunc) != 0) {
            HILOG_ERROR("Failed to create uv thread!");
        }
    }

    usleep(200 * 1000);
    EXPECT_EQ(callSuccessCount, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("Threadsafe_Test_0500 end");
}
/**
 * @tc.name: ThreadsafeTest
 * @tc.desc: Test initial_thread_count not enough，but acquire.
 * @tc.type: FUNC
 */
HWTEST_F(NativeEngineTest, Threadsafe_Test_0600, testing::ext::TestSize.Level0)
{
    HILOG_INFO("Threadsafe_Test_0600 start");
    napi_env env = (napi_env)engine_;
    napi_threadsafe_function tsFunc = nullptr;
    napi_value resourceName = 0;
    callSuccessCount=0;
    napi_create_string_latin1(env, __func__, NAPI_AUTO_LENGTH, &resourceName);
    g_mainTid = gettid();
    g_jsData.id = CALL_JS_CB_DATA_TEST_ID;
    g_finalData.id = FINAL_CB_DATA_TEST_ID;

    auto status = napi_create_threadsafe_function(env,
                                                  nullptr,
                                                  nullptr,
                                                  resourceName,
                                                  0,
                                                  1,
                                                  &g_finalData,
                                                  TsFuncFinalTotalFour,
                                                  &g_jsData,
                                                  TsFuncCallJsFour,
                                                  &tsFunc);
    EXPECT_EQ(status, napi_ok);
    int threadCount = THREAD_COUNT_FOUR;
    acquireFlag=true;

    for (int i = 0; i < threadCount; i++) {
        if (uv_thread_create(&g_uvThreadTest7, TsFuncDataSourceThreadCountTotal, tsFunc) != 0) {
            HILOG_ERROR("Failed to create uv thread!");
        }
    }

    usleep(200 * 1000);
    EXPECT_EQ(callSuccessCount, SUCCESS_COUNT_JS_FOUR);
    HILOG_INFO("Threadsafe_Test_0600 end");
}