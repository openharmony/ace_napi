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

#include <string>

#include "ability.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#define GET_PARAMS(env, info, num) \
    size_t argc = num;             \
    napi_value argv[num];          \
    napi_value thisVar = nullptr;  \
    void* data = nullptr;          \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

/*
 * Get bundle path
 */
static napi_value JSAbilityGetBundlePath(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 0);
    // get global value
    napi_value global = nullptr;
    napi_get_global(env, &global);

    // get ability
    napi_value abilityObj = nullptr;
    napi_get_named_property(env, global, "ability", &abilityObj);

    // get ability pointer
    OHOS::AppExecFwk::Ability* ability = nullptr;
    napi_get_value_external(env, abilityObj, (void**)&ability);

    // get bundle path
    std::string path = ability->GetBundleCodePath();
    napi_value bundlePath = nullptr;
    napi_create_string_utf8(env, path.c_str(), path.length(), &bundlePath);
    return bundlePath;
}

static napi_value AbilityExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getBundlePath", JSAbilityGetBundlePath),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

// ability module
static napi_module abilityModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = AbilityExport,
    .nm_modname = "ability",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

// ability module register
extern "C" __attribute__((constructor)) void AbilityRegister()
{
    napi_module_register(&abilityModule);
}