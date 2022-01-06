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

#include "native_callback_scope_manager.h"
#include "native_engine/native_engine.h"

NativeCallbackScope::NativeCallbackScope(NativeEngine* env)
{
    env_ = env;
}

NativeCallbackScope::~NativeCallbackScope()
{
    Close();
}

void NativeCallbackScope::Close()
{
    if (!closed_) {
        closed_ = true;
        if (env_->IsStopping()) {
            MarkAsFailed();
        }
    }
}

NativeCallbackScopeManager::NativeCallbackScopeManager() {}

NativeCallbackScopeManager::~NativeCallbackScopeManager() {}

NativeCallbackScope* NativeCallbackScopeManager::Open(NativeEngine* env)
{
    NativeCallbackScope* scope = new (std::nothrow)NativeCallbackScope(env);

    if (scope) {
        asyncCallbackScopeDepth_++;
        return scope;
    } else {
        return nullptr;
    }
}

void NativeCallbackScopeManager::Close(NativeCallbackScope* scope)
{
    if (scope != nullptr) {
        delete scope;
    }
    asyncCallbackScopeDepth_--;
}

size_t NativeCallbackScopeManager::IncrementOpenCallbackScopes()
{
    openCallbackScopes_++;
    return openCallbackScopes_;
}

size_t NativeCallbackScopeManager::DecrementOpenCallbackScopes()
{
    openCallbackScopes_--;
    return openCallbackScopes_;
}
