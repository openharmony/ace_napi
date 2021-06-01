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

#include "native_scope_manager.h"

#include "native_engine/native_value.h"

struct NativeHandle {
    NativeValue* value = nullptr;
    NativeHandle* sibling = nullptr;
};

struct NativeScope {
    NativeHandle* handlePtr = nullptr;
    size_t handleCount = 0;
    bool escaped = false;

    NativeScope* child = nullptr;
    NativeScope* parent = nullptr;
};

NativeScopeManager::NativeScopeManager()
{
    root_ = new NativeScope();
    current_ = root_;
}

NativeScopeManager::~NativeScopeManager()
{
    NativeScope* scope = root_;
    while (scope != nullptr) {
        NativeScope* tempScope = scope->child;
        NativeHandle* handle = scope->handlePtr;
        while (handle != nullptr) {
            NativeHandle* tempHandle = handle->sibling;
            delete handle->value;
            delete handle;
            handle = tempHandle;
        }
        delete scope;
        scope = tempScope;
    }
    root_ = nullptr;
    current_ = nullptr;
}

NativeScope* NativeScopeManager::Open()
{
    auto scope = new NativeScope();
    current_->child = scope;
    scope->parent = current_;
    current_ = scope;

    return scope;
}

void NativeScopeManager::Close(NativeScope* scope)
{
    if ((scope == nullptr) || (scope == root_)) {
        return;
    }
    if (scope == current_) {
        current_ = scope->parent;
    }

    scope->parent->child = scope->child;

    NativeHandle* handle = scope->handlePtr;
    while (handle != nullptr) {
        scope->handlePtr = handle->sibling;
        delete handle->value;
        delete handle;
        handle = scope->handlePtr;
    }
    delete scope;
}

NativeScope* NativeScopeManager::OpenEscape()
{
    NativeScope* scope = Open();
    if (scope != nullptr) {
        scope->escaped = true;
    }
    return scope;
}

void NativeScopeManager::CloseEscape(NativeScope* scope)
{
    if (scope == nullptr) {
        return;
    }
    Close(scope);
}

NativeValue* NativeScopeManager::Escape(NativeScope* scope, NativeValue* value)
{
    NativeValue* result = nullptr;

    if ((scope == nullptr) || (value == nullptr)) {
        return result;
    }

    NativeHandle* handle = scope->handlePtr;
    NativeHandle* temp = nullptr;
    while (handle != nullptr && scope->escaped) {
        if (handle->value == value) {
            if (temp == nullptr) {
                scope->handlePtr = handle->sibling;
            } else {
                temp->sibling = handle->sibling;
            }
            if (scope->parent->handlePtr == nullptr) {
                scope->parent->handlePtr = handle;
                handle->sibling = nullptr;
            } else {
                handle->sibling = scope->parent->handlePtr;
                scope->parent->handlePtr = handle;
            }
            scope->handleCount--;
            scope->parent->handleCount++;
            result = scope->parent->handlePtr->value;
            break;
        }
        temp = handle;
        handle = handle->sibling;
    }
    return result;
}

void NativeScopeManager::CreateHandle(NativeValue* value)
{
    if (current_->handlePtr == nullptr) {
        current_->handlePtr = new NativeHandle();
        current_->handlePtr->value = value;
        current_->handlePtr->sibling = nullptr;
    } else {
        auto temp = new NativeHandle();
        temp->sibling = current_->handlePtr;
        temp->value = value;
        current_->handlePtr = temp;
    }
    current_->handleCount++;
}
