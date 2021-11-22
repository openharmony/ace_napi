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

#include "utils/log.h"
#include "ark_native_engine.h"

using panda::RuntimeOption;
static NativeEngine* g_nativeEngine = nullptr;

NativeEngineTest::NativeEngineTest()
{
    engine_ = g_nativeEngine;
}

NativeEngineTest::~NativeEngineTest()
{}

int main(int argc, char** argv)
{
    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);

    // Setup
    RuntimeOption option;
    option.SetGcType(RuntimeOption::GC_TYPE::GEN_GC);
    // const int64_t poolSize = 0x10000000;  // 256M
    const int64_t poolSize = 0x1000000;  // 16M
    option.SetGcPoolSize(poolSize);
    // option.SetPandaStdFile("pandastdlib/arkstdlib.abc");
    option.SetLogLevel(RuntimeOption::LOG_LEVEL::ERROR);
    option.SetDebuggerLibraryPath("");
    EcmaVM* vm = panda::JSNApi::CreateJSVM(option);
    if (vm == nullptr) {
        return 0;
    }

    g_nativeEngine = new ArkNativeEngine(vm, nullptr);

    int ret = testing::UnitTest::GetInstance()->Run();

    g_nativeEngine->Loop(LOOP_NOWAIT);

    delete g_nativeEngine;
    g_nativeEngine = nullptr;
    panda::JSNApi::DestoryJSVM(vm);
    vm = nullptr;

    return ret;
}
