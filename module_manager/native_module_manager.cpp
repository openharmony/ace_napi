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

NativeModuleManager NativeModuleManager::instance_;

NativeModuleManager::NativeModuleManager()
{
    firstNativeModule_ = nullptr;
    lastNativeModule_ = nullptr;

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
        lastNativeModule_ = firstNativeModule_;
    } else {
        lastNativeModule_->next = new NativeModule();
        lastNativeModule_ = lastNativeModule_->next;
    }

    lastNativeModule_->version = nativeModule->version;
    lastNativeModule_->fileName = nativeModule->fileName;
    lastNativeModule_->name = nativeModule->name;
    lastNativeModule_->refCount = nativeModule->refCount;
    lastNativeModule_->registerCallback = nativeModule->registerCallback;
    lastNativeModule_->next = nullptr;
}

NativeModule* NativeModuleManager::LoadNativeModule(const char* moduleName)
{
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
        nativeModule = FindNativeModuleByDisk(moduleName);
    }

    if (pthread_mutex_unlock(&mutex_) != 0) {
        HILOG_ERROR("pthread_mutex_unlock is failed");
        return nullptr;
    }

    return nativeModule;
}

bool NativeModuleManager::GetNativeModulePath(const char* moduleName, char* nativeModulePath, int32_t pathLength)
{
    const char* soPostfix = ".so";
#ifdef _ARM64_
    const char* prefix = "/system/lib64/module";
#else
    const char* prefix = "/system/lib/module";
#endif

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
        if (sprintf_s(nativeModulePath, pathLength, "%s/%s", prefix, dupModuleName) == -1) {
            return false;
        }
        return true;
    }

    char* lastDot = strrchr(dupModuleName, '.');
    if (lastDot == nullptr) {
        if (sprintf_s(nativeModulePath, pathLength, "%s/lib%s.z.so", prefix, dupModuleName) == -1) {
            return false;
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
        if (sprintf_s(nativeModulePath, pathLength, "%s/%s/lib%s.z.so", prefix, dupModuleName, afterDot) == -1) {
            return false;
        }
    }
    return true;
}

NativeModule* NativeModuleManager::FindNativeModuleByDisk(const char* moduleName)
{
    char nativeModulePath[PATH_MAX] = { 0 };
    if (!GetNativeModulePath(moduleName, nativeModulePath, sizeof(nativeModulePath))) {
        HILOG_ERROR("get module filed");
        return nullptr;
    }

    HILOG_INFO("get module path: %{public}s", nativeModulePath);
    if (strlen(nativeModulePath) <= 0) {
        return nullptr;
    }
    void* lib = dlopen(nativeModulePath, RTLD_LAZY);
    if (lib == nullptr) {
        HILOG_ERROR("dlopen failed: %{public}s", dlerror());
        return nullptr;
    }

    return lastNativeModule_;
}

NativeModule* NativeModuleManager::FindNativeModuleByCache(const char* moduleName)
{
    NativeModule* result = nullptr;
    for (NativeModule* temp = firstNativeModule_; temp != nullptr; temp = temp->next) {
        if (!strcmp(temp->name, moduleName)) {
            result = temp;
            break;
        }
    }
    return result;
}
