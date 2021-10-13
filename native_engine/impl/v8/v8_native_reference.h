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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_V8_NATIVE_REFERENCE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_V8_NATIVE_REFERENCE_H

#include "native_engine/native_value.h"

#include "native_engine/native_reference.h"

class V8NativeEngine;

class V8NativeReference : public NativeReference {
public:
    V8NativeReference(V8NativeEngine* engine,
                      NativeValue* value,
                      uint32_t initialRefcount,
                      bool deleteSelf,
                      NativeFinalize callback = nullptr,
                      void* data = nullptr,
                      void* hint = nullptr);
    virtual ~V8NativeReference();

    virtual uint32_t Ref() override;
    virtual uint32_t Unref() override;
    virtual NativeValue* Get() override;
    virtual void* GetData() override;
    virtual operator NativeValue*() override;

private:
    static void FinalizeCallback(const v8::WeakCallbackInfo<V8NativeReference> &data);
    static void SecondPassCallback(const v8::WeakCallbackInfo<V8NativeReference> &data);

    V8NativeEngine* engine_;
    v8::Global<v8::Value> value_;
    uint32_t refCount_;
    bool deleteSelf_;
    NativeFinalize callback_;
    void *data_;
    void *hint_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_V8_NATIVE_REFERENCE_H */