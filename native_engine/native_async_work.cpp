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

#include "napi/native_api.h"
#include "native_engine.h"
#include "utils/log.h"

NativeAsyncWork::NativeAsyncWork(NativeEngine* engine,
                                 NativeAsyncExecuteCallback execute,
                                 NativeAsyncCompleteCallback complete,
                                 void* data)
    : work_({ 0 }), engine_(engine), execute_(execute), complete_(complete), data_(data)
{
    work_.data = this;
}

NativeAsyncWork::~NativeAsyncWork() = default;

void NativeAsyncWork::Close()
{
    if (uv_is_active((uv_handle_t*)&workAsyncHandler_)) {
        uv_close((uv_handle_t*)&workAsyncHandler_, nullptr);
    }
}

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

void NativeAsyncWork::AsyncWorkRecvCallback(const uv_async_t* req)
{
    NativeAsyncWork* that = NativeAsyncWork::DereferenceOf(&NativeAsyncWork::workAsyncHandler_, req);
    if (that == nullptr) {
        return;
    }
    NativeAsyncWorkDataPointer res;
    while (that->PopData(&res)) {
        if (that->execute_ != nullptr) {
            that->execute_(that->engine_, res.data_);
        }
        if (that->complete_ != nullptr) {
            that->complete_(that->engine_, -1, res.data_);
        }
    }
}

void NativeAsyncWork::Send(void* data)
{
    std::lock_guard<std::mutex> lock(workAsyncMutex_);
    NativeAsyncWorkDataPointer dataPointer(data);
    asyncWorkRecvData_.push(dataPointer);
    uv_async_send(&workAsyncHandler_);
}

bool NativeAsyncWork::PopData(NativeAsyncWorkDataPointer* data)
{
    std::lock_guard<std::mutex> lock(workAsyncMutex_);
    if (asyncWorkRecvData_.empty()) {
        return false;
    }
    *data = asyncWorkRecvData_.front();
    asyncWorkRecvData_.pop();
    return true;
}

bool NativeAsyncWork::Init()
{
    uv_loop_t* loop = engine_->GetUVLoop();
    if (loop == nullptr) {
        HILOG_ERROR("Get loop failed");
        return false;
    }
    uv_async_init(loop, &workAsyncHandler_, reinterpret_cast<uv_async_cb>(AsyncWorkRecvCallback));
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

    napi_status nstatus = napi_generic_failure;

    switch (status) {
        case 0:
            nstatus = napi_ok;
            break;
        case (int)UV_EINVAL:
            nstatus = napi_invalid_arg;
            break;
        case (int)UV_ECANCELED:
            nstatus = napi_cancelled;
            break;
        default:
            nstatus = napi_generic_failure;
    }

    that->complete_(that->engine_, nstatus, that->data_);
    scopeManager->Close(scope);
}
