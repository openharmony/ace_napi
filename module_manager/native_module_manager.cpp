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

#include "native_module_manager.h"

#include "native_engine/native_engine.h"

#include "securec.h"
#include "utils/log.h"

#include <dirent.h>
#include <dlfcn.h>

namespace {
constexpr static int32_t NATIVE_PATH_NUMBER = 2;
} // namespace

NativeModuleManager NativeModuleManager::instance_;

NativeModuleManager::NativeModuleManager()
{
    firstNativeModule_ = nullptr;
    lastNativeModule_ = nullptr;
    appLibPath_ = nullptr;

    pthread_mutex_init(&mutex_, nullptr);
}

NativeModuleManager::~NativeModuleManager()
{
    NativeModule* nativeModule = firstNativeModule_;
    while (nativeModule != nullptr) {
        nativeModule = nativeModule->next;
        delete firstNativeModule_;
        firstNativeModule_ = nativeModule;
    }
    firstNativeModule_ = lastNativeModule_ = nullptr;
    if (appLibPath_) {
        delete[] appLibPath_;
    }

    pthread_mutex_destroy(&mutex_);
}

NativeModuleManager* NativeModuleManager::GetInstance()
{
    return &instance_;
}

void NativeModuleManager::Register(NativeModule* nativeModule)
{
    if (nativeModule == nullptr) {
        HILOG_ERROR("nativeModule value is null");
        return;
    }

    if (firstNativeModule_ == lastNativeModule_ && lastNativeModule_ == nullptr) {
        firstNativeModule_ = new NativeModule();
        if (firstNativeModule_ == nullptr) {
            HILOG_ERROR("first NativeModule create failed");
            return;
        }
        lastNativeModule_ = firstNativeModule_;
    } else {
        auto next = new NativeModule();
        if (next == nullptr) {
            HILOG_ERROR("next NativeModule create failed");
            return;
        }
        lastNativeModule_->next = next;
        lastNativeModule_ = lastNativeModule_->next;
    }

    lastNativeModule_->version = nativeModule->version;
    lastNativeModule_->fileName = nativeModule->fileName;
    lastNativeModule_->name = nativeModule->name;
    lastNativeModule_->refCount = nativeModule->refCount;
    lastNativeModule_->registerCallback = nativeModule->registerCallback;
    lastNativeModule_->next = nullptr;
}

void NativeModuleManager::SetAppLibPath(const char* appLibPath)
{
    char* tmp = new char[PATH_MAX];
    errno_t err = EOK;
    err = memset_s(tmp, PATH_MAX, 0, PATH_MAX);
    if (err != EOK) {
        return;
    }
    err = strcpy_s(tmp, PATH_MAX, appLibPath);
    if (err != EOK) {
        delete[] tmp;
        return;
    }
    appLibPath_ = tmp;
}

NativeModule* NativeModuleManager::LoadNativeModule(const char* moduleName,
    const char* path, bool isAppModule, bool internal, bool isArk)
{
    HILOG_ERROR("Kee LoadNativeModule");
    if (moduleName == nullptr) {
        HILOG_ERROR("moduleName value is null");
        return nullptr;
    }

    if (pthread_mutex_lock(&mutex_) != 0) {
        HILOG_ERROR("pthread_mutex_lock is failed");
        return nullptr;
    }

    NativeModule* nativeModule = FindNativeModuleByCache(moduleName);
    if (nativeModule == nullptr) {
        HILOG_INFO("not in cache: moduleName: %{public}s", moduleName);
        nativeModule = FindNativeModuleByDisk(moduleName, internal, isAppModule, isArk);
    }

    if (pthread_mutex_unlock(&mutex_) != 0) {
        HILOG_ERROR("pthread_mutex_unlock is failed");
        return nullptr;
    }

    return nativeModule;
}

bool NativeModuleManager::GetNativeModulePath(
    const char* moduleName, const bool isAppModule, char nativeModulePath[][PATH_MAX], int32_t pathLength) const
{
    const char* soPostfix = ".so";
#ifdef _ARM64_
    const char* sysPrefix = "/system/lib64/module";
#else
    const char* sysPrefix = "/system/lib/module";
#endif
    const char* prefix = nullptr;
    if (isAppModule && appLibPath_) {
        prefix = appLibPath_;
    } else {
        prefix = sysPrefix;
    }
    int32_t lengthOfModuleName = strlen(moduleName);
    char dupModuleName[PATH_MAX] = { 0 };
    if (strcpy_s(dupModuleName, PATH_MAX, moduleName) != 0) {
        HILOG_ERROR("strcpy moduleName failed");
        return false;
    }

    for (int32_t i = 0; i < lengthOfModuleName; i++) {
        dupModuleName[i] = tolower(dupModuleName[i]);
    }

    int32_t lengthOfPostfix = strlen(soPostfix);
    if ((lengthOfModuleName > lengthOfPostfix) &&
        (strcmp(dupModuleName + lengthOfModuleName - lengthOfPostfix, soPostfix) == 0)) {
        if (sprintf_s(nativeModulePath[0], pathLength, "%s/%s", prefix, dupModuleName) == -1) {
            return false;
        }
        return true;
    }

    char* lastDot = strrchr(dupModuleName, '.');
    if (lastDot == nullptr) {
        if (strcmp(prefix, sysPrefix) == 0) {
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/lib%s.z.so", prefix, dupModuleName) == -1) {
                return false;
            }
            if (sprintf_s(nativeModulePath[1], pathLength, "%s/lib%s_napi.z.so", prefix, dupModuleName) == -1) {
                return false;
            }
        } else {
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/lib%s.so", prefix, dupModuleName) == -1) {
                return false;
            }
        }
    } else {
        char* afterDot = lastDot + 1;
        if (*afterDot == '\0') {
            return false;
        }
        *lastDot = '\0';
        lengthOfModuleName = strlen(dupModuleName);
        for (int32_t i = 0; i < lengthOfModuleName; i++) {
            if (*(dupModuleName + i) == '.') {
                *(dupModuleName + i) = '/';
            }
        }
        if (strcmp(prefix, sysPrefix) == 0) {
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/%s/lib%s.z.so",
                prefix, dupModuleName, afterDot) == -1) {
                return false;
            }
            if (sprintf_s(nativeModulePath[1], pathLength, "%s/%s/lib%s_napi.z.so",
                prefix, dupModuleName, afterDot) == -1) {
                return false;
            }
        } else {
            if (sprintf_s(nativeModulePath[0], pathLength, "%s/%s/lib%s.so", prefix, dupModuleName, afterDot) == -1) {
                return false;
            }
        }
    }
    return true;
}

void* NativeModuleManager::LoadLibrary(const char* path) const
{
    if (strlen(path) == 0) {
        HILOG_ERROR("primary module path is empty");
        return nullptr;
    }
    void* lib = dlopen(path, RTLD_LAZY);
    if (lib == nullptr) {
        HILOG_ERROR("dlopen failed: %{public}s", dlerror());
    }
    return lib;
}

using NAPIGetJSCode = void (*)(const char** buf, int* bufLen);
NativeModule* NativeModuleManager::FindNativeModuleByDisk(const char* moduleName, bool internal, const bool isAppModule,
                                                          bool isArk)
{
    char nativeModulePath[NATIVE_PATH_NUMBER][PATH_MAX] = { 0 };
    if (!GetNativeModulePath(moduleName, isAppModule, nativeModulePath, PATH_MAX)) {
        HILOG_ERROR("get module filed");
        return nullptr;
    }

    // load primary module path first
    char* loadPath = nativeModulePath[0];
    HILOG_INFO("get primary module path: %{public}s", loadPath);
    void* lib = LoadLibrary(loadPath);
    if (lib == nullptr) {
        loadPath = nativeModulePath[1];
        HILOG_WARN("primary module path load failed, try to load secondary module path: %{public}s", loadPath);
        lib = LoadLibrary(loadPath);
        if (lib == nullptr) {
            HILOG_ERROR("secondary module path load failed, load native module failed");
            return nullptr;
        }
    }

    if (!internal) {
        char symbol[PATH_MAX] = { 0 };
        if (!isArk) {
            if (sprintf_s(symbol, sizeof(symbol), "NAPI_%s_GetJSCode", moduleName) == -1) {
                return nullptr;
            }
        } else {
            if (sprintf_s(symbol, sizeof(symbol), "NAPI_%s_GetABCCode", moduleName) == -1) {
                return nullptr;
            }
        }
        auto getJSCode = reinterpret_cast<NAPIGetJSCode>(dlsym(lib, symbol));
        if (getJSCode != nullptr) {
            const char* buf = nullptr;
            int bufLen = 0;
            getJSCode(&buf, &bufLen);
            if (lastNativeModule_ != nullptr) {
                HILOG_INFO("get js code from module: bufLen: %{public}d", bufLen);
                lastNativeModule_->jsCode = buf;
                lastNativeModule_->jsCodeLen = bufLen;
            }
        } else {
            HILOG_INFO("ignore: no %{public}s in %{public}s", symbol, loadPath);
        }
    }

    return lastNativeModule_;
}

NativeModule* NativeModuleManager::FindNativeModuleByCache(const char* moduleName) const
{
    NativeModule* result = nullptr;
    for (NativeModule* temp = firstNativeModule_; temp != nullptr; temp = temp->next) {
        if (!strcasecmp(temp->name, moduleName)) {
            result = temp;
            break;
        }
    }
    return result;
}
