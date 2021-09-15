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

#include "v8_native_string.h"

#include "utils/log.h"

V8NativeString::V8NativeString(V8NativeEngine* engine, const char* value, size_t length)
    : V8NativeString(
          engine,
          v8::String::NewFromUtf8(engine->GetIsolate(), value, v8::NewStringType::kNormal, length).ToLocalChecked())
{
}
V8NativeString::V8NativeString(V8NativeEngine* engine, v8::Local<v8::Value> value) : V8NativeValue(engine, value) {}

V8NativeString::~V8NativeString() {}

void* V8NativeString::GetInterface(int interfaceId)
{
    return (NativeString::INTERFACE_ID == interfaceId) ? (NativeString*)this : nullptr;
}

void V8NativeString::GetCString(char* buffer, size_t size, size_t* length)
{
    if (length == nullptr) {
        HILOG_ERROR("length is nullptr");
        return;
    }

    v8::Local<v8::String> val = value_;
    if (buffer == nullptr) {
        *length = val->Utf8Length(engine_->GetIsolate());
    } else if (size != 0) {
        int copied = val->WriteUtf8(engine_->GetIsolate(), buffer, size, nullptr,
                                            v8::String::REPLACE_INVALID_UTF8 | v8::String::NO_NULL_TERMINATION);
        buffer[copied] = '\0';
        *length = copied;
    } else {
        *length = 0;
    }
}

size_t V8NativeString::GetLength()
{
    v8::Local<v8::String> value = value_;
    return value->Utf8Length(engine_->GetIsolate());
}

size_t V8NativeString::EncodeWriteUtf8(char* buffer, size_t bufferSize, int32_t* nchars)
{
    if (nchars == nullptr) {
        HILOG_ERROR("nchars is nullptr");
        return 0;
    }

    v8::Local<v8::String> val = value_;
    int32_t copied = 0;
    if (buffer == nullptr) {
        copied = val->Utf8Length(engine_->GetIsolate());
    } else {
        copied = val->WriteUtf8(engine_->GetIsolate(),
                                buffer,
                                static_cast<int32_t>(bufferSize),
                                nchars,
                                v8::String::REPLACE_INVALID_UTF8 | v8::String::NO_NULL_TERMINATION);
        buffer[copied] = '\0';
    }

    return static_cast<size_t>(copied);
}
