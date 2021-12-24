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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_V8_NATIVE_ENGINE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_V8_NATIVE_ENGINE_H

#include "v8_headers.h"

#include "native_engine/native_engine.h"

namespace {
const int MB = 1024 * 1024;
const int MAX_SERIALIZER_MEMORY_USAGE = 1 * MB;
} // namespace

#define DCHECK(condition)           assert(condition)
#define DCHECK_NOT_NULL(val) DCHECK((val) != nullptr)

#define DISALLOW_COPY_AND_ASSIGN(TypeName)         \
    TypeName(const TypeName&) = delete;            \
    TypeName& operator=(const TypeName&) = delete

class SerializationData {
public:
    SerializationData() : data_(nullptr), size_(0) {}
    ~SerializationData() = default;

    uint8_t* GetData() const
    {
        return data_.get();
    }
    size_t GetSize() const
    {
        return size_;
    }
    const std::vector<std::shared_ptr<v8::BackingStore>>& GetBackingStores()
    {
        return backingStores_;
    }

private:
    struct DataDeleter {
        void operator()(uint8_t* p) const
        {
            free(p);
        }
    };

    std::unique_ptr<uint8_t, DataDeleter> data_;
    size_t size_;
    std::vector<std::shared_ptr<v8::BackingStore>> backingStores_;

private:
    friend class Serializer;

    DISALLOW_COPY_AND_ASSIGN(SerializationData);
};

class WorkerIsolateScope {
public:
    WorkerIsolateScope() {}
    explicit WorkerIsolateScope(v8::Isolate* isolate) : isolate_(isolate) {}

    void SetIsolate(v8::Isolate* isolate)
    {
        isolate_ = isolate;
    }

    ~WorkerIsolateScope()
    {
        if (isolate_ != nullptr) {
            v8::Isolate::Scope iscope(isolate_);
            isolate_->LowMemoryNotification();
        }
    }
private:
    v8::Isolate* isolate_ { nullptr };
};

class V8NativeEngine : public NativeEngine {
public:
    // V8NativeEngine constructor
    V8NativeEngine(v8::Platform *platform, v8::Isolate* isolate, v8::Persistent<v8::Context>& context, void* jsEngine);
    // V8NativeEngine destructor
    virtual ~V8NativeEngine();

    virtual void Loop(LoopMode mode, bool needSync = false) override;

    v8::Isolate* GetIsolate();
    v8::Local<v8::Context> GetContext();
    void MarkAutoDispose()
    {
        workerIsolateScope_.SetIsolate(isolate_);
    }

    // Get global native object value
    virtual NativeValue* GetGlobal() override;
    // Create native null value
    virtual NativeValue* CreateNull() override;
    // Create native undefined value
    virtual NativeValue* CreateUndefined() override;
    // Create native boolean value
    virtual NativeValue* CreateBoolean(bool value) override;
    // Create number value by int32_t
    virtual NativeValue* CreateNumber(int32_t value) override;
    // Create number value by uint32_t
    virtual NativeValue* CreateNumber(uint32_t value) override;
    // Create native number value by int64_t
    virtual NativeValue* CreateNumber(int64_t value) override;
    // Create native number value by double
    virtual NativeValue* CreateNumber(double value) override;
    // Create native string value by const char pointer
    virtual NativeValue* CreateString(const char* value, size_t length) override;
    // Create native symbol value
    virtual NativeValue* CreateSymbol(NativeValue* value) override;
    // Create native value of external pointer
    virtual NativeValue* CreateExternal(void* value, NativeFinalize callback, void* hint) override;
    // Create native object value
    virtual NativeValue* CreateObject() override;
    // Create native function value
    virtual NativeValue* CreateFunction(const char* name, size_t length, NativeCallback cb, void* value) override;
    // Create native array value
    virtual NativeValue* CreateArray(size_t length) override;
    // Create native array buffer value
    virtual NativeValue* CreateArrayBuffer(void** value, size_t length) override;
    // Create native array buffer value of external
    virtual NativeValue* CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) override;
    // Create native typed array value
    virtual NativeValue* CreateTypedArray(NativeTypedArrayType type,
                                          NativeValue* value,
                                          size_t length,
                                          size_t offset) override;
    // Create native data view value
    virtual NativeValue* CreateDataView(NativeValue* value, size_t length, size_t offset) override;
    // Create native promise value
    virtual NativeValue* CreatePromise(NativeDeferred** deferred) override;
    virtual void SetPromiseRejectCallback(NativeReference* rejectCallbackRef,
                                          NativeReference* checkCallbackRef) override;
    static void PromiseRejectCallback(v8::PromiseRejectMessage message);
    // Create native error value
    virtual NativeValue* CreateError(NativeValue* code, NativeValue* message) override;
    // Call function
    virtual NativeValue* CallFunction(NativeValue* thisVar,
                                      NativeValue* function,
                                      NativeValue* const* argv,
                                      size_t argc) override;
    // Run script
    virtual NativeValue* RunScript(NativeValue* script) override;
    // Run buffer script
    virtual NativeValue* RunBufferScript(std::vector<uint8_t>& buffer) override;
    // Define native class
    virtual NativeValue* DefineClass(const char* name,
                                     NativeCallback callback,
                                     void* data,
                                     const NativePropertyDescriptor* properties,
                                     size_t length) override;

    virtual NativeAsyncWork* CreateAsyncWork(NativeValue* asyncResource,
                                             NativeValue* asyncResourceName,
                                             NativeAsyncExecuteCallback execute,
                                             NativeAsyncCompleteCallback complete,
                                             void* data) override;

    // Create instance by defined class
    virtual NativeValue* CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc) override;

    // Create native reference
    virtual NativeReference* CreateReference(NativeValue* value, uint32_t initialRefcount) override;
    // Throw exception
    virtual bool Throw(NativeValue* error) override;
    // Throw exception
    virtual bool Throw(NativeErrorType type, const char* code, const char* message) override;

    virtual void* CreateRuntime() override;
    virtual NativeValue* Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer) override;
    virtual NativeValue* Deserialize(NativeEngine* context, NativeValue* recorder) override;
    virtual ExceptionInfo* GetExceptionForWorker() const override;
    virtual void DeleteSerializationData(NativeValue* value) const override;
    void SetPackagePath(const std::string& packagePath);

    static NativeValue* V8ValueToNativeValue(V8NativeEngine* engine, v8::Local<v8::Value> value);
    virtual NativeValue* LoadModule(NativeValue* str, const std::string& fileName) override;
    virtual NativeValue* ValueToNativeValue(JSValueWrapper& value) override;

    v8::Local<v8::Object> GetModuleFromName(
        const std::string& moduleName, bool isAppModule, const std::string& id, const std::string& param,
        const std::string& instanceName, void** instance);
    v8::Local<v8::Object> LoadModuleByName(
        const std::string& moduleName, bool isAppModule, const std::string& param,
        const std::string& instanceName, void* instance);
    void StartCpuProfiler() override {}
    void StopCpuProfiler() override {}
private:
    static void ExecuteWrap(NativeEngine* engine, void* data);
    static void CompleteWrap(NativeEngine* engine, int status, void* data);

    v8::Platform* platform_;
    v8::Isolate* isolate_;
    WorkerIsolateScope workerIsolateScope_;
    v8::Global<v8::Context> context_;
    v8::Isolate::Scope isolateScope_;
    v8::HandleScope handleScope_;
    v8::Context::Scope contextScope_;
    v8::TryCatch tryCatch_ { NULL };
    NativeReference* promiseRejectCallbackRef_ { nullptr };
    NativeReference* checkCallbackRef_ { nullptr };
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_V8_NATIVE_ENGINE_H */
