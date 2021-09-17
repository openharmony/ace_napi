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

#include "jerryscript_native_external.h"

JerryScriptNativeExternal::JerryScriptNativeExternal(JerryScriptNativeEngine* engine,
                                                     void* value,
                                                     NativeFinalize callback,
                                                     void* hint)
    : JerryScriptNativeExternal(engine, 0)
{
    NativeObjectInfo* info = new NativeObjectInfo();
    info->engine = engine;
    info->nativeObject = value;
    info->callback = callback;
    info->hint = hint;

    value_ = jerry_create_external(
        info,
        [](void* data, void* hint) {
            auto info = reinterpret_cast<NativeObjectInfo*>(data);
            if (info != nullptr) {
                info->callback(info->engine, info->nativeObject, info->hint);
                delete info;
            }
        },
        nullptr);
}

JerryScriptNativeExternal::JerryScriptNativeExternal(JerryScriptNativeEngine* engine, jerry_value_t value)
    : JerryScriptNativeObject(engine, value)
{
}

JerryScriptNativeExternal::~JerryScriptNativeExternal() {}

void* JerryScriptNativeExternal::GetInterface(int interfaceId)
{
    return (NativeExternal::INTERFACE_ID == interfaceId) ? (NativeExternal*)this
                                                         : JerryScriptNativeObject::GetInterface(interfaceId);
}

JerryScriptNativeExternal::operator void*()
{
    auto info = (NativeObjectInfo*)jerry_value_get_external(value_);
    if (info != nullptr) {
        return info->nativeObject;
    } else {
        return nullptr;
    }
}