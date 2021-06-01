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

#include "native_async_work.h"

#include "native_engine.h"
#include "utils/log.h"

NativeAsyncWork::NativeAsyncWork(NativeEngine* engine,
                                 NativeAsyncExecuteCallback execute,
                                 NativeAsyncCompleteCallback complete,
                                 void* data)
{
    work_ = { 0 };
    work_.data = this;
    engine_ = engine;
    status_ = 0;
    execute_ = execute;
    complete_ = complete;
    data_ = data;
}

NativeAsyncWork::~NativeAsyncWork() {}

bool NativeAsyncWork::Queue()
{
    uv_loop_t* loop = engine_->GetUVLoop();
    if (loop == nullptr) {
        HILOG_ERROR("Get loop failed");
        return false;
    }

    int status = uv_queue_work(loop, &work_, AsyncWorkCallback, AsyncAfterWorkCallback);
    if (status != 0) {
        HILOG_ERROR("uv_queue_work failed");
        return false;
    }
    return true;
}

bool NativeAsyncWork::Cancel()
{
    int status = uv_cancel((uv_req_t*)&work_);
    if (status != 0) {
        HILOG_ERROR("uv_cancel failed");
        return false;
    }
    return true;
}

void NativeAsyncWork::AsyncWorkCallback(uv_work_t* req)
{
    if (req == nullptr) {
        HILOG_ERROR("req is nullptr");
        return;
    }

    auto that = reinterpret_cast<NativeAsyncWork*>(req->data);
    that->execute_(that->engine_, that->data_);
}

void NativeAsyncWork::AsyncAfterWorkCallback(uv_work_t* req, int status)
{
    if (req == nullptr) {
        HILOG_ERROR("req is nullptr");
        return;
    }

    auto that = reinterpret_cast<NativeAsyncWork*>(req->data);

    NativeScopeManager* scopeManager = that->engine_->GetScopeManager();
    if (scopeManager == nullptr) {
        HILOG_ERROR("Get scope manager failed");
        return;
    }

    NativeScope* scope = scopeManager->Open();
    if (scope == nullptr) {
        HILOG_ERROR("Open scope failed");
        return;
    }

    that->complete_(that->engine_, status, that->data_);
    scopeManager->Close(scope);
}
