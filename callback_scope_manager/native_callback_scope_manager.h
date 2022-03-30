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

#ifndef FOUNDATION_ACE_NAPI_CALLBACK_SCOPE_MANAGER_NATIVE_CALLBACK_SCOPE_MANAGER_H
#define FOUNDATION_ACE_NAPI_CALLBACK_SCOPE_MANAGER_NATIVE_CALLBACK_SCOPE_MANAGER_H

#include <cstddef>

class NativeEngine;

class NativeCallbackScope {
public:
    explicit NativeCallbackScope(NativeEngine* env);
    virtual ~NativeCallbackScope();
    void Close();

    inline bool Failed() const
    {
        return failed_;
    }
    inline void MarkAsFailed()
    {
        failed_ = true;
    }

private:
    bool closed_ = false;
    bool failed_ = false;
    NativeEngine* env_ = nullptr;
};

class NativeCallbackScopeManager {
public:
    NativeCallbackScopeManager();
    virtual ~NativeCallbackScopeManager();

    NativeCallbackScope* Open(NativeEngine* env);
    void Close(NativeCallbackScope* scope);

    size_t IncrementOpenCallbackScopes();
    size_t DecrementOpenCallbackScopes();

    size_t GetOpenCallbackScopes() const
    {
        return openCallbackScopes_;
    }

    size_t GetAsyncCallbackScopeDepth() const
    {
        return asyncCallbackScopeDepth_;
    }

private:
    size_t openCallbackScopes_ = 0;
    size_t asyncCallbackScopeDepth_ = 0;
};

#endif /* FOUNDATION_ACE_NAPI_CALLBACK_SCOPE_MANAGER_NATIVE_CALLBACK_SCOPE_MANAGER_H */
