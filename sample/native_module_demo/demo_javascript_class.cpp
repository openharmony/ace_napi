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

#include "demo_javascript_class.h"

static napi_value DemoJavascriptClassConstructor(napi_env env, napi_callback_info info);
static napi_value DemoJavascriptClassAdd(napi_env env, napi_callback_info info);
static napi_value DemoJavascriptClassSub(napi_env env, napi_callback_info info);
static napi_value DemoJavascriptClassMul(napi_env env, napi_callback_info info);
static napi_value DemoJavascriptClassDiv(napi_env env, napi_callback_info info);

enum TestEnum {
    ONE = 0,
    TWO,
    THREE,
    FOUR
};

/*
 * Class Init
 */
void DemoJavascriptClassInit(napi_env env, napi_value exports)
{
    napi_value one = nullptr;
    napi_value two = nullptr;
    napi_value three = nullptr;
    napi_value four = nullptr;

    napi_create_int32(env, TestEnum::ONE, &one);
    napi_create_int32(env, TestEnum::TWO, &two);
    napi_create_int32(env, TestEnum::THREE, &three);
    napi_create_int32(env, TestEnum::FOUR, &four);

    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("add", DemoJavascriptClassAdd),
        DECLARE_NAPI_FUNCTION("sub", DemoJavascriptClassSub),
        DECLARE_NAPI_FUNCTION("mul", DemoJavascriptClassMul),
        DECLARE_NAPI_FUNCTION("div", DemoJavascriptClassDiv),
        DECLARE_NAPI_STATIC_PROPERTY("ONE", one),
        DECLARE_NAPI_STATIC_PROPERTY("TWO", two),
        DECLARE_NAPI_STATIC_PROPERTY("THREE", three),
        DECLARE_NAPI_STATIC_PROPERTY("FOUR", four),
    };

    napi_value result = nullptr;
    napi_define_class(env, "DemoClass", NAPI_AUTO_LENGTH, DemoJavascriptClassConstructor, nullptr,
                      sizeof(descriptors) / sizeof(*descriptors), descriptors, &result);

    napi_set_named_property(env, exports, "DemoClass", result);
}

/*
 * Constructor
 */
static napi_value DemoJavascriptClassConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void* data = nullptr;

    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);

    napi_value global = nullptr;
    napi_get_global(env, &global);

    return thisArg;
}

/*
 * Add Method
 */
static napi_value DemoJavascriptClassAdd(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);

    napi_value result = nullptr;

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameters of number type");

    napi_valuetype valueType = napi_undefined;
    double param1 = 0;
    double param2 = 0;

    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 1");
        return nullptr;
    }

    napi_typeof(env, argv[1], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 2");
        return nullptr;
    }

    napi_get_value_double(env, argv[0], &param1);
    napi_get_value_double(env, argv[1], &param2);

    napi_create_double(env, param1 + param2, &result);

    return result;
}

/*
 * Sub Method
 */
static napi_value DemoJavascriptClassSub(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);

    napi_value result = nullptr;

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameters of number type");

    napi_valuetype valueType = napi_undefined;
    double param1 = 0;
    double param2 = 0;

    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 1");
        return nullptr;
    }

    napi_typeof(env, argv[1], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 2");
        return nullptr;
    }

    napi_get_value_double(env, argv[0], &param1);
    napi_get_value_double(env, argv[1], &param2);

    napi_create_double(env, param1 - param2, &result);

    return result;
}

/*
 * Mul Method
 */
static napi_value DemoJavascriptClassMul(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);

    napi_value result = nullptr;

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameters of number type");

    napi_valuetype valueType = napi_undefined;
    double param1 = 0;
    double param2 = 0;

    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 1");
        return nullptr;
    }

    napi_typeof(env, argv[1], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 2");
        return nullptr;
    }

    napi_get_value_double(env, argv[0], &param1);
    napi_get_value_double(env, argv[1], &param2);

    napi_create_double(env, param1 * param2, &result);

    return result;
}

/*
 * Div Method
 */
static napi_value DemoJavascriptClassDiv(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisArg = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);

    napi_value result = nullptr;

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameters of number type");

    napi_valuetype valueType = napi_undefined;
    double param1 = 0;
    double param2 = 0;

    napi_typeof(env, argv[0], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 1");
        return nullptr;
    }

    napi_typeof(env, argv[1], &valueType);
    if (valueType != napi_valuetype::napi_number) {
        napi_throw_type_error(env, nullptr, "type mismatch for parameter 2");
        return nullptr;
    }

    napi_get_value_double(env, argv[0], &param1);
    napi_get_value_double(env, argv[1], &param2);

    NAPI_ASSERT(env, param2 == 0, "parameter 2 cannot be zero");

    napi_create_double(env, param1 / param2, &result);

    return result;
}
