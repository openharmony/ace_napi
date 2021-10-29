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

#include "ark_native_function.h"

#include "utils/log.h"

using panda::ArrayRef;
using panda::NativePointerRef;
using panda::FunctionRef;
using panda::StringRef;
ArkNativeFunction::ArkNativeFunction(ArkNativeEngine* engine, Local<JSValueRef> value) : ArkNativeObject(engine, value)
{
}

// common function
ArkNativeFunction::ArkNativeFunction(ArkNativeEngine* engine,
                                     const char* name,
                                     size_t length,
                                     NativeCallback cb,
                                     void* value)
    : ArkNativeFunction(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    LocalScope scope(vm);

    NativeFunctionInfo* funcInfo = NativeFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return;
    }
    funcInfo->engine = engine;
    funcInfo->callback = cb;
    funcInfo->data = value;

    Local<FunctionRef> fn = FunctionRef::New(vm, NativeFunctionCallBack,
                                             [](void* data, void* hint) {
                                                auto info = reinterpret_cast<NativeFunctionInfo*>(data);
                                                if (info != nullptr) {
                                                    delete info;
                                                }
                                             },
                                             reinterpret_cast<void*>(funcInfo));
    Local<StringRef> fnName = StringRef::NewFromUtf8(vm, name);
    fn->SetName(vm, fnName);

    Global<JSValueRef> globalFn(vm, fn);
    value_ = globalFn;
}


// class function
ArkNativeFunction::ArkNativeFunction(ArkNativeEngine* engine,
                                     const char* name,
                                     NativeCallback cb,
                                     void* value)
    : ArkNativeFunction(engine, JSValueRef::Undefined(engine->GetEcmaVm()))
{
    auto vm = const_cast<EcmaVM*>(engine->GetEcmaVm());
    LocalScope scope(vm);

    NativeFunctionInfo* funcInfo = NativeFunctionInfo::CreateNewInstance();
    if (funcInfo == nullptr) {
        HILOG_ERROR("funcInfo is nullptr");
        return;
    }
    funcInfo->engine = engine;
    funcInfo->callback = cb;
    funcInfo->data = value;

    Local<FunctionRef> fn = FunctionRef::NewClassFunction(vm, NativeClassFunctionCallBack,
                                                          [](void* data, void* hint) {
                                                              auto info = reinterpret_cast<NativeFunctionInfo*>(data);
                                                              if (info != nullptr) {
                                                                 delete info;
                                                              }
                                                          },
                                                          reinterpret_cast<void*>(funcInfo));
    Local<StringRef> fnName = StringRef::NewFromUtf8(vm, name);
    fn->SetName(vm, fnName);

    Global<JSValueRef> globalFn(vm, fn);
    value_ = globalFn;
}

ArkNativeFunction::~ArkNativeFunction()
{
}

void* ArkNativeFunction::GetInterface(int interfaceId)
{
    return (NativeFunction::INTERFACE_ID == interfaceId) ? (NativeFunction*)this
                                                         : ArkNativeObject::GetInterface(interfaceId);
}

Local<JSValueRef> ArkNativeFunction::NativeFunctionCallBack(EcmaVM* vm,
                                                            Local<JSValueRef> thisObj,
                                                            const Local<JSValueRef> argv[],
                                                            int32_t length,
                                                            void* data)
{
    panda::EscapeLocalScope scope(vm);
    auto info = reinterpret_cast<NativeFunctionInfo*>(data);
    auto engine = reinterpret_cast<ArkNativeEngine*>(info->engine);
    auto cb = info->callback;
    if (engine == nullptr) {
        HILOG_ERROR("native engine is null");
        return JSValueRef::Undefined(vm);
    }

    NativeCallbackInfo cbInfo = { 0 };
    NativeScopeManager* scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        HILOG_ERROR("scope manager is null");
        return JSValueRef::Undefined(vm);
    }
    NativeScope* nativeScope = scopeManager->Open();
    cbInfo.thisVar = ArkNativeEngine::ArkValueToNativeValue(engine, thisObj);
    cbInfo.function = nullptr;
    cbInfo.argc = length;
    cbInfo.argv = nullptr;
    cbInfo.functionInfo = info;
    if (cbInfo.argc > 0) {
        cbInfo.argv = new NativeValue* [cbInfo.argc];
        for (size_t i = 0; i < cbInfo.argc; i++) {
            cbInfo.argv[i] = ArkNativeEngine::ArkValueToNativeValue(engine, argv[i]);
        }
    }

    NativeValue* result = nullptr;
    if (cb != nullptr) {
        result = cb(engine, &cbInfo);
    }

    if (cbInfo.argv != nullptr) {
        delete[] cbInfo.argv;
        cbInfo.argv = nullptr;
    }

    Global<JSValueRef> ret(vm, JSValueRef::Undefined(vm));
    if (result == nullptr) {
        if (engine->IsExceptionPending()) {
            [[maybe_unused]] NativeValue* error = engine->GetAndClearLastException();
        }
    } else {
        ret = *result;
    }
    auto localRet = ret.ToLocal(vm);
    scopeManager->Close(nativeScope);
    if (localRet.IsEmpty()) {
        return scope.Escape(JSValueRef::Undefined(vm));
    }
    return scope.Escape(localRet);
}

Local<JSValueRef> ArkNativeFunction::NativeClassFunctionCallBack(EcmaVM* vm,
                                                                 Local<JSValueRef> function,
                                                                 Local<JSValueRef> newTarget,
                                                                 const Local<JSValueRef> argv[],
                                                                 int32_t length,
                                                                 void* data)
{
    panda::EscapeLocalScope scope(vm);
    auto info = reinterpret_cast<NativeFunctionInfo*>(data);
    auto engine = reinterpret_cast<ArkNativeEngine*>(info->engine);
    auto cb = info->callback;
    if (engine == nullptr) {
        HILOG_ERROR("native engine is null");
        return JSValueRef::Undefined(vm);
    }

    NativeCallbackInfo cbInfo = { 0 };
    NativeScopeManager* scopeManager = engine->GetScopeManager();
    if (scopeManager == nullptr) {
        HILOG_ERROR("scope manager is null");
        return JSValueRef::Undefined(vm);
    }
    NativeScope* nativeScope = scopeManager->Open();
    cbInfo.thisVar = ArkNativeEngine::ArkValueToNativeValue(engine, function);
    cbInfo.function = ArkNativeEngine::ArkValueToNativeValue(engine, newTarget);
    cbInfo.argc = length;
    cbInfo.argv = nullptr;
    cbInfo.functionInfo = info;
    if (cbInfo.argc > 0) {
        cbInfo.argv = new NativeValue* [cbInfo.argc];
        for (size_t i = 0; i < cbInfo.argc; i++) {
            cbInfo.argv[i] = ArkNativeEngine::ArkValueToNativeValue(engine, argv[i]);
        }
    }

    NativeValue* result = nullptr;
    if (cb != nullptr) {
        result = cb(engine, &cbInfo);
    }

    if (cbInfo.argv != nullptr) {
        delete[] cbInfo.argv;
        cbInfo.argv = nullptr;
    }

    Global<JSValueRef> ret(vm, JSValueRef::Undefined(vm));
    if (result == nullptr) {
        if (engine->IsExceptionPending()) {
            [[maybe_unused]] NativeValue* error = engine->GetAndClearLastException();
        }
    } else {
        ret = *result;
    }
    auto localRet = ret.ToLocal(vm);
    scopeManager->Close(nativeScope);
    if (localRet.IsEmpty()) {
        return scope.Escape(JSValueRef::Undefined(vm));
    }
    return scope.Escape(localRet);
}

NativeValue* ArkNativeFunction::GetFunctionPrototype()
{
    auto vm = engine_->GetEcmaVm();
    LocalScope scope(vm);
    Global<FunctionRef> func = value_;
    Local<JSValueRef> prototype = func->GetFunctionPrototype(vm);
    return ArkNativeEngine::ArkValueToNativeValue(engine_, prototype);
}