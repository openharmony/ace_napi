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

#include "test.h"

#include "quickjs_native_engine.h"

static NativeEngine* g_nativeEngine = nullptr;

NativeEngineTest::NativeEngineTest()
{
    engine_ = g_nativeEngine;
}

NativeEngineTest::~NativeEngineTest() {}

static void LoopNewThread(void* data)
{
    g_nativeEngine->Loop(LOOP_DEFAULT);
}

int main(int argc, char** argv)
{
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);

    JSRuntime* rt = JS_NewRuntime();
    if (rt == nullptr) {
        return 0;
    }

    JSContext* ctx = JS_NewContext(rt);
    if (ctx == nullptr) {
        return 0;
    }

    js_std_add_helpers(ctx, 0, nullptr);

    g_nativeEngine = new QuickJSNativeEngine(rt, ctx, 0); // default instance id 0

    uv_thread_t tid;
    uv_thread_create(&tid, LoopNewThread, nullptr);

    int ret = RUN_ALL_TESTS();

    uv_thread_join(&tid);

    delete g_nativeEngine;
    g_nativeEngine = nullptr;

    js_std_free_handlers(rt);
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);

    return ret;
}
