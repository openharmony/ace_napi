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

#include "native_engine/native_value.h"

#include <string.h>

struct JSObjectInfo {
    JSContext* context = nullptr;
    JSFinalizer finalizer = nullptr;
    void* data = nullptr;
    void* hint = nullptr;
};

JSClassID g_baseClassId = 0;

void AddIntrinsicExternal(JSContext* context)
{
    const char* className = "External";
    JSValue global = JS_GetGlobalObject(context);
    JSValue external = JS_NewCFunction2(
        context,
        [](JSContext* ctx, JSValueConst newTarget, int argc, JSValueConst* argv) {
            JSValue proto = JS_GetPropertyStr(ctx, newTarget, "prototype");
            JSValue result = JS_NewObjectProtoClass(ctx, proto, GetBaseClassID());
            JS_FreeValue(ctx, proto);
            return result;
        },
        className, strlen(className), JS_CFUNC_constructor, 0);
    JSValue proto = JS_NewObject(context);

    JS_DefinePropertyValueStr(context, external, "prototype", JS_DupValue(context, proto), 0);
    JS_DefinePropertyValueStr(context, proto, "constructor", JS_DupValue(context, external),
                              JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);

    JS_SetPropertyStr(context, global, "External", external);
    JS_FreeValue(context, proto);
    JS_FreeValue(context, global);
}

JSClassID GetBaseClassID()
{
    return g_baseClassId;
}

JSValue JS_NewExternal(JSContext* context, void* value, JSFinalizer finalizer, void* hint)
{
    JSValue global = JS_GetGlobalObject(context);
    JSValue external = JS_GetPropertyStr(context, global, "External");
    JSValue result = JS_CallConstructor(context, external, 0, nullptr);

    auto info = new JSObjectInfo();
    info->context = context;
    info->finalizer = finalizer;
    info->data = value;
    info->hint = hint;
    JS_SetOpaque(result, info);

    JS_FreeValue(context, external);
    JS_FreeValue(context, global);
    return result;
}

void* JS_ExternalToNativeObject(JSContext* context, JSValue value)
{
    auto* info = reinterpret_cast<JSObjectInfo*>(JS_GetOpaque(value, GetBaseClassID()));
    return (info != nullptr) ? info->data : nullptr;
}

bool JS_IsExternal(JSContext* context, JSValue value)
{
    bool result = false;
    JSValue constructor = JS_GetPropertyStr(context, value, "constructor");
    JSValue name = JS_GetPropertyStr(context, constructor, "name");
    const char* cName = JS_ToCString(context, name);
    result = !strcmp("External", cName ? cName : "");
    JS_FreeCString(context, cName);
    JS_FreeValue(context, name);
    JS_FreeValue(context, constructor);
    return result;
}

void AddIntrinsicBaseClass(JSContext* context)
{
    const JSClassDef baseClassDef = {
        .class_name = "BaseClass",
        .finalizer =
            [](JSRuntime* rt, JSValue val) {
                auto* info = reinterpret_cast<JSObjectInfo*>(JS_GetOpaque(val, GetBaseClassID()));
                if (info != nullptr) {
                    info->finalizer(info->context, info->data, info->hint);
                    delete info;
                }
            },
    };

    JS_NewClassID(&g_baseClassId);
    JS_NewClass(JS_GetRuntime(context), g_baseClassId, &baseClassDef);
}

void JS_SetNativePointer(JSContext* context, JSValue value, void* pointer, JSFinalizer finalizer, void* hint)
{
    auto* info = reinterpret_cast<JSObjectInfo*>(JS_GetOpaque(value, GetBaseClassID()));
    if (info == nullptr) {
        info = new JSObjectInfo();
        info->context = context;
        info->finalizer = finalizer;
        info->data = pointer;
        info->hint = hint;
    } else if (pointer == nullptr) {
        delete info;
        info = nullptr;
    }

    JS_SetOpaque(value, info);
}

void* JS_GetNativePointer(JSContext* context, JSValue value)
{
    auto* info = reinterpret_cast<JSObjectInfo*>(JS_GetOpaque(value, GetBaseClassID()));
    return (info != nullptr) ? info->data : nullptr;
}

bool JS_IsPromise(JSContext* context, JSValue value)
{
    bool result = false;
    JSValue constructor = JS_GetPropertyStr(context, value, "constructor");
    JSValue name = JS_GetPropertyStr(context, constructor, "name");
    const char* cName = JS_ToCString(context, name);
    result = !strcmp("Promise", cName ? cName : "");
    JS_FreeCString(context, cName);
    JS_FreeValue(context, name);
    JS_FreeValue(context, constructor);
    return result;
}

bool JS_IsArrayBuffer(JSContext* context, JSValue value)
{
    bool result = false;
    JSValue constructor = JS_GetPropertyStr(context, value, "constructor");
    JSValue name = JS_GetPropertyStr(context, constructor, "name");
    const char* cName = JS_ToCString(context, name);
    result = !strcmp("ArrayBuffer", cName ? cName : "");
    JS_FreeCString(context, cName);
    JS_FreeValue(context, name);
    JS_FreeValue(context, constructor);
    return result;
}

bool JS_IsDate(JSContext* context, JSValue value)
{
    bool result = false;
    JSValue constructor = JS_GetPropertyStr(context, value, "constructor");
    JSValue name = JS_GetPropertyStr(context, constructor, "name");
    const char* cName = JS_ToCString(context, name);
    result = !strcmp("Date", cName ? cName : "");
    JS_FreeCString(context, cName);
    JS_FreeValue(context, name);
    JS_FreeValue(context, constructor);
    return result;
}

bool JS_IsDataView(JSContext* context, JSValue value)
{
    bool result = false;
    JSValue constructor = JS_GetPropertyStr(context, value, "constructor");
    JSValue name = JS_GetPropertyStr(context, constructor, "name");
    const char* cName = JS_ToCString(context, name);
    result = !strcmp("DataView", cName ? cName : "");
    JS_FreeCString(context, cName);
    JS_FreeValue(context, name);
    JS_FreeValue(context, constructor);
    return result;
}

bool JS_IsTypedArray(JSContext* context, JSValue value)
{
    bool result = false;
    JSValue constructor = JS_GetPropertyStr(context, value, "constructor");
    JSValue name = JS_GetPropertyStr(context, constructor, "name");
    const char* cName = JS_ToCString(context, name);
    result = !strcmp("Uint8ClampedArray", cName ? cName : "") || !strcmp("Int8Array", cName ? cName : "") ||
             !strcmp("Uint8Array", cName ? cName : "") || !strcmp("Int16Array", cName ? cName : "") ||
             !strcmp("Uint16Array", cName ? cName : "") || !strcmp("Int32Array", cName ? cName : "") ||
             !strcmp("Uint32Array", cName ? cName : "") || !strcmp("BigInt64Array", cName ? cName : "") ||
             !strcmp("BigUint64Array", cName ? cName : "") || !strcmp("Float32Array", cName ? cName : "") ||
             !strcmp("Float64Array", cName ? cName : "");

    JS_FreeCString(context, cName);
    JS_FreeValue(context, name);
    JS_FreeValue(context, constructor);
    return result;
}

bool JS_StrictEquals(JSContext* context, JSValue v1, JSValue v2)
{
    JSValue thisVar = JS_UNDEFINED;
    JSValue argv[2] = { v1, v2 };
    const char script[] = "(v1, v2) => v1 === v2;";
    JSValue func = JS_Eval(context, script, strlen(script), "<input>", JS_EVAL_TYPE_GLOBAL);
    JSValue ret = JS_Call(context, func, thisVar, 2, (JSValue*)&argv);
    bool result = JS_ToBool(context, ret);
    JS_FreeValue(context, func);
    JS_FreeValue(context, ret);

    return result;
}