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

static constexpr int32_t MAX_THEAD_SAFE_COUNT = 128;

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

NAPI_EXTERN napi_status napi_fatal_exception(napi_env env, napi_value err)
{
    HILOG_INFO("%{public}s, start.", __func__);
    CHECK_ENV(env);
    CHECK_ARG(env, err);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto jsError = reinterpret_cast<NativeValue*>(err);
    if (engine->TriggerFatalException(jsError)) {
        HILOG_INFO("%{public}s, end.", __func__);
        return napi_status::napi_ok;
    } else {
        HILOG_INFO("%{public}s, end.", __func__);
        exit(1);
        return napi_status::napi_ok;
    }
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

NAPI_EXTERN napi_status napi_add_env_cleanup_hook(napi_env env, void (*fun)(void* arg), void* arg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, fun);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->AddCleanupHook(fun, arg);

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_remove_env_cleanup_hook(napi_env env, void (*fun)(void* arg), void* arg)
{
    CHECK_ENV(env);
    CHECK_ARG(env, fun);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->RemoveCleanupHook(fun, arg);

    return napi_clear_last_error(env);
}

using CleanupHook = void (*)(void* arg);
using AsyncCleanupHook = void (*)(void* arg, void (*)(void*), void*);

struct AsyncCleanupHookInfo final {
    napi_env env;
    AsyncCleanupHook fun;
    void* arg;
    bool started = false;
    // Use a self-reference to make sure the storage is kept alive while the
    // cleanup hook is registered but not yet finished.
    std::shared_ptr<AsyncCleanupHookInfo> self;
};

// Opaque type that is basically an alias for `shared_ptr<AsyncCleanupHookInfo>`
// (but not publicly so for easier ABI/API changes). In particular,
// std::shared_ptr does not generally maintain a consistent ABI even on a
// specific platform.
struct ACHHandle final {
    std::shared_ptr<AsyncCleanupHookInfo> info;
};

struct DeleteACHHandle {
    void operator()(ACHHandle* handle) const
    {
        delete handle;
    };
};
using AsyncCleanupHookHandle = std::unique_ptr<ACHHandle, DeleteACHHandle>;

static void FinishAsyncCleanupHook(void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    AsyncCleanupHookInfo* info = static_cast<AsyncCleanupHookInfo*>(arg);
    std::shared_ptr<AsyncCleanupHookInfo> keep_alive = info->self;
    auto engine = reinterpret_cast<NativeEngine*>(info->env);
    engine->DecreaseWaitingRequestCounter();
    info->self.reset();
    HILOG_INFO("%{public}s, end.", __func__);
}

static void RunAsyncCleanupHook(void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    AsyncCleanupHookInfo* info = static_cast<AsyncCleanupHookInfo*>(arg);
    auto engine = reinterpret_cast<NativeEngine*>(info->env);
    engine->IncreaseWaitingRequestCounter();
    info->started = true;
    info->fun(info->arg, FinishAsyncCleanupHook, info);
    HILOG_INFO("%{public}s, end.", __func__);
}

static AsyncCleanupHookHandle AddEnvironmentCleanupHook(napi_env env, AsyncCleanupHook fun, void* arg)
{
    HILOG_INFO("%{public}s, start.", __func__);
    auto info = std::make_shared<AsyncCleanupHookInfo>();
    info->env = env;
    info->fun = fun;
    info->arg = arg;
    info->self = info;
    auto engine = reinterpret_cast<NativeEngine*>(env);
    engine->AddCleanupHook(RunAsyncCleanupHook, info.get());
    HILOG_INFO("%{public}s, end.", __func__);
    return AsyncCleanupHookHandle(new ACHHandle { info });
}

static void RemoveEnvironmentCleanupHook(AsyncCleanupHookHandle handle)
{
    HILOG_INFO("%{public}s, start.", __func__);
    if (handle->info->started)
        return;
    handle->info->self.reset();
    auto engine = reinterpret_cast<NativeEngine*>(handle->info->env);
    engine->RemoveCleanupHook(RunAsyncCleanupHook, handle->info.get());
    HILOG_INFO("%{public}s, end.", __func__);
}

struct napi_async_cleanup_hook_handle__ {
    napi_async_cleanup_hook_handle__(napi_env env, napi_async_cleanup_hook user_hook, void* user_data)
        : env_(env), user_hook_(user_hook), user_data_(user_data)
    {
        HILOG_INFO("%{public}s, start.", __func__);
        handle_ = AddEnvironmentCleanupHook(env, Hook, this);
        HILOG_INFO("%{public}s, end.", __func__);
    }

    ~napi_async_cleanup_hook_handle__()
    {
        HILOG_INFO("%{public}s, start.", __func__);
        RemoveEnvironmentCleanupHook(std::move(handle_));
        if (done_cb_ != nullptr) {
            done_cb_(done_data_);
        }
        HILOG_INFO("%{public}s, end.", __func__);
    }

    static void Hook(void* data, void (*done_cb)(void*), void* done_data)
    {
        HILOG_INFO("%{public}s, start.", __func__);
        auto handle = static_cast<napi_async_cleanup_hook_handle__*>(data);
        handle->done_cb_ = done_cb;
        handle->done_data_ = done_data;
        handle->user_hook_(handle, handle->user_data_);
        HILOG_INFO("%{public}s, end.", __func__);
    }

    AsyncCleanupHookHandle handle_;
    napi_env env_ = nullptr;
    napi_async_cleanup_hook user_hook_ = nullptr;
    void* user_data_ = nullptr;
    void (*done_cb_)(void*) = nullptr;
    void* done_data_ = nullptr;
};

NAPI_EXTERN napi_status napi_add_async_cleanup_hook(
    napi_env env, napi_async_cleanup_hook hook, void* arg, napi_async_cleanup_hook_handle* remove_handle)
{
    CHECK_ENV(env);
    CHECK_ARG(env, hook);

    napi_async_cleanup_hook_handle__* handle = new napi_async_cleanup_hook_handle__(env, hook, arg);

    if (remove_handle != nullptr)
        *remove_handle = handle;

    return napi_clear_last_error(env);
}

NAPI_EXTERN napi_status napi_remove_async_cleanup_hook(napi_async_cleanup_hook_handle remove_handle)
{
    if (remove_handle == nullptr) {
        return napi_invalid_arg;
    }

    delete remove_handle;
    return napi_ok;
}

// Methods to manager threadsafe
NAPI_EXTERN napi_status napi_create_threadsafe_function(napi_env env, napi_value func, napi_value async_resource,
    napi_value async_resource_name, size_t max_queue_size, size_t initial_thread_count, void* thread_finalize_data,
    napi_finalize thread_finalize_cb, void* context, napi_threadsafe_function_call_js call_js_cb,
    napi_threadsafe_function* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_resource_name);
    RETURN_STATUS_IF_FALSE(env, max_queue_size >= 0, napi_invalid_arg);
    RETURN_STATUS_IF_FALSE(
        env, initial_thread_count > 0 && initial_thread_count <= MAX_THEAD_SAFE_COUNT, napi_invalid_arg);
    CHECK_ARG(env, result);
    if (func == nullptr) {
        CHECK_ARG(env, call_js_cb);
    }

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto jsFunc = reinterpret_cast<NativeValue*>(func);
    auto asyncResource = reinterpret_cast<NativeValue*>(async_resource);
    auto asyncResourceName = reinterpret_cast<NativeValue*>(async_resource_name);
    auto finalizeCallback = reinterpret_cast<NativeFinalize>(thread_finalize_cb);
    auto callJsCallback = reinterpret_cast<NativeThreadSafeFunctionCallJs>(call_js_cb);
    auto safeAsyncWork = engine->CreateSafeAsyncWork(jsFunc, asyncResource, asyncResourceName, max_queue_size,
        initial_thread_count, thread_finalize_data, finalizeCallback, context, callJsCallback);
    CHECK_ENV(safeAsyncWork);

    auto ret = safeAsyncWork->Init();
    if (ret) {
        *result = reinterpret_cast<napi_threadsafe_function>(safeAsyncWork);
    } else {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_call_threadsafe_function(
    napi_threadsafe_function func, void* data, napi_threadsafe_function_call_mode is_blocking)
{
    CHECK_ENV(func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto callMode = static_cast<NativeThreadSafeFunctionCallMode>(is_blocking);

    napi_status status = napi_status::napi_ok;
    auto code = safeAsyncWork->Send(data, callMode);
    switch (code) {
        case SafeAsyncCode::SAFE_ASYNC_QUEUE_FULL:
            status = napi_status::napi_queue_full;
            break;
        case SafeAsyncCode::SAFE_ASYNC_INVALID_ARGS:
            status = napi_status::napi_invalid_arg;
            break;
        case SafeAsyncCode::SAFE_ASYNC_CLOSED:
            status = napi_status::napi_closing;
            break;
        case SafeAsyncCode::SAFE_ASYNC_FAILED:
            status = napi_status::napi_generic_failure;
            break;
        default:
            status = napi_status::napi_ok;
            break;
    }

    return status;
}

NAPI_EXTERN napi_status napi_acquire_threadsafe_function(napi_threadsafe_function func)
{
    CHECK_ENV(func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);

    auto ret = safeAsyncWork->Acquire();
    if (ret != SafeAsyncCode::SAFE_ASYNC_OK) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_release_threadsafe_function(
    napi_threadsafe_function func, napi_threadsafe_function_release_mode mode)
{
    CHECK_ENV(func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto releaseMode = static_cast<NativeThreadSafeFunctionReleaseMode>(mode);

    auto ret = safeAsyncWork->Release(releaseMode);
    if (ret != SafeAsyncCode::SAFE_ASYNC_OK) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_get_threadsafe_function_context(napi_threadsafe_function func, void** result)
{
    CHECK_ENV(func);
    CHECK_ENV(result);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    *result = safeAsyncWork->GetContext();

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_ref_threadsafe_function(napi_env env, napi_threadsafe_function func)
{
    CHECK_ENV(env);
    CHECK_ARG(env, func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto ret = safeAsyncWork->Ref();
    if (!ret) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

NAPI_EXTERN napi_status napi_unref_threadsafe_function(napi_env env, napi_threadsafe_function func)
{
    CHECK_ENV(env);
    CHECK_ARG(env, func);

    auto safeAsyncWork = reinterpret_cast<NativeSafeAsyncWork*>(func);
    auto ret = safeAsyncWork->Unref();
    if (!ret) {
        return napi_status::napi_generic_failure;
    }

    return napi_status::napi_ok;
}

napi_status napi_async_init(
    napi_env env, napi_value async_resource, napi_value async_resource_name, napi_async_context* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_resource_name);
    CHECK_ARG(env, result);

    auto asyncResource = reinterpret_cast<NativeValue*>(async_resource);
    auto asyncResourceName = reinterpret_cast<NativeValue*>(async_resource_name);

    auto async_context = new NativeAsyncContext();
    async_context->asyncResource = asyncResource;
    async_context->asyncResourceName = asyncResourceName;

    *result = reinterpret_cast<napi_async_context>(async_context);

    return napi_clear_last_error(env);
}

napi_status napi_async_destroy(napi_env env, napi_async_context async_context)
{
    CHECK_ENV(env);
    CHECK_ARG(env, async_context);

    NativeAsyncContext* native_async_context = reinterpret_cast<NativeAsyncContext*>(async_context);

    delete native_async_context;

    return napi_clear_last_error(env);
}

napi_status napi_open_callback_scope(
    napi_env env, napi_value, napi_async_context async_context_handle, napi_callback_scope* result)
{
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callbackScopeManager = engine->GetCallbackScopeManager();
    CHECK_ARG(env, callbackScopeManager);

    auto callbackScope = callbackScopeManager->Open(engine);
    callbackScopeManager->IncrementOpenCallbackScopes();

    *result = reinterpret_cast<napi_callback_scope>(callbackScope);

    return napi_clear_last_error(env);
}

napi_status napi_close_callback_scope(napi_env env, napi_callback_scope scope)
{
    CHECK_ENV(env);
    CHECK_ARG(env, scope);

    auto engine = reinterpret_cast<NativeEngine*>(env);
    auto callbackScopeManager = engine->GetCallbackScopeManager();
    CHECK_ARG(env, callbackScopeManager);
    size_t count = callbackScopeManager->GetOpenCallbackScopes();
    if (count == 0) {
        return napi_callback_scope_mismatch;
    }
    callbackScopeManager->DecrementOpenCallbackScopes();

    auto callbackScope = reinterpret_cast<NativeCallbackScope*>(scope);
    callbackScopeManager->Close(callbackScope);

    return napi_clear_last_error(env);
}
