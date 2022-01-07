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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

extern const char _binary_calc_js_start[];
extern const char _binary_calc_js_end[];

/*
 * Sync callback
 */
static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value args[2] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));

    NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

    napi_valuetype valuetype0;
    NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

    napi_valuetype valuetype1;
    NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));

    NAPI_ASSERT(env, valuetype0 == napi_number && valuetype1 == napi_number, "Wrong argument type. Numbers expected.");

    double value0;
    NAPI_CALL(env, napi_get_value_double(env, args[0], &value0));

    double value1;
    NAPI_CALL(env, napi_get_value_double(env, args[1], &value1));

    napi_value sum;
    NAPI_CALL(env, napi_create_double(env, value0 + value1, &sum));

    return sum;
}

/*
 * function for module exports
 */
static napi_value Init(napi_env env, napi_value exports)
{
    /*
     * Properties define
     */
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("add", Add),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

extern "C" __attribute__((visibility("default"))) void NAPI_calc_GetJSCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_calc_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_calc_js_end - _binary_calc_js_start;
    }
}

/*
 * Module define
 */
static napi_module calcModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "calc",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};
/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void CalcRegisterModule(void)
{
    napi_module_register(&calcModule);
}