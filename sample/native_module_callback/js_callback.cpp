/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <thread>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "uv.h"

struct CallbackContext {
    napi_env env = nullptr;
    napi_ref callbackRef = nullptr;
    int retData = 0;
};

void callbackTest(CallbackContext* context)
{
    uv_loop_s* loop = nullptr;
    napi_get_uv_event_loop(context->env, &loop);
    
    uv_work_t* work = new uv_work_t;
    context->retData = 1;
    work->data = (void*)context;
    
    uv_queue_work(
        loop, work, [](uv_work_t* work) {},
        // using callback function back to JS thread
        [](uv_work_t* work, int status) {
            CallbackContext* context = (CallbackContext*)work->data;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(context->env, &scope);
            if (scope == nullptr) {
                return;
            }

            napi_value callback = nullptr;
            napi_get_reference_value(context->env, context->callbackRef, &callback);
            napi_value retArg;
            napi_create_int32(context->env, context->retData, &retArg);
            napi_value ret;
            napi_call_function(context->env, nullptr, callback, 1, &retArg, &ret);
            napi_delete_reference(context->env, context->callbackRef);

            napi_close_handle_scope(context->env, scope);

            if (work != nullptr) {
                delete work;
            }

            delete context;
        }
    );
}

static napi_value JSTest(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_function) {
        return nullptr;
    }
    auto asyncContext = new CallbackContext();
    asyncContext->env = env;
    napi_create_reference(env, argv[0], 1, &asyncContext->callbackRef);
    // using callback function on other thread
    std::thread testThread(callbackTest, asyncContext);
    testThread.detach();

    return nullptr;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value CallbackExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("test", JSTest)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

// callback module define
static napi_module callbackModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = CallbackExport,
    .nm_modname = "callback",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

// callback module register
extern "C" __attribute__((constructor)) void CallbackTestRegister()
{
    napi_module_register(&callbackModule);
}