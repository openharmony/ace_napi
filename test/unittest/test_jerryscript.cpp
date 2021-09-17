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

NativeEngineTest::NativeEngineTest()
{
    engine_ = g_nativeEngine;
}

NativeEngineTest::~NativeEngineTest() {}

int main(int argc, char **argv)
{
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);

    jerry_init(jerry_init_flag_t::JERRY_INIT_EMPTY);
    g_nativeEngine = new JerryScriptNativeEngine(0); // default instance id 0
    int ret = RUN_ALL_TESTS();
    g_nativeEngine->Loop(LoopMode::LOOP_DEFAULT);
    delete g_nativeEngine;
    g_nativeEngine = nullptr;
    jerry_cleanup();

    return ret;
}
