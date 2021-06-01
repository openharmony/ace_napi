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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "securec.h"

#include <map>
#include <string>

namespace {
constexpr size_t EVENT_TYPE_SIZE = 32;
constexpr size_t KEY_BUFFER_SIZE = 32;
constexpr size_t VALUE_BUFFER_SIZE = 128;
} // namespace

static std::map<std::string, std::string> g_keyValueStorage;

/***********************************************
 * Storage Constructor
 ***********************************************/
struct EventHandler {
    napi_ref callbackRef = nullptr;
    EventHandler* next = nullptr;
};

class EventListener {
public:
    EventListener() : type_(nullptr), handlers_(nullptr) {}
    virtual ~EventListener() {}
    void Add(napi_env env, napi_value handler)
    {
        if (Find(env, handler) != nullptr)
            return;

        if (handlers_ == nullptr) {
            handlers_ = new EventHandler();
            handlers_->next = nullptr;
        } else {
            auto temp = new EventHandler();
            temp->next = handlers_;
            handlers_ = temp;
        }
        napi_create_reference(env, handler, 1, &handlers_->callbackRef);
    }

    void Del(napi_env env, napi_value handler)
    {
        EventHandler* temp = nullptr;
        for (EventHandler* i = handlers_; i != nullptr; i = handlers_) {
            napi_value callback = nullptr;
            napi_get_reference_value(env, i->callbackRef, &callback);
            bool isEquals = false;
            napi_strict_equals(env, handler, callback, &isEquals);
            if (isEquals) {
                if (temp == nullptr) {
                    handlers_ = i->next;
                } else {
                    temp->next = i->next;
                }
                napi_delete_reference(env, i->callbackRef);
                delete i;
            } else {
                temp = i;
            }
        }
    }

    void Clear(napi_env env)
    {
        for (EventHandler* i = handlers_; i != nullptr; i = handlers_) {
            handlers_ = i->next;
            delete i;
        }
    }

    const char* type_;
    EventHandler* handlers_;

protected:
    EventHandler* Find(napi_env env, napi_value handler)
    {
        EventHandler* result = nullptr;
        for (EventHandler* i = handlers_; i != nullptr; i = i->next) {
            napi_value callback = nullptr;
            napi_get_reference_value(env, i->callbackRef, &callback);
            bool isEquals = false;
            napi_strict_equals(env, handler, callback, &isEquals);
            if (isEquals) {
                result = i;
            }
        }
        return result;
    }
};

enum StorageEvent {
    STORAGE_EVENT_UNKNOWN = -1,
    STORAGE_EVENT_CHANGE,
    STORAGE_EVENT_CLEAR,
    STORAGE_EVENT_ERROR,
};

class StorageObjectInfo {
public:
    explicit StorageObjectInfo(napi_env env) : env_(env), listeners_()
    {
        listeners_[STORAGE_EVENT_CHANGE].type_ = "change";
        listeners_[STORAGE_EVENT_CLEAR].type_ = "clear";
        listeners_[STORAGE_EVENT_ERROR].type_ = "error";
    }

    virtual ~StorageObjectInfo()
    {
        listeners_[STORAGE_EVENT_CHANGE].Clear(env_);
        listeners_[STORAGE_EVENT_CLEAR].Clear(env_);
        listeners_[STORAGE_EVENT_ERROR].Clear(env_);
    }

    void On(const char* type, napi_value handler)
    {
        StorageEvent event = Find(type);
        if (event == STORAGE_EVENT_UNKNOWN) {
            return;
        }
        listeners_[event].Add(env_, handler);
    }

    void Off(const char* type, napi_value handler = nullptr)
    {
        StorageEvent event = Find(type);
        if (event == STORAGE_EVENT_UNKNOWN) {
            return;
        }
        if (handler == nullptr) {
            listeners_[event].Clear(env_);
        } else {
            listeners_[event].Del(env_, handler);
        }
    }

    void Emit(napi_value thisArg, const char* type)
    {
        StorageEvent event = Find(type);
        if (event == STORAGE_EVENT_UNKNOWN) {
            return;
        }
        for (EventHandler* handler = listeners_[event].handlers_; handler != nullptr; handler = handler->next) {
            if (thisArg == nullptr) {
                napi_get_undefined(env_, &thisArg);
            }
            napi_value callback = nullptr;
            napi_value result = nullptr;
            napi_get_reference_value(env_, handler->callbackRef, &callback);
            napi_call_function(env_, thisArg, callback, 0, nullptr, &result);
        }
    }

protected:
    StorageEvent Find(const char* type) const
    {
        StorageEvent result = STORAGE_EVENT_UNKNOWN;
        if (!strcmp(listeners_[STORAGE_EVENT_CHANGE].type_, type)) {
            result = STORAGE_EVENT_CHANGE;
        } else if (!strcmp(listeners_[STORAGE_EVENT_CLEAR].type_, type)) {
            result = STORAGE_EVENT_CLEAR;
        } else if (!strcmp(listeners_[STORAGE_EVENT_ERROR].type_, type)) {
            result = STORAGE_EVENT_ERROR;
        }
        return result;
    }

private:
    napi_env env_;
    EventListener listeners_[3];
};

static napi_value JSStorageConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    auto objectInfo = new StorageObjectInfo(env);
    napi_wrap(
        env, thisVar, objectInfo,
        [](napi_env env, void* data, void* hint) {
            auto objectInfo = (StorageObjectInfo*)data;
            if (objectInfo != nullptr) {
                delete objectInfo;
            }
        },
        nullptr, nullptr);

    return thisVar;
}

/***********************************************
 * Async Function Set
 ***********************************************/
struct StorageAsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;

    char key[KEY_BUFFER_SIZE] = { 0 };
    size_t keyLen = 0;
    char value[VALUE_BUFFER_SIZE] = { 0 };
    size_t valueLen = 0;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;

    int status = 0;
    StorageObjectInfo* objectInfo = nullptr;
};

// storage.get(key: string, defaultValue?: string, callback?: Function): void | Promise<string>
static napi_value JSStorageGet(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 1;
    size_t argc = 3;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 1 parameter");

    auto asyncContext = new StorageAsyncContext();

    asyncContext->env = env;

    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if ((i == 0) && (valueType == napi_string)) {
            napi_get_value_string_utf8(env, argv[i], asyncContext->key, KEY_BUFFER_SIZE, &asyncContext->keyLen);
        } else if (valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], asyncContext->value, VALUE_BUFFER_SIZE, &asyncContext->valueLen);
        } else if (valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef);
            break;
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }

    napi_value result = nullptr;

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_unwrap(env, thisVar, (void**)&asyncContext->objectInfo);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSStorageGet", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            auto itr = g_keyValueStorage.find(asyncContext->key);
            if (itr != g_keyValueStorage.end()) {
                if (strncpy_s(asyncContext->value, VALUE_BUFFER_SIZE, itr->second.c_str(), itr->second.length()) ==
                    -1) {
                    asyncContext->status = 1;
                } else {
                    asyncContext->status = 0;
                }
            } else {
                asyncContext->status = 1;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            napi_value result[2] = { 0 };
            if (!asyncContext->status) {
                napi_get_undefined(env, &result[0]);
                napi_create_string_utf8(env, asyncContext->value, strlen(asyncContext->value), &result[1]);
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "key does not exist", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
                napi_get_undefined(env, &result[1]);
                asyncContext->objectInfo->Emit(nullptr, "error");
            }
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result[0]);
                }
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(result[0]), result, nullptr);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return result;
}

// storage.set(key: string, value: string, callback?: Function): void | Promise<void>
static napi_value JSStorageSet(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 3;
    napi_value argv[3] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameters");
    auto asyncContext = new StorageAsyncContext();
    asyncContext->env = env;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == 0 && valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], asyncContext->key, KEY_BUFFER_SIZE, &asyncContext->keyLen);
        } else if (i == 1 && valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], asyncContext->value, VALUE_BUFFER_SIZE, &asyncContext->valueLen);
        } else if (i == 2 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef);
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }

    napi_value result = nullptr;

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_unwrap(env, thisVar, (void**)&asyncContext->objectInfo);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JStorageSet", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            auto itr = g_keyValueStorage.find(asyncContext->key);
            if (itr == g_keyValueStorage.end()) {
                g_keyValueStorage.insert(std::pair<std::string, std::string>(asyncContext->key, asyncContext->value));
                asyncContext->status = 0;
            } else {
                asyncContext->status = 1;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            napi_value result[2] = { 0 };
            if (!asyncContext->status) {
                napi_get_undefined(env, &result[0]);
                napi_get_undefined(env, &result[1]);
                asyncContext->objectInfo->Emit(nullptr, "change");
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "key already exists", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
                napi_get_undefined(env, &result[1]);
                asyncContext->objectInfo->Emit(nullptr, "error");
            }

            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result[0]);
                }
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(result[0]), result, nullptr);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return result;
}

// storage.delete(key: string, callback?: Function): void | Promise<void>
static napi_value JSStorageDelete(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 1;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 1 parameter");

    auto asyncContext = new StorageAsyncContext();

    asyncContext->env = env;

    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == 0 && valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], asyncContext->key, KEY_BUFFER_SIZE, &asyncContext->keyLen);
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef);
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }

    napi_value result = nullptr;

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_unwrap(env, thisVar, (void**)&asyncContext->objectInfo);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSStorageDelete", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            auto itr = g_keyValueStorage.find(asyncContext->key);
            if (itr != g_keyValueStorage.end()) {
                g_keyValueStorage.erase(itr);
                asyncContext->status = 0;
            } else {
                asyncContext->status = 1;
            }
        },
        [](napi_env env, napi_status status, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            napi_value result[2] = { 0 };
            if (!asyncContext->status) {
                napi_get_undefined(env, &result[0]);
                napi_get_undefined(env, &result[1]);
                asyncContext->objectInfo->Emit(nullptr, "change");
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "key does not exist", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
                napi_get_undefined(env, &result[1]);
                asyncContext->objectInfo->Emit(nullptr, "error");
            }

            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, result[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result[0]);
                }
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(result[0]), result, nullptr);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return result;
}

// storage.clear(callback?: Function): void | Promise<void>
static napi_value JSStorageClear(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    auto asyncContext = new StorageAsyncContext();

    asyncContext->env = env;

    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef);
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }

    napi_value result = nullptr;

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    napi_unwrap(env, thisVar, (void**)&asyncContext->objectInfo);

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "JSStorageClear", NAPI_AUTO_LENGTH, &resource);

    napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            g_keyValueStorage.clear();
            asyncContext->status = 0;
        },
        [](napi_env env, napi_status status, void* data) {
            StorageAsyncContext* asyncContext = (StorageAsyncContext*)data;
            napi_value result[2] = { 0 };
            if (!asyncContext->status) {
                napi_get_undefined(env, &result[0]);
                napi_get_undefined(env, &result[1]);
                asyncContext->objectInfo->Emit(nullptr, "clear");
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "key does not exist", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &result[0]);
                napi_get_undefined(env, &result[1]);
                asyncContext->objectInfo->Emit(nullptr, "error");
            }

            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, result[1]);
                } else {

                    napi_reject_deferred(env, asyncContext->deferred, result[0]);
                }
            } else {
                napi_value callback = nullptr;
                napi_get_reference_value(env, asyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(result[0]), result, nullptr);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return result;
}

/***********************************************
 * Sync Function Set
 ***********************************************/
// storage.getSync(key: string, defaultValue?: string): string
static napi_value JSStorageGetSync(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 1;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 1 parameter");

    char key[KEY_BUFFER_SIZE] = { 0 };
    size_t keyLen = 0;
    char value[VALUE_BUFFER_SIZE] = { 0 };
    size_t valueLen = 0;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == 0 && valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], key, KEY_BUFFER_SIZE, &keyLen);
        } else if (i == 1 && valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], value, VALUE_BUFFER_SIZE, &valueLen);
            break;
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }
    StorageObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    auto itr = g_keyValueStorage.find(key);
    napi_value result = nullptr;
    if (itr != g_keyValueStorage.end()) {
        napi_create_string_utf8(env, itr->second.c_str(), itr->second.length(), &result);
    } else if (valueLen > 0) {
        napi_create_string_utf8(env, value, valueLen, &result);
    } else {
        objectInfo->Emit(nullptr, "error");
        NAPI_ASSERT(env, false, "key does not exist");
    }
    return result;
}

// storage.setSync(key: string, value: string): void
static napi_value JSStorageSetSync(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameters");

    char key[KEY_BUFFER_SIZE] = { 0 };
    size_t keyLen = 0;
    char value[VALUE_BUFFER_SIZE] = { 0 };
    size_t valueLen = 0;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);

        if (i == 0 && valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], key, KEY_BUFFER_SIZE, &keyLen);
        } else if (i == 1 && valueType == napi_string) {
            napi_get_value_string_utf8(env, argv[i], value, VALUE_BUFFER_SIZE, &valueLen);
            break;
        } else {
            NAPI_ASSERT(env, false, "type mismatch");
        }
    }
    StorageObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    auto itr = g_keyValueStorage.find(key);
    if (itr == g_keyValueStorage.end()) {
        g_keyValueStorage.insert(std::pair<std::string, std::string>(key, value));
        objectInfo->Emit(nullptr, "change");

    } else {
        objectInfo->Emit(nullptr, "error");
        NAPI_ASSERT(env, false, "key already exists");
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

// storage.deleteSync(key: string): void
static napi_value JSStorageDeleteSync(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 1;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 1 parameter");

    char key[KEY_BUFFER_SIZE] = { 0 };
    size_t keyLen = 0;

    napi_valuetype keyType = napi_undefined;
    napi_typeof(env, argv[0], &keyType);
    NAPI_ASSERT(env, keyType == napi_string, "type mismatch");
    napi_get_value_string_utf8(env, argv[0], key, KEY_BUFFER_SIZE, &keyLen);

    StorageObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    auto itr = g_keyValueStorage.find(key);

    if (itr != g_keyValueStorage.end()) {
        g_keyValueStorage.erase(itr);
        objectInfo->Emit(nullptr, "change");
    } else {
        objectInfo->Emit(nullptr, "error");
        NAPI_ASSERT(env, itr != g_keyValueStorage.end(), "key does not exist");
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

// storage.clearSync(): void
static napi_value JSStorageClearSync(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    StorageObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);
    g_keyValueStorage.clear();
    objectInfo->Emit(nullptr, "clear");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

/***********************************************
 * Event Function Set
 ***********************************************/
// storage.on(event: "change" | "clear", callback: Function): void
static napi_value JSStorageOn(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 2 parameters");

    char eventType[EVENT_TYPE_SIZE] = { 0 };
    size_t eventTypeLen = 0;
    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "parameter 1 type mismatch");
    napi_get_value_string_utf8(env, argv[0], eventType, EVENT_TYPE_SIZE, &eventTypeLen);

    napi_valuetype callbackType = napi_undefined;
    napi_typeof(env, argv[1], &callbackType);
    NAPI_ASSERT(env, callbackType == napi_function, "parameter 2 type mismatch");

    StorageObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    objectInfo->On(eventType, argv[1]);

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

// storage.off(event: "change" | "clear", callback?: Function): void
static napi_value JSStorageOff(napi_env env, napi_callback_info info)
{
    size_t requireArgc = 1;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = nullptr;
    void* data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    NAPI_ASSERT(env, argc >= requireArgc, "requires 1 parameter");

    char eventType[EVENT_TYPE_SIZE] = { 0 };
    size_t eventTypeLen = 0;
    napi_valuetype eventValueType = napi_undefined;
    napi_typeof(env, argv[0], &eventValueType);
    NAPI_ASSERT(env, eventValueType == napi_string, "parameter 1 type mismatch");
    napi_get_value_string_utf8(env, argv[0], eventType, EVENT_TYPE_SIZE, &eventTypeLen);

    StorageObjectInfo* objectInfo = nullptr;
    napi_unwrap(env, thisVar, (void**)&objectInfo);

    if (argc > requireArgc) {
        napi_valuetype callbackType = napi_undefined;
        napi_typeof(env, argv[1], &callbackType);
        NAPI_ASSERT(env, callbackType == napi_function, "parameter 2 type mismatch");
        objectInfo->Off(eventType, argv[1]);
    } else {
        objectInfo->Off(eventType);
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

/***********************************************
 * Module export and register
 ***********************************************/
static napi_value StorageExport(napi_env env, napi_value exports)
{
    const char* storageClassName = "Storage";
    napi_value storageClass = nullptr;
    static napi_property_descriptor storageDesc[] = {
        DECLARE_NAPI_FUNCTION("get", JSStorageGet),
        DECLARE_NAPI_FUNCTION("set", JSStorageSet),
        DECLARE_NAPI_FUNCTION("delete", JSStorageDelete),
        DECLARE_NAPI_FUNCTION("clear", JSStorageClear),
        DECLARE_NAPI_FUNCTION("getSync", JSStorageGetSync),
        DECLARE_NAPI_FUNCTION("setSync", JSStorageSetSync),
        DECLARE_NAPI_FUNCTION("deleteSync", JSStorageDeleteSync),
        DECLARE_NAPI_FUNCTION("clearSync", JSStorageClearSync),
        DECLARE_NAPI_FUNCTION("on", JSStorageOn),
        DECLARE_NAPI_FUNCTION("off", JSStorageOff),
    };
    napi_define_class(env, storageClassName, strlen(storageClassName), JSStorageConstructor, nullptr,
                      sizeof(storageDesc) / sizeof(storageDesc[0]), storageDesc, &storageClass);

    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("Storage", storageClass),
    };

    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

// storage module define
static napi_module storageModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = StorageExport,
    .nm_modname = "storage",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

// storage module register
extern "C" __attribute__((constructor)) void StorageRegister()
{
    napi_module_register(&storageModule);
}