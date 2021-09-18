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

#include "jerryscript_native_function.h"

namespace {
jerry_object_native_info_t g_freeCallback = {
    .free_cb = [](void* nativePointer) -> void {
        delete ((NativeFunctionInfo*)nativePointer);
    }
};
} // namespace

JerryScriptNativeFunction::JerryScriptNativeFunction(JerryScriptNativeEngine* engine,
                                                     const char* name,
                                                     NativeCallback cb,
                                                     void* value)
    : JerryScriptNativeObject(engine, jerry_create_external_function(NativeFunctionCallback))
{
    NativeFunctionInfo* functionInfo = new NativeFunctionInfo();
    functionInfo->data = value;
    functionInfo->callback = cb;
    functionInfo->engine = engine;
    jerry_set_object_native_pointer(value_, functionInfo, &g_freeCallback);
}

JerryScriptNativeFunction::JerryScriptNativeFunction(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeObject(engine, value)
{
}

JerryScriptNativeFunction::~JerryScriptNativeFunction() {}

void* JerryScriptNativeFunction::GetInterface(int interfaceId)
{
    return (NativeFunction::INTERFACE_ID == interfaceId) ? (NativeFunction*)this
                                                         : JerryScriptNativeObject::GetInterface(interfaceId);
}

jerry_value_t JerryScriptNativeFunction::NativeFunctionCallback(const jerry_value_t function,
                                                                const jerry_value_t thisVal,
                                                                const jerry_value_t args[],
                                                                const jerry_length_t argc)
{
    NativeFunctionInfo* functionInfo = nullptr;
    jerry_get_object_native_pointer(function, (void**)&functionInfo, &g_freeCallback);
    auto engine = (JerryScriptNativeEngine*)functionInfo->engine;

    NativeScopeManager* scopeManager = engine->GetScopeManager();
    NativeScope* scope = scopeManager->Open();

    NativeCallbackInfo callbackInfo;
    callbackInfo.thisVar = JerryScriptNativeEngine::JerryValueToNativeValue(engine, jerry_acquire_value(thisVal));
    callbackInfo.function = JerryScriptNativeEngine::JerryValueToNativeValue(engine, jerry_acquire_value(function));
    callbackInfo.argc = argc;
    callbackInfo.argv = nullptr;

    if (argc > 0) {
        callbackInfo.argv = new NativeValue*[argc];
        for (uint32_t i = 0; i < argc; i++) {
            callbackInfo.argv[i] =
                JerryScriptNativeEngine::JerryValueToNativeValue(engine, jerry_acquire_value(args[i]));
        }
    }

    NativeValue* result = functionInfo->callback(functionInfo->engine, &callbackInfo);
    
    if (callbackInfo.argv != nullptr) {
        delete[] callbackInfo.argv;
    }
    
    jerry_value_t ret = 0;
    if (result != nullptr) {
        ret = jerry_acquire_value(*result);
    } else if (engine->IsExceptionPending()) {
        ret = jerry_acquire_value(*engine->GetAndClearLastException());
    } else {
        ret = jerry_create_undefined();
    }

    scopeManager->Close(scope);
    return ret;
}
