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

#include "jerryscript_native_deferred.h"

JerryScriptNativeDeferred::JerryScriptNativeDeferred(jerry_value_t value)
    : value_(value)
{
    jerry_acquire_value(value_);
}

JerryScriptNativeDeferred::~JerryScriptNativeDeferred()
{
    jerry_release_value(value_);
}

void JerryScriptNativeDeferred::Resolve(NativeValue* data)
{
    jerry_value_t value = jerry_resolve_or_reject_promise(value_, *data, true);
    jerry_release_value(value);
}

void JerryScriptNativeDeferred::Reject(NativeValue* reason)
{
    jerry_value_t value = jerry_resolve_or_reject_promise(value_, *reason, false);
    jerry_release_value(value);
}
