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

#include "jerryscript_native_engine.h"

static NativeEngine* g_nativeEngine = nullptr;
static constexpr size_t JERRY_SCRIPT_MEM_SIZE = 50 * 1024 * 1024;

NativeEngineTest::NativeEngineTest()
{
    engine_ = g_nativeEngine;
}

NativeEngineTest::~NativeEngineTest() {}

static void* context_alloc_fn(size_t size, void* cb_data)
{
    (void)cb_data;
    size_t newSize = size > JERRY_SCRIPT_MEM_SIZE ? JERRY_SCRIPT_MEM_SIZE : size;
    return malloc(newSize);
}

int main(int argc, char** argv)
{
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);
    // allocate 50MB space
    jerry_context_t* ctx_p = jerry_create_context(1024 * 1024 * 50, context_alloc_fn, NULL);

    jerry_port_default_set_current_context(ctx_p);

    jerry_init(jerry_init_flag_t::JERRY_INIT_EMPTY);

    g_nativeEngine = new JerryScriptNativeEngine(0); // default instance id 0

    uv_thread_t tid;

    int ret = RUN_ALL_TESTS();
    g_nativeEngine->Loop(LOOP_DEFAULT);

    uv_thread_join(&tid);

    delete g_nativeEngine;
    g_nativeEngine = nullptr;
    jerry_cleanup();
    free(ctx_p);
    return ret;
}
