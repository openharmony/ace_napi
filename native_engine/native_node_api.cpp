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

#include "napi/native_node_api.h"
#include "native_api_internal.h"

#include "native_engine/native_engine.h"
#include "utils/log.h"

NAPI_EXTERN void napi_module_register(napi_module* mod)
{
    if (mod == nullptr) {
        HILOG_ERROR("mod is nullptr");
        return;
    }

    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    NativeModule module;

    module.version = mod->nm_version;
    module.fileName = mod->nm_filename;
    module.name = mod->nm_modname;
    module.registerCallback = (RegisterCallback)mod->nm_register_func;

    moduleManager->Register(&module);
}

NAPI_EXTERN NAPI_NO_RETURN void napi_fatal_error(const char* location,
                                                 size_t location_len,
                                                 const char* message,
                                                 size_t message_len)
{
    (void)location_len;
    (void)message_len;
    HILOG_FATAL("FATAL ERROR: %{public}s %{public}s\n", location, message);
    abort();
}

// Methods to manage simple async operations
NAPI_EXTERN napi_status napi_create_async_work(napi_env env,
                                               napi_value async_resource,
                                               napi_value async_resource_name,
                                               napi_async_execute_callback execute,
                                               napi_async_complete_callback complete,
                                               void* data,
                                               napi_async_work* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_resource_name);
    CHECK_ARG(env, execute);
    CHECK_ARG(env, complete);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto asyncResource = reinterpret_cast<NativeValue*>(async_resource);
    auto asyncResourceName = reinterpret_cast<NativeValue*>(async_resource_name);
    auto asyncExecute = reinterpret_cast<NativeAsyncExecuteCallback>(execute);
    auto asyncComplete = reinterpret_cast<NativeAsyncCompleteCallback>(complete);

    auto asyncWork = engine->CreateAsyncWork(asyncResource, asyncResourceName, asyncExecute, asyncComplete, data);

    *result = reinterpret_cast<napi_async_work>(asyncWork);
    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_delete_async_work(napi_env env, napi_async_work work)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);

    delete asyncWork;
    asyncWork = nullptr;

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_queue_async_work(napi_env env, napi_async_work work)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);

    asyncWork->Queue();
    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_cancel_async_work(napi_env env, napi_async_work work)
{
    CHECK_ENV(env);
    CHECK_ARG(env, work);

    auto asyncWork = reinterpret_cast<NativeAsyncWork*>(work);

    asyncWork->Cancel();
    return napi_status::napi_ok;
}

// Version management
NAPI_EXTERN napi_status napi_get_node_version(napi_env env, const napi_node_version** version)
{
    (void)version;
    return napi_status::napi_ok;
}

// Return the current libuv event loop for a given environment
NAPI_EXTERN napi_status napi_get_uv_event_loop(napi_env env, struct uv_loop_s** loop)
{
    CHECK_ENV(env);
    CHECK_ARG(env, loop);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    *loop = engine->GetUVLoop();

    return napi_status::napi_ok;
}
