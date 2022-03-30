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

#ifndef FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_NETSERVER_EVENT_TARGET_H
#define FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_NETSERVER_EVENT_TARGET_H

#include "napi/native_api.h"

struct EventListener;

class Event {
public:
    virtual napi_value ToJsObject() = 0;
};

class EventTarget {
public:
    EventTarget(napi_env env, napi_value thisVar);
    virtual ~EventTarget();

    virtual void On(const char* type, napi_value handler);
    virtual void Once(const char* type, napi_value handler);
    virtual void Off(const char* type, napi_value handler);
    virtual void Off(const char* type);
    virtual void Emit(const char* type, Event* event);

protected:
    napi_env env_;
    napi_ref thisVarRef_;
    EventListener* first_;
    EventListener* last_;
};

#endif /* FOUNDATION_ACE_NAPI_TEST_NATIVE_MODULE_NETSERVER_EVENT_TARGET_H */
