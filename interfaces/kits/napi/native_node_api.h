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

#ifndef FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_API_H
#define FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_API_H

#include "native_common.h"

#include <node_api.h>

DEPRECATED napi_status napi_async_init(napi_env env,
                                       napi_value async_resource,
                                       napi_value async_resource_name,
                                       napi_async_context* result);
DEPRECATED napi_status napi_async_destroy(napi_env env, napi_async_context async_context);
DEPRECATED napi_status napi_make_callback(napi_env env,
                                          napi_async_context async_context,
                                          napi_value recv,
                                          napi_value func,
                                          size_t argc,
                                          const napi_value* argv,
                                          napi_value* result);
DEPRECATED napi_status napi_create_buffer(napi_env env, size_t length, void** data, napi_value* result);
DEPRECATED napi_status napi_create_external_buffer(napi_env env,
                                                   size_t length,
                                                   void* data,
                                                   napi_finalize finalize_cb,
                                                   void* finalize_hint,
                                                   napi_value* result);
DEPRECATED napi_status napi_create_buffer_copy(napi_env env,
                                               size_t length,
                                               const void* data,
                                               void** result_data,
                                               napi_value* result);
DEPRECATED napi_status napi_is_buffer(napi_env env, napi_value value, bool* result);
DEPRECATED napi_status napi_get_buffer_info(napi_env env, napi_value value, void** data, size_t* length);

#endif /* FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_API_H */
