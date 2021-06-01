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

#include "quickjs_native_string.h"

#include "utils/log.h"

#include "securec.h"

QuickJSNativeString::QuickJSNativeString(QuickJSNativeEngine* engine, JSValue value) : QuickJSNativeValue(engine, value)
{
}

QuickJSNativeString::QuickJSNativeString(QuickJSNativeEngine* engine, JSAtom value)
    : QuickJSNativeString(engine, JS_AtomToString(engine->GetContext(), value))
{
}

QuickJSNativeString::QuickJSNativeString(QuickJSNativeEngine* engine, const char* value, size_t length)
    : QuickJSNativeString(engine, JS_NewStringLen(engine->GetContext(), value, length))
{
}

QuickJSNativeString::~QuickJSNativeString() {}

void* QuickJSNativeString::GetInterface(int interfaceId)
{
    return (NativeString::INTERFACE_ID == interfaceId) ? (NativeString*)this : nullptr;
}

void QuickJSNativeString::GetCString(char* buffer, size_t size, size_t* length)
{
    const char* str = JS_ToCStringLen(engine_->GetContext(), length, value_);

    if (str == nullptr) {
        HILOG_ERROR("JS_ToCStringLen return value is null");
        return;
    }

    if (buffer != nullptr && length != nullptr) {
        int ret = strncpy_s(buffer, size, str, *length);
        if (ret != EOK) {
            HILOG_ERROR("strncpy_s failed");
        }
    }
    JS_FreeCString(engine_->GetContext(), str);
}

size_t QuickJSNativeString::GetLength()
{
    size_t length = 0;
    GetCString(nullptr, 0, &length);
    return length;
}
