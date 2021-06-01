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

#include <uv.h>

class NativeAsyncWork {
public:
    NativeAsyncWork(NativeEngine* engine,
                    NativeAsyncExecuteCallback execute,
                    NativeAsyncCompleteCallback complete,
                    void* data);

    virtual ~NativeAsyncWork();
    virtual bool Queue();
    virtual bool Cancel();

private:
    static void AsyncWorkCallback(uv_work_t* req);
    static void AsyncAfterWorkCallback(uv_work_t* req, int status);

    uv_work_t work_;

    NativeEngine* engine_;

    int status_;
    NativeAsyncExecuteCallback execute_;
    NativeAsyncCompleteCallback complete_;
    void* data_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ASYNC_WORK_H */
