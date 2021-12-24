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

#ifndef FOUNDATION_ACE_NAPI_MODULE_MANAGER_NATIVE_MODULE_MANAGER_H
#define FOUNDATION_ACE_NAPI_MODULE_MANAGER_NATIVE_MODULE_MANAGER_H

#include <pthread.h>
#include <stdint.h>
#include "utils/macros.h"

#ifdef WINDOWS_PLATFORM
#include <winsock2.h>
#include <windows.h>
using LIBHANDLE = HMODULE;
#define LIBFREE FreeLibrary
#define LIBSYM GetProcAddress
#else
#include <dlfcn.h>
using LIBHANDLE = void*;
#define LIBFREE dlclose
#define LIBSYM dlsym
#endif

#define NAPI_PATH_MAX 4096

class NativeValue;

class NativeEngine;

typedef NativeValue* (*RegisterCallback)(NativeEngine*, NativeValue*);

struct NativeModule {
    const char* name = nullptr;
    const char* fileName = nullptr;
    RegisterCallback registerCallback = nullptr;
    int32_t version = 0;
    unsigned int refCount = 0;
    NativeModule* next = nullptr;
    const char* jsCode = nullptr;
    int32_t jsCodeLen = 0;
};

class NAPI_EXPORT NativeModuleManager {
public:
    static NativeModuleManager* GetInstance();
    static unsigned long Release();

    void Register(NativeModule* nativeModule);
    void SetAppLibPath(const char* appLibPath);
    NativeModule* LoadNativeModule(const char* moduleName, const char* path, bool isAppModule, bool internal = false,
                                   bool isArk = false);

private:
    NativeModuleManager();
    virtual ~NativeModuleManager();

    bool GetNativeModulePath(const char* moduleName, const bool isAppModule, char nativeModulePath[][NAPI_PATH_MAX],
        int32_t pathLength) const;
    NativeModule* FindNativeModuleByDisk(const char* moduleName, bool internal, const bool isAppModule, bool isArk);
    NativeModule* FindNativeModuleByCache(const char* moduleName) const;
    LIBHANDLE LoadModuleLibrary(const char* path) const;

    NativeModule* firstNativeModule_;
    NativeModule* lastNativeModule_;
    char* appLibPath_;

    static NativeModuleManager instance_;
    pthread_mutex_t mutex_;
};

#endif /* FOUNDATION_ACE_NAPI_MODULE_MANAGER_NATIVE_MODULE_MANAGER_H */
