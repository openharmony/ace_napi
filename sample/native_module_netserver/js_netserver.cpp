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

#include "net_server.h"

napi_value NetServer::JS_Constructor(napi_env env, napi_callback_info cbinfo)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, nullptr, nullptr, &thisVar, &data);

    NetServer* netServer = new NetServer(env, thisVar);

    napi_wrap(
        env, thisVar, netServer,
        [](napi_env env, void* data, void* hint) {
            NetServer* netServer = (NetServer*)data;
            delete netServer;
        },
        nullptr, nullptr);

    return thisVar;
}

napi_value NetServer::JS_Start(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 1;
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    NetServer* netServer = nullptr;
    napi_unwrap(env, thisVar, (void**)&netServer);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 1 parameter");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_number, "type mismatch for parameter 1");

    int32_t port = 0;
    napi_get_value_int32(env, argv[0], &port);

    netServer->Start(port);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NetServer::JS_Stop(napi_env env, napi_callback_info cbinfo)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    NetServer* netServer = nullptr;
    napi_unwrap(env, thisVar, (void**)&netServer);

    netServer->Stop();

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NetServer::JS_On(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    NetServer* netServer = nullptr;
    napi_unwrap(env, thisVar, (void**)&netServer);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[1], &eventHandleType);
    NAPI_ASSERT(env, eventHandleType == napi_function, "type mismatch for parameter 2");

    char type[64] = { 0 };
    size_t typeLen = 0;

    napi_get_value_string_utf8(env, argv[0], type, sizeof(type), &typeLen);

    netServer->On((const char*)type, argv[1]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NetServer::JS_Once(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    NetServer* netServer = nullptr;
    napi_unwrap(env, thisVar, (void**)&netServer);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[1], &eventHandleType);
    NAPI_ASSERT(env, eventValueType == napi_function, "type mismatch for parameter 2");

    char* type = nullptr;
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);

    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    type = new char[typeLen + 1];
    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    netServer->Once(type, argv[1]);

    delete []type;

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NetServer::JS_Off(napi_env env, napi_callback_info cbinfo)
{
    size_t requireArgc = 1;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    void* data = nullptr;
    napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, &data);

    NetServer* netServer = nullptr;
    napi_unwrap(env, thisVar, (void**)&netServer);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameter");

    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "type mismatch for parameter 1");

    napi_valuetype eventHandleType = napi_undefined;
    napi_typeof(env, argv[1], &eventHandleType);
    NAPI_ASSERT(env, eventValueType == napi_function, "type mismatch for parameter 2");

    char* type = nullptr;
    size_t typeLen = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen);

    NAPI_ASSERT(env, typeLen > 0, "typeLen == 0");
    type = new char[typeLen + 1];

    napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen);

    if (argc > requireArgc) {
        netServer->Off(type, argv[1]);
    } else {
        netServer->Off(type);
    }
    delete []type;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NetServer::Export(napi_env env, napi_value exports)
{
    const char className[] = "NetServer";
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("start", JS_Start), DECLARE_NAPI_FUNCTION("stop", JS_Stop),
        DECLARE_NAPI_FUNCTION("on", JS_On),       DECLARE_NAPI_FUNCTION("once", JS_Once),
        DECLARE_NAPI_FUNCTION("off", JS_Off),
    };
    napi_value netServerClass = nullptr;

    napi_define_class(env, className, sizeof(className), JS_Constructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &netServerClass);

    napi_set_named_property(env, exports, "NetServer", netServerClass);

    return exports;
}