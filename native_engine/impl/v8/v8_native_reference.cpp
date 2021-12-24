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

#include "v8_native_engine.h"

#include "utils/log.h"
#include "v8_native_reference.h"

V8NativeReference::V8NativeReference(V8NativeEngine* engine,
                                     NativeValue* value,
                                     uint32_t initialRefcount,
                                     bool deleteSelf,
                                     NativeFinalize callback,
                                     void* data,
                                     void* hint)
    : engine_(engine),
      value_(),
      refCount_(initialRefcount),
      deleteSelf_(deleteSelf),
      callback_(callback),
      data_(data),
      hint_(hint)
{
    v8::Local<v8::Value> v8Value = *value;
    value_.Reset(engine->GetIsolate(), v8Value);
    if (initialRefcount == 0) {
        value_.SetWeak(this, FinalizeCallback, v8::WeakCallbackType::kParameter);
    }
    if (deleteSelf) {
        NativeReferenceManager* referenceManager = engine->GetReferenceManager();
        if (referenceManager != nullptr) {
            referenceManager->CreateHandler(this);
        }
    }
}

V8NativeReference::~V8NativeReference()
{
    if (deleteSelf_ && engine_->GetReferenceManager()) {
        engine_->GetReferenceManager()->ReleaseHandler(this);
    }
    if (value_.IsEmpty()) {
        HILOG_WARN("V8NativeReference::~V8NativeReference value is empty");
        return;
    }

    value_.SetWeak(this, FinalizeCallback, v8::WeakCallbackType::kParameter);
    if (callback_) {
        callback_(engine_, data_, hint_);
    }
}

uint32_t V8NativeReference::Ref()
{
    ++refCount_;
    if (refCount_ == 1) {
        value_.ClearWeak();
    }
    return refCount_;
}

uint32_t V8NativeReference::Unref()
{
    --refCount_;
    uint32_t refCount = refCount_;
    if (refCount == 0) {
        value_.SetWeak(this, FinalizeCallback, v8::WeakCallbackType::kParameter);
    }
    return refCount;
}

NativeValue* V8NativeReference::Get()
{
    v8::Local<v8::Value> value = value_.Get(engine_->GetIsolate());
    return V8NativeEngine::V8ValueToNativeValue(engine_, value);
}

void* V8NativeReference::GetData()
{
    return data_;
}

V8NativeReference::operator NativeValue*()
{
    return Get();
}

void V8NativeReference::FinalizeCallback(const v8::WeakCallbackInfo<V8NativeReference>& data)
{
    V8NativeReference* that = data.GetParameter();
    that->value_.Reset();
    data.SetSecondPassCallback(SecondPassCallback);
}

void V8NativeReference::SecondPassCallback(const v8::WeakCallbackInfo<V8NativeReference>& data)
{
    V8NativeReference* that = data.GetParameter();
    if (that->callback_ != nullptr) {
        that->callback_(that->engine_, that->data_, that->hint_);
    }
}