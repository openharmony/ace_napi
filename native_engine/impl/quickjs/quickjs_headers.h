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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_QUICKJS_HEADERS_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_QUICKJS_HEADERS_H

extern "C" {
#include "cutils.h"
#include "quickjs-libc.h"
}

typedef void (*JSFinalizer)(JSContext* context, void* data, void* hint);

JSClassID GetBaseClassID();

void AddIntrinsicBaseClass(JSContext* context);
void AddIntrinsicExternal(JSContext* context);

JSValue JS_NewExternal(JSContext* context, void* value, JSFinalizer finalizer, void* hint);
void* JS_ExternalToNativeObject(JSContext* context, JSValue value);
bool JS_IsExternal(JSContext* context, JSValue value);

void JS_SetNativePointer(JSContext* context, JSValue value, void* pointer, JSFinalizer finalizer, void* hint);
void* JS_GetNativePointer(JSContext* context, JSValue value);

bool JS_IsPromise(JSContext* context, JSValue value);
bool JS_IsArrayBuffer(JSContext* context, JSValue value);
bool JS_IsDate(JSContext* context, JSValue value);
bool JS_IsDataView(JSContext* context, JSValue value);
bool JS_IsTypedArray(JSContext* context, JSValue value);
bool JS_StrictEquals(JSContext* context, JSValue v1, JSValue v2);

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_QUICKJS_HEADERS_H */
