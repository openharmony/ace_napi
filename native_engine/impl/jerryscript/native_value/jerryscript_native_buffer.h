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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_BUFFER_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_BUFFER_H

#include "jerryscript_native_object.h"

struct JerryScriptBufferCallback;

class JerryScriptNativeBuffer : public JerryScriptNativeObject, public NativeBuffer {
public:
    JerryScriptNativeBuffer(JerryScriptNativeEngine* engine, jerry_value_t value);
    JerryScriptNativeBuffer(JerryScriptNativeEngine* engine, uint8_t** value, size_t length);
    JerryScriptNativeBuffer(JerryScriptNativeEngine* engine, uint8_t** value, size_t length, const uint8_t* data);
    JerryScriptNativeBuffer(JerryScriptNativeEngine* engine, uint8_t* data, size_t length,
        NativeFinalize callback, void* hint);
    ~JerryScriptNativeBuffer() override;

    void* GetInterface(int interfaceId) override;
    void* GetBuffer() override;
    size_t GetLength() override;

    bool IsBuffer() override
    {
        return true;
    }
    bool IsArrayBuffer() override
    {
        return false;
    }

private:
    jerry_value_t CheckAndCreateBuffer(size_t length);
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_JERRYSCRIPT_NATIVE_VALUE_JERRYSCRIPT_NATIVE_BUFFER_H */
