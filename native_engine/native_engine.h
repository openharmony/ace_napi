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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H

#include <functional>
#include <string>
#include <unordered_set>
#include <vector>

#include "callback_scope_manager/native_callback_scope_manager.h"
#include "module_manager/native_module_manager.h"
#include "native_engine/native_async_work.h"
#include "native_engine/native_deferred.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_safe_async_work.h"
#include "native_engine/native_value.h"
#include "native_property.h"
#include "reference_manager/native_reference_manager.h"
#include "scope_manager/native_scope_manager.h"
#include "utils/macros.h"

typedef struct uv_loop_s uv_loop_t;

struct NativeErrorExtendedInfo {
    const char* message = nullptr;
    void* reserved = nullptr;
    uint32_t engineErrorCode = 0;
    int errorCode = 0;
};

struct ExceptionInfo {
    const char* message_ = nullptr;
    int32_t lineno_ = 0;
    int32_t colno_ = 0;

    ~ExceptionInfo()
    {
        if (message_ != nullptr) {
            delete[] message_;
        }
    }
};

enum LoopMode {
    LOOP_DEFAULT, LOOP_ONCE, LOOP_NOWAIT
};

class CleanupHookCallback {
public:
    using Callback = void (*)(void*);

    CleanupHookCallback(Callback fn, void* arg, uint64_t insertion_order_counter)
        : fn_(fn), arg_(arg), insertion_order_counter_(insertion_order_counter)
    {}

    struct Hash {
        inline size_t operator()(const CleanupHookCallback& cb) const
        {
            return std::hash<void*>()(cb.arg_);
        }
    };
    struct Equal {
        inline bool operator()(const CleanupHookCallback& a, const CleanupHookCallback& b) const
        {
            return a.fn_ == b.fn_ && a.arg_ == b.arg_;
        };
    };

private:
    friend class NativeEngine;
    Callback fn_;
    void* arg_;
    uint64_t insertion_order_counter_;
};

using PostTask = std::function<void(bool needSync)>;
using CleanEnv = std::function<void()>;
using InitWorkerFunc = std::function<void(NativeEngine* engine)>;
using GetAssetFunc = std::function<void(const std::string& uri, std::vector<uint8_t>& content)>;
using OffWorkerFunc = std::function<void(NativeEngine* engine)>;

class NAPI_EXPORT NativeEngine {
public:
    NativeEngine(void* jsEngine);
    virtual ~NativeEngine();

    virtual NativeScopeManager* GetScopeManager();
    virtual NativeModuleManager* GetModuleManager();
    virtual NativeReferenceManager* GetReferenceManager();
    virtual NativeCallbackScopeManager* GetCallbackScopeManager();
    virtual uv_loop_t* GetUVLoop() const;
    virtual pthread_t GetTid() const;

    virtual void Loop(LoopMode mode, bool needSync = false);
    virtual void SetPostTask(PostTask postTask);
    virtual void TriggerPostTask();
#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM)
    virtual void CheckUVLoop();
    virtual void CancelCheckUVLoop();
#endif
    virtual void* GetJsEngine();

    virtual NativeValue* GetGlobal() = 0;

    virtual NativeValue* CreateNull() = 0;
    virtual NativeValue* CreateUndefined() = 0;
    virtual NativeValue* CreateBoolean(bool value) = 0;
    virtual NativeValue* CreateNumber(int32_t value) = 0;
    virtual NativeValue* CreateNumber(uint32_t value) = 0;
    virtual NativeValue* CreateNumber(int64_t value) = 0;
    virtual NativeValue* CreateNumber(double value) = 0;
    virtual NativeValue* CreateBigInt(int64_t value) = 0;
    virtual NativeValue* CreateBigInt(uint64_t value) = 0;
    virtual NativeValue* CreateString(const char* value, size_t length) = 0;
    virtual NativeValue* CreateString16(const char16_t* value, size_t length) = 0;

    virtual NativeValue* CreateSymbol(NativeValue* value) = 0;
    virtual NativeValue* CreateExternal(void* value, NativeFinalize callback, void* hint) = 0;

    virtual NativeValue* CreateObject() = 0;
    virtual NativeValue* CreateFunction(const char* name, size_t length, NativeCallback cb, void* value) = 0;
    virtual NativeValue* CreateArray(size_t length) = 0;
    virtual NativeValue* CreateBuffer(void** value, size_t length) = 0;
    virtual NativeValue* CreateBufferCopy(void** value, size_t length, const void* data) = 0;
    virtual NativeValue* CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) = 0;
    virtual NativeValue* CreateArrayBuffer(void** value, size_t length) = 0;
    virtual NativeValue* CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) = 0;

    virtual NativeValue* CreateTypedArray(NativeTypedArrayType type,
                                          NativeValue* value,
                                          size_t length,
                                          size_t offset) = 0;
    virtual NativeValue* CreateDataView(NativeValue* value, size_t length, size_t offset) = 0;
    virtual NativeValue* CreatePromise(NativeDeferred** deferred) = 0;
    virtual void SetPromiseRejectCallback(NativeReference* rejectCallbackRef, NativeReference* checkCallbackRef) = 0;
    virtual NativeValue* CreateError(NativeValue* code, NativeValue* message) = 0;

    virtual NativeValue* CallFunction(NativeValue* thisVar,
                                      NativeValue* function,
                                      NativeValue* const* argv,
                                      size_t argc) = 0;
    virtual NativeValue* RunScript(NativeValue* script) = 0;
    virtual NativeValue* RunBufferScript(std::vector<uint8_t>& buffer) = 0;
    virtual NativeValue* DefineClass(const char* name,
                                     NativeCallback callback,
                                     void* data,
                                     const NativePropertyDescriptor* properties,
                                     size_t length) = 0;

    virtual NativeValue* CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc) = 0;

    virtual NativeReference* CreateReference(NativeValue* value, uint32_t initialRefcount,
        NativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr) = 0;

    virtual NativeAsyncWork* CreateAsyncWork(NativeValue* asyncResource,
                                             NativeValue* asyncResourceName,
                                             NativeAsyncExecuteCallback execute,
                                             NativeAsyncCompleteCallback complete,
                                             void* data);

    virtual NativeAsyncWork* CreateAsyncWork(NativeAsyncExecuteCallback execute,
                                             NativeAsyncCompleteCallback complete,
                                             void* data);
    virtual NativeSafeAsyncWork* CreateSafeAsyncWork(NativeValue* func, NativeValue* asyncResource,
        NativeValue* asyncResourceName, size_t maxQueueSize, size_t threadCount, void* finalizeData,
        NativeFinalize finalizeCallback, void* context, NativeThreadSafeFunctionCallJs callJsCallback);
    virtual void InitAsyncWork(NativeAsyncExecuteCallback execute, NativeAsyncCompleteCallback complete, void* data);
    virtual bool SendAsyncWork(void* data);
    virtual void CloseAsyncWork();

    virtual bool Throw(NativeValue* error) = 0;
    virtual bool Throw(NativeErrorType type, const char* code, const char* message) = 0;

    virtual void* CreateRuntime() = 0;
    virtual NativeValue* Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer) = 0;
    virtual NativeValue* Deserialize(NativeEngine* context, NativeValue* recorder) = 0;
    virtual ExceptionInfo* GetExceptionForWorker() const = 0;
    virtual void DeleteSerializationData(NativeValue* value) const = 0;
    virtual NativeValue* LoadModule(NativeValue* str, const std::string& fileName) = 0;

    virtual void StartCpuProfiler() = 0;
    virtual void StopCpuProfiler() = 0;

    NativeErrorExtendedInfo* GetLastError();
    void SetLastError(int errorCode, uint32_t engineErrorCode = 0, void* engineReserved = nullptr);
    void ClearLastError();
    bool IsExceptionPending() const;
    NativeValue* GetAndClearLastException();
    void EncodeToUtf8(NativeValue* nativeValue, char* buffer, int32_t* written, size_t bufferSize, int32_t* nchars);
    NativeEngine(NativeEngine&) = delete;
    virtual NativeEngine& operator=(NativeEngine&) = delete;

    virtual NativeValue* ValueToNativeValue(JSValueWrapper& value) = 0;
    virtual bool TriggerFatalException(NativeValue* error) = 0;
    virtual bool AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue) = 0;

    void MarkSubThread()
    {
        isMainThread_ = false;
    }

    bool IsMainThread() const
    {
        return isMainThread_;
    }

    void SetCleanEnv(CleanEnv cleanEnv)
    {
        cleanEnv_ = cleanEnv;
    }

    // register init worker func
    virtual void SetInitWorkerFunc(InitWorkerFunc func);
    virtual void SetGetAssetFunc(GetAssetFunc func);
    virtual void SetOffWorkerFunc(OffWorkerFunc func);
    virtual void SetWorkerAsyncWorkFunc(NativeAsyncExecuteCallback executeCallback,
                                NativeAsyncCompleteCallback completeCallback);
    // call init worker func
    virtual bool CallInitWorkerFunc(NativeEngine* engine);
    virtual bool CallGetAssetFunc(const std::string& uri, std::vector<uint8_t>& content);
    virtual bool CallOffWorkerFunc(NativeEngine* engine);
    virtual bool CallWorkerAsyncWorkFunc(NativeEngine* engine);

    virtual NativeValue* CreateDate(double value) = 0;
    virtual NativeValue* CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words) = 0;
    using CleanupCallback = CleanupHookCallback::Callback;
    virtual void AddCleanupHook(CleanupCallback fun, void* arg);
    virtual void RemoveCleanupHook(CleanupCallback fun, void* arg);

    void CleanupHandles();
    void IncreaseWaitingRequestCounter();
    void DecreaseWaitingRequestCounter();
    virtual void RunCleanup();

    bool IsStopping() const
    {
        return isStopping_.load();
    }
    
    void SetStopping(bool value)
    {
        isStopping_.store(value);
    }

protected:
    void Init();
    void Deinit();

    NativeModuleManager* moduleManager_ = nullptr;
    NativeScopeManager* scopeManager_ = nullptr;
    NativeReferenceManager* referenceManager_ = nullptr;
    NativeCallbackScopeManager* callbackScopeManager_ = nullptr;

    NativeErrorExtendedInfo lastError_;
    NativeValue* lastException_ = nullptr;

    uv_loop_t* loop_ = nullptr;

    void *jsEngine_;

private:
    bool isMainThread_ { true };

#if !defined(WINDOWS_PLATFORM) && !defined(MAC_PLATFORM)
    static void UVThreadRunner(void* nativeEngine);
    void PostLoopTask();

    bool checkUVLoop_ = false;
    uv_thread_t uvThread_;
#endif

    PostTask postTask_ = nullptr;
    CleanEnv cleanEnv_ = nullptr;
    uv_sem_t uvSem_;
    uv_async_t uvAsync_;
    std::unordered_set<CleanupHookCallback, CleanupHookCallback::Hash, CleanupHookCallback::Equal> cleanup_hooks_;
    uint64_t cleanup_hook_counter_ = 0;
    int request_waiting_ = 0;
    std::atomic_bool isStopping_ { false };
    pthread_t tid_ = 0;
    std::unique_ptr<NativeAsyncWork> asyncWorker_ {};
    // register for worker
    InitWorkerFunc initWorkerFunc_ {nullptr};
    GetAssetFunc getAssetFunc_ {nullptr};
    OffWorkerFunc offWorkerFunc_ {nullptr};
    NativeAsyncExecuteCallback nativeAsyncExecuteCallback_ {nullptr};
    NativeAsyncCompleteCallback nativeAsyncCompleteCallback_ {nullptr};
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_NATIVE_ENGINE_H */
