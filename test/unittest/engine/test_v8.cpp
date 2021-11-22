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
#include "v8_native_engine.h"

static NativeEngine* g_nativeEngine = nullptr;

NativeEngineTest::NativeEngineTest()
{
    engine_ = g_nativeEngine;
}

NativeEngineTest::~NativeEngineTest() {}

extern const char _binary_strip_native_min_js_bin_start[];
extern const char _binary_strip_native_min_js_bin_end[];

int main(int argc, char** argv)
{
    int retCode = 0;

    testing::GTEST_FLAG(output) = "xml:./";
    testing::InitGoogleTest(&argc, argv);

    static v8::StartupData snapshotBlob = {
        .data = _binary_strip_native_min_js_bin_start,
        .raw_size = _binary_strip_native_min_js_bin_end - _binary_strip_native_min_js_bin_start,
    };
    v8::V8::SetSnapshotDataBlob(&snapshotBlob);

    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    if (!v8::V8::Initialize()) {
        return retCode;
    }
    v8::Isolate::CreateParams createParams;
    createParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();

    if (createParams.array_buffer_allocator == nullptr) {
        return retCode;
    }

    v8::Isolate* isolate = v8::Isolate::New(createParams);
    {
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);
        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
        v8::Local<v8::Context> context = v8::Context::New(isolate, nullptr, global);

        v8::Persistent<v8::Context> persistContext;
        persistContext.Reset(isolate, context);

        if (context.IsEmpty()) {
            return retCode;
        }

        v8::Context::Scope contextScope(context);
        {
            g_nativeEngine = new V8NativeEngine(platform.get(), isolate, persistContext, 0); // default instance id 0

            retCode = testing::UnitTest::GetInstance()->Run();

            g_nativeEngine->Loop(LOOP_DEFAULT);
            delete g_nativeEngine;
            g_nativeEngine = nullptr;
        }
    }
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::ShutdownPlatform();
    delete createParams.array_buffer_allocator;

    return retCode;
}
