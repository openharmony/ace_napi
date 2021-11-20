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

#include "foundation/ace/napi/native_engine/native_value.h"
#include "native_common.h"
#include "node_api.h"

DEPRECATED napi_status napi_make_callback(napi_env env, napi_async_context async_context, napi_value recv,
    napi_value func, size_t argc, const napi_value* argv, napi_value* result);

napi_status napi_create_string_utf16(napi_env env, const char16_t* str, size_t length, napi_value* result);

napi_status napi_get_value_string_utf16(napi_env env, napi_value value, char16_t* buf, size_t bufsize, size_t* result);

napi_status napi_create_bigint_int64(napi_env env, int64_t value, napi_value* result);

napi_status napi_create_bigint_uint64(napi_env env, uint64_t value, napi_value* result);

napi_status napi_get_value_bigint_int64(napi_env env, napi_value value, int64_t* result, bool* lossless);

napi_status napi_get_value_bigint_uint64(napi_env env, napi_value value, uint64_t* result, bool* lossless);

napi_status napi_detach_arraybuffer(napi_env env, napi_value arraybuffer);

napi_status napi_is_detached_arraybuffer(napi_env env, napi_value arraybuffer, bool* result);

napi_status napi_is_date(napi_env env, napi_value value, bool* result);

napi_status napi_object_freeze(napi_env env, napi_value object);

napi_status napi_object_seal(napi_env env, napi_value object);

napi_status napi_create_date(napi_env env, double time, napi_value* result);

napi_status napi_get_date_value(napi_env env, napi_value value, double* result);

napi_status napi_create_bigint_words(
    napi_env env, int sign_bit, size_t word_count, const uint64_t* words, napi_value* result);

napi_status napi_get_value_bigint_words(
    napi_env env, napi_value value, int* sign_bit, size_t* word_count, uint64_t* words);

napi_status napi_add_finalizer(napi_env env, napi_value js_object, void* native_object, napi_finalize finalize_cb,
    void* finalize_hint, napi_ref* result);

napi_status napi_set_instance_data(napi_env env, void* data, napi_finalize finalize_cb, void* finalize_hint);

napi_status napi_get_instance_data(napi_env env, void** data);

NAPI_EXTERN napi_status napi_add_env_cleanup_hook(napi_env env, void (*fun)(void* arg), void* arg);

NAPI_EXTERN napi_status napi_remove_env_cleanup_hook(napi_env env, void (*fun)(void* arg), void* arg);

NAPI_EXTERN napi_status napi_add_async_cleanup_hook(
    napi_env env, napi_async_cleanup_hook hook, void* arg, napi_async_cleanup_hook_handle* remove_handle);

NAPI_EXTERN napi_status napi_remove_async_cleanup_hook(napi_async_cleanup_hook_handle remove_handle);

NAPI_EXTERN napi_status napi_adjust_external_memory(napi_env env, int64_t change_in_bytes, int64_t* result);
#endif /* FOUNDATION_ACE_NAPI_INTERFACES_KITS_NAPI_NATIVE_NODE_API_H */
