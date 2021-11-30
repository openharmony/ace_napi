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

#include "jerryscript_native_engine.h"
#include "./native_value/jerryscript_native_object.h"
#include "jerryscript_native_reference.h"

JerryScriptNativeReference::JerryScriptNativeReference(JerryScriptNativeEngine* engine,
                                                       NativeValue* value,
                                                       uint32_t initialRefcount,
                                                       NativeFinalize callback,
                                                       void* data,
                                                       void* hint)
    : engine_(engine), value_(*value), refCount_(initialRefcount),
    callback_(callback), data_(data), hint_(hint)
{
    for (uint32_t i = 0; i < refCount_; i++) {
        jerry_acquire_value(value_);
    }
}

JerryScriptNativeReference::~JerryScriptNativeReference()
{
    FinalizeCallback();
    for (uint32_t i = 0; i < refCount_; i++) {
        jerry_release_value(value_);
    }
}

uint32_t JerryScriptNativeReference::Ref()
{
    if (refCount_ == 0) {
        return 0;
    }
    jerry_acquire_value(value_);
    return ++refCount_;
}

uint32_t JerryScriptNativeReference::Unref()
{
    if (refCount_ == 0) {
        return 0;
    }

    --refCount_;
    if (refCount_ == 0) {
        FinalizeCallback();
    }
    jerry_release_value(value_);
    return refCount_;
}

NativeValue* JerryScriptNativeReference::Get()
{
    if (refCount_ > 0) {
        return JerryScriptNativeEngine::JerryValueToNativeValue(engine_, jerry_acquire_value(value_));
    } else {
        return nullptr;
    }
}

JerryScriptNativeReference::operator NativeValue*()
{
    return Get();
}

void JerryScriptNativeReference::FinalizeCallback(void)
{
    if (callback_ != nullptr) {
        JerryScriptNativeObject* nativeObject = new JerryScriptNativeObject(engine_, jerry_value_to_object(value_));
        if (nativeObject != nullptr) {
            nativeObject->AddFinalizer(nullptr, nullptr, nullptr);
            delete nativeObject;
        }
        callback_(engine_, data_, hint_);
    }
    callback_ = nullptr;
    data_ = nullptr;
    hint_ = nullptr;
}