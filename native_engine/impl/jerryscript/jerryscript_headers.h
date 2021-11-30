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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_JERRYSCRIPT_HEADERS_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_JERRYSCRIPT_HEADERS_H

#include "jerryscript-port-default.h"
#include "jerryscript.h"

using jerry_finalizer_t = void (*)(void* data, void* hint);

bool jerry_add_external();
jerry_value_t jerry_create_external(void* value, jerry_finalizer_t finalizer, void* hint);
bool jerry_value_is_external(const jerry_value_t object);
void* jerry_value_get_external(const jerry_value_t object);

void jerry_freeze(jerry_value_t value);
void jerry_seal(jerry_value_t value);
jerry_value_t jerry_strict_date(double time);
double jerry_get_date(jerry_value_t value);
bool jerry_is_date(jerry_value_t value);

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_JERRYSCRIPT_HEADERS_H */