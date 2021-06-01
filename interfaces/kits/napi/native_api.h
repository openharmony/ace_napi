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

#ifndef FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_API_H
#define FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_API_H

#include "native_common.h"

#include <js_native_api.h>

DEPRECATED napi_status napi_create_string_utf16(napi_env env, const char16_t* str, size_t length, napi_value* result);
DEPRECATED napi_status napi_get_value_string_utf16(napi_env env,
                                                   napi_value value,
                                                   char16_t* buf,
                                                   size_t bufsize,
                                                   size_t* result);
DEPRECATED napi_status napi_adjust_external_memory(napi_env env, int64_t change_in_bytes, int64_t* adjusted_value);

#endif /* FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_API_H */
