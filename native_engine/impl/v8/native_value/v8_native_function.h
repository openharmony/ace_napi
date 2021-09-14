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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_FUNCTION_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_FUNCTION_H

#include "v8_native_object.h"

class V8NativeFunction : public V8NativeObject, public NativeFunction {
public:
    V8NativeFunction(V8NativeEngine* engine, v8::Local<v8::Value> value);
    V8NativeFunction(V8NativeEngine* engine, const char* name, size_t length, NativeCallback cb, void* value);
    virtual ~V8NativeFunction();

    virtual void* GetInterface(int interfaceId) override;

private:
    static void NativeFunctionCallback(const v8::FunctionCallbackInfo<v8::Value>& info);
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_V8_NATIVE_VALUE_V8_NATIVE_FUNCTION_H */