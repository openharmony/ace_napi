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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_WORK_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_WORK_H

#include "native_value.h"

#include <mutex>
#include <queue>
#include <uv.h>

struct NativeAsyncWorkDataPointer {
    NativeAsyncWorkDataPointer()
    {
        data_ = nullptr;
    }

    explicit NativeAsyncWorkDataPointer(void* data)
    {
        data_ = data;
    }
    void* data_ { nullptr };
};

class NativeAsyncWork {
public:
    NativeAsyncWork(NativeEngine* engine,
                    NativeAsyncExecuteCallback execute,
                    NativeAsyncCompleteCallback complete,
                    void* data);

    virtual ~NativeAsyncWork();
    virtual bool Queue();
    virtual bool Cancel();
    virtual bool Init();
    virtual void Send(void* data);
    virtual void Close();
    virtual bool PopData(NativeAsyncWorkDataPointer* data);

    template<typename Inner, typename Outer>
    static Outer* DereferenceOf(const Inner Outer::*field, const Inner* pointer)
    {
        if (field != nullptr && pointer != nullptr) {
            auto fieldOffset = reinterpret_cast<uintptr_t>(&(static_cast<Outer*>(0)->*field));
            auto outPointer = reinterpret_cast<Outer*>(reinterpret_cast<uintptr_t>(pointer) - fieldOffset);
            return outPointer;
        }
        return nullptr;
    }

private:
    static void AsyncWorkCallback(uv_work_t* req);
    static void AsyncAfterWorkCallback(uv_work_t* req, int status);
    static void AsyncWorkRecvCallback(const uv_async_t* req);

    uv_work_t work_;
    uv_async_t workAsyncHandler_;
    NativeEngine* engine_;
    NativeAsyncExecuteCallback execute_;
    NativeAsyncCompleteCallback complete_;
    void* data_;
    std::mutex workAsyncMutex_;
    std::queue<NativeAsyncWorkDataPointer> asyncWorkRecvData_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_WORK_H */
