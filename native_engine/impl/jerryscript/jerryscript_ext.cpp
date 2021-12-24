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

#include "jerryscript-ext/handler.h"
#include "jerryscript_headers.h"
#include "native_engine/native_value.h"
#include "utils/log.h"

struct jerry_external_context {
    void* value = nullptr;
    jerry_finalizer_t callback = nullptr;
    void* hint = nullptr;
};

namespace {
static jerry_object_native_info_t g_objectNativeInfo = { .free_cb = [](void* nativePointer) {
    auto externalCtx = (jerry_external_context*)nativePointer;
    if (externalCtx != nullptr) {
        externalCtx->callback(externalCtx->value, externalCtx->hint);
        delete externalCtx;
    }
} };
} // namespace

bool jerry_add_external()
{
    auto callback = [](const jerry_value_t function, const jerry_value_t thisVal, const jerry_value_t args[],
                        const jerry_length_t argc) -> jerry_value_t { return jerry_create_boolean(true); };

    auto constructor = jerry_create_external_function(callback);
    auto prototype = jerry_create_object();

    jerry_value_t propName = jerry_create_string_from_utf8((const jerry_char_t*)"prototype");
    jerry_value_t resultVal = jerry_set_property(constructor, propName, prototype);
    jerry_release_value(propName);

    if (!jerry_value_to_boolean(resultVal)) {
        HILOG_ERROR("jerry_add_external failed");
        return false;
    }

    jerry_value_t global = jerry_get_global_object();
    jerryx_set_property_str(global, "External", constructor);
    jerry_release_value(global);
    return true;
}

jerry_value_t jerry_create_external(void* value, jerry_finalizer_t finalizer, void* hint)
{
    auto externalCtx = new jerry_external_context {
        .value = value,
        .callback = finalizer,
        .hint = hint,
    };
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t external = jerryx_get_property_str(global, "External");
    jerry_value_t object = jerry_create_undefined();

    if (!jerry_value_is_undefined(external)) {
        object = jerry_construct_object(external, nullptr, 0);
        jerry_set_object_native_pointer(object, (void*)externalCtx, &g_objectNativeInfo);
    }

    jerry_release_value(external);
    jerry_release_value(global);

    return object;
}

bool jerry_value_is_external(const jerry_value_t object)
{
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t external = jerryx_get_property_str(global, "External");
    jerry_value_t op = jerry_create_boolean(false);

    if (!jerry_value_is_undefined(external)) {
        op = jerry_binary_operation(JERRY_BIN_OP_INSTANCEOF, object, external);
    }

    jerry_release_value(external);
    jerry_release_value(global);
    return jerry_get_boolean_value(op);
}

void* jerry_value_get_external(const jerry_value_t object)
{
    jerry_external_context* externalCtx = nullptr;

    jerry_get_object_native_pointer(object, (void**)&externalCtx, &g_objectNativeInfo);
    if (externalCtx != nullptr) {
        return externalCtx->value;
    } else {
        return nullptr;
    }
}

void jerry_freeze(jerry_value_t value)
{
    const jerry_char_t funcResource[] = "unknown";
    const jerry_char_t funcArgList[] = "v1 ";
    const jerry_char_t funcSrc[] = " Object.freeze( v1 ) ";

    jerry_value_t funcVal = jerry_parse_function(funcResource, sizeof(funcResource) - 1, funcArgList,
        sizeof(funcArgList) - 1, funcSrc, sizeof(funcSrc) - 1, JERRY_PARSE_NO_OPTS);

    jerry_value_t funcArgs[1] = { value };

    jerry_value_t valRet = jerry_call_function(funcVal, funcArgs[0], funcArgs, 1);

    jerry_release_value(valRet);
    jerry_release_value(funcVal);
}

void jerry_seal(jerry_value_t value)
{
    const jerry_char_t funcResource[] = "unknown";
    const jerry_char_t funcArgList[] = "v1 ";
    const jerry_char_t funcSrc[] = " Object.seal( v1 ) ";

    jerry_value_t funcVal = jerry_parse_function(funcResource, sizeof(funcResource) - 1, funcArgList,
        sizeof(funcArgList) - 1, funcSrc, sizeof(funcSrc) - 1, JERRY_PARSE_NO_OPTS);

    jerry_value_t funcArgs[1] = { value };

    jerry_value_t valRet = jerry_call_function(funcVal, funcArgs[0], funcArgs, 1);

    jerry_release_value(valRet);
    jerry_release_value(funcVal);
}

jerry_value_t jerry_strict_date(double time)
{
    const jerry_char_t funcResource[] = "unknown";
    const jerry_char_t funcArgList[] = "v1 ";
    const jerry_char_t funcSrc[] = " return new Date(v1) ";

    jerry_value_t funcVal = jerry_parse_function(funcResource, sizeof(funcResource) - 1, funcArgList,
        sizeof(funcArgList) - 1, funcSrc, sizeof(funcSrc) - 1, JERRY_PARSE_NO_OPTS);

    jerry_value_t value = jerry_create_number(time);
    jerry_value_t funcArgs[1] = { value };

    jerry_value_t valRet = jerry_call_function(funcVal, funcArgs[0], funcArgs, 1);
    jerry_release_value(funcVal);
    return valRet;
}

double jerry_get_date(jerry_value_t value)
{
    const jerry_char_t funcResource[] = "unknown";
    const jerry_char_t funcArgList[] = "v1 ";
    const jerry_char_t funcSrc[] = " return v1.getTime() ";

    jerry_value_t funcVal = jerry_parse_function(funcResource, sizeof(funcResource) - 1, funcArgList,
        sizeof(funcArgList) - 1, funcSrc, sizeof(funcSrc) - 1, JERRY_PARSE_NO_OPTS);

    jerry_value_t funcArgs[1] = { value };

    jerry_value_t valRet = jerry_call_function(funcVal, funcArgs[0], funcArgs, 1);

    double retTime = jerry_get_number_value(valRet);

    jerry_release_value(valRet);
    jerry_release_value(funcVal);

    return retTime;
}

bool jerry_is_date(jerry_value_t value)
{
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t date = jerryx_get_property_str(global, "Date");
    jerry_value_t result = jerry_binary_operation(JERRY_BIN_OP_INSTANCEOF, value, date);

    bool retVal = jerry_get_boolean_value(result);

    jerry_release_value(result);
    jerry_release_value(date);
    jerry_release_value(global);

    return retVal;
}
