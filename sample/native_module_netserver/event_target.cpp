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

#include "event_target.h"

#include "securec.h"

#include "utils/log.h"

#define LISTENER_TYPTE_MAX_LENGTH 64

struct EventListener {
    char type[LISTENER_TYPTE_MAX_LENGTH] = { 0 };
    bool isOnce = false;
    napi_ref handlerRef = nullptr;
    EventListener* back = nullptr;
    EventListener* next = nullptr;
};

EventTarget::EventTarget(napi_env env, napi_value thisVar)
{
    env_ = env;
    first_ = last_ = nullptr;
    thisVarRef_ = nullptr;
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
}

EventTarget::~EventTarget()
{
    EventListener* temp = nullptr;
    for (EventListener* i = first_; i != nullptr; i = temp) {
        temp = i->next;
        if (i == first_) {
            first_ = first_->next;
        } else if (i == last_) {
            last_ = last_->back;
        } else {
            i->next->back = i->back;
            i->back->next = i->next;
        }
        napi_delete_reference(env_, i->handlerRef);
        delete i;
    }
    napi_delete_reference(env_, thisVarRef_);
}

void EventTarget::On(const char* type, napi_value handler)
{
    auto tmp = new EventListener();

    if (strncpy_s(tmp->type, LISTENER_TYPTE_MAX_LENGTH, type, strlen(type)) == -1) {
        delete tmp;
        tmp = nullptr;
        return;
    }

    if (first_ == nullptr) {
        first_ = last_ = tmp;
    } else {
        last_->next = tmp;
        last_->next->back = last_;
        last_ = last_->next;
    }
    last_->isOnce = false;
    napi_create_reference(env_, handler, 1, &last_->handlerRef);
}

void EventTarget::Once(const char* type, napi_value handler)
{
    auto tmp = new EventListener();

    if (strncpy_s(tmp->type, LISTENER_TYPTE_MAX_LENGTH, type, strlen(type)) == -1) {
        delete tmp;
        tmp = nullptr;
        return;
    }

    if (first_ == nullptr) {
        first_ = last_ = tmp;
    } else {
        last_->next = tmp;
        last_->next->back = last_;
        last_ = last_->next;
    }
    last_->isOnce = true;
    napi_create_reference(env_, handler, 1, &last_->handlerRef);
}

void EventTarget::Off(const char* type, napi_value handler)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        HILOG_ERROR("scope is nullptr");
        return;
    }

    EventListener* temp = nullptr;
    for (EventListener* eventListener = first_; eventListener != nullptr; eventListener = temp) {
        temp = eventListener->next;
        bool isEquals = false;
        napi_value handlerTemp = nullptr;
        napi_get_reference_value(env_, eventListener->handlerRef, &handlerTemp);
        napi_strict_equals(env_, handlerTemp, handler, &isEquals);
        if (strcmp(eventListener->type, type) == 0 && isEquals) {
            if (eventListener == first_) {
                first_ = first_->next;
            } else if (eventListener == last_) {
                last_ = last_->back;
            } else {
                eventListener->next->back = eventListener->back;
                eventListener->back->next = eventListener->next;
            }
            napi_delete_reference(env_, eventListener->handlerRef);
            delete eventListener;
            eventListener = nullptr;
            break;
        }
    }
    napi_close_handle_scope(env_, scope);
}

void EventTarget::Off(const char* type)
{
    EventListener* temp = nullptr;
    for (EventListener* eventListener = first_; eventListener != nullptr; eventListener = temp) {
        temp = eventListener->next;
        if (strcmp(eventListener->type, type) == 0) {
            if (eventListener == first_) {
                first_ = first_->next;
            } else if (eventListener == last_) {
                last_ = last_->back;
            } else {
                eventListener->next->back = eventListener->back;
                eventListener->back->next = eventListener->next;
            }
            napi_delete_reference(env_, eventListener->handlerRef);
            delete eventListener;
            eventListener = nullptr;
        }
    }
}

void EventTarget::Emit(const char* type, Event* event)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);

    napi_value thisVar = nullptr;
    napi_get_reference_value(env_, thisVarRef_, &thisVar);
    for (EventListener* eventListener = first_; eventListener != nullptr; eventListener = eventListener->next) {
        if (strcmp(eventListener->type, type) == 0) {
            napi_value jsEvent = event ? event->ToJsObject() : nullptr;
            napi_value handler = nullptr;
            napi_get_reference_value(env_, eventListener->handlerRef, &handler);
            napi_call_function(env_, thisVar, handler, jsEvent ? 1 : 0, jsEvent ? &jsEvent : nullptr, nullptr);
            if (eventListener->isOnce) {
                Off(type, handler);
            }
        }
    }

    napi_close_handle_scope(env_, scope);
}