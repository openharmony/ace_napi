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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_PROPERTY_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_PROPERTY_H

#include "native_engine/native_value.h"

class NativeEngine;

#define NATIVE_DEFAULT 0
#define NATIVE_WRITABLE 1 << 0
#define NATIVE_ENUMERABLE 1 << 1
#define NATIVE_CONFIGURABLE 1 << 2
#define NATIVE_STATIC 1 << 10
#define NATIVE_DEFAULT_METHOD NATIVE_WRITABLE | NATIVE_CONFIGURABLE,
#define NATIVE_DEFAULT_PROPERTY NATIVE_WRITABLE | NATIVE_ENUMERABLE | NATIVE_CONFIGURABLE

struct NativePropertyDescriptor {
    const char* utf8name = nullptr;
    NativeValue* name = nullptr;
    NativeCallback method = nullptr;
    NativeCallback getter = nullptr;
    NativeCallback setter = nullptr;
    NativeValue* value = nullptr;
    uint32_t attributes = NATIVE_DEFAULT;
    void* data = nullptr;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_PROPERTY_H */
