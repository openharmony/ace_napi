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

#include "quickjs_headers.h"

#include "native_value/quickjs_native_value.h"
#include "quickjs_native_reference.h"

QuickJSNativeReference::QuickJSNativeReference(QuickJSNativeEngine* engine,
                                               NativeValue* value,
                                               uint32_t initialRefcount)
{
    engine_ = engine;
    value_ = *value;
    refCount_ = initialRefcount;

    for (uint32_t i = 0; i < initialRefcount; i++) {
        JS_DupValue(engine_->GetContext(), value_);
    }
}

QuickJSNativeReference::~QuickJSNativeReference()
{
    while (refCount_) {
        JS_FreeValue(engine_->GetContext(), value_);
        refCount_--;
    }
}

uint32_t QuickJSNativeReference::Ref()
{
    if (refCount_ >= 0) {
        ++refCount_;
        JS_DupValue(engine_->GetContext(), value_);
    }
    return refCount_;
}

uint32_t QuickJSNativeReference::Unref()
{
    if (refCount_ > 0) {
        --refCount_;
        JS_FreeValue(engine_->GetContext(), value_);
    }
    return refCount_;
}

NativeValue* QuickJSNativeReference::Get()
{
    return new QuickJSNativeValue(engine_, JS_DupValue(engine_->GetContext(), value_));
}

QuickJSNativeReference::operator NativeValue*()
{
    return Get();
}
