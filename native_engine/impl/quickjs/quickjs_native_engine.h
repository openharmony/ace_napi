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

#ifndef FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_QUICKJS_NATIVE_ENGINE_H
#define FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_QUICKJS_NATIVE_ENGINE_H

#include "native_engine/native_engine.h"
#include "quickjs_headers.h"

class SerializeData {
public:
    SerializeData(size_t size, uint8_t* data) : dataSize_(size), value_(data) {}
    ~SerializeData() = default;

    uint8_t* GetData() const
    {
        return value_.get();
    }
    size_t GetSize() const
    {
        return dataSize_;
    }

private:
    struct Deleter {
        void operator()(uint8_t* ptr) const
        {
            free(ptr);
        }
    };

    size_t dataSize_;
    std::unique_ptr<uint8_t, Deleter> value_;
};

class QuickJSNativeEngine : public NativeEngine {
public:
    QuickJSNativeEngine(JSRuntime* runtime, JSContext* contex, void* jsEngine);
    virtual ~QuickJSNativeEngine();

    JSRuntime* GetRuntime();
    JSContext* GetContext();

    virtual void Loop(LoopMode mode, bool needSync = false) override;

    virtual NativeValue* GetGlobal() override;
    virtual NativeValue* CreateNull() override;
    virtual NativeValue* CreateUndefined() override;
    virtual NativeValue* CreateBoolean(bool value) override;
    virtual NativeValue* CreateNumber(int32_t value) override;
    virtual NativeValue* CreateNumber(uint32_t value) override;
    virtual NativeValue* CreateNumber(int64_t value) override;
    virtual NativeValue* CreateNumber(double value) override;
    virtual NativeValue* CreateBigInt(int64_t value) override;
    virtual NativeValue* CreateBigInt(uint64_t value) override;
    virtual NativeValue* CreateString(const char* value, size_t length) override;
    virtual NativeValue* CreateString16(const char16_t* value, size_t length) override;
    virtual NativeValue* CreateSymbol(NativeValue* value) override;
    virtual NativeValue* CreateExternal(void* value, NativeFinalize callback, void* hint) override;

    virtual NativeValue* CreateObject() override;
    virtual NativeValue* CreateFunction(const char* name, size_t length, NativeCallback cb, void* value) override;
    virtual NativeValue* CreateArray(size_t length) override;

    virtual NativeValue* CreateArrayBuffer(void** value, size_t length) override;
    virtual NativeValue* CreateArrayBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) override;
    virtual NativeValue* CreateBuffer(void** value, size_t length) override;
    virtual NativeValue* CreateBufferCopy(void** value, size_t length, const void* data) override;
    virtual NativeValue* CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint) override;
    virtual NativeValue* CreateTypedArray(NativeTypedArrayType type,
                                          NativeValue* value,
                                          size_t length,
                                          size_t offset) override;
    virtual NativeValue* CreateDataView(NativeValue* value, size_t length, size_t offset) override;
    virtual NativeValue* CreatePromise(NativeDeferred** deferred) override;
    virtual void SetPromiseRejectCallback(NativeReference* rejectCallbackRef,
                                          NativeReference* checkCallbackRef) override;

    virtual NativeValue* CreateError(NativeValue* code, NativeValue* Message) override;
    virtual NativeValue* CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc) override;

    virtual NativeReference* CreateReference(NativeValue* value, uint32_t initialRefcount,
        NativeFinalize callback = nullptr, void* data = nullptr, void* hint = nullptr) override;
    virtual NativeValue* CallFunction(
        NativeValue* thisVar, NativeValue* function, NativeValue* const* argv, size_t argc) override;

    virtual NativeValue* DefineClass(const char* name, NativeCallback callback, void* data,
        const NativePropertyDescriptor* properties, size_t length) override;

    virtual NativeValue* RunScript(NativeValue* script) override;
    virtual NativeValue* RunBufferScript(std::vector<uint8_t>& buffer) override;

    void SetPackagePath(const std::string& packagePath);

    virtual bool Throw(NativeValue* error) override;
    virtual bool Throw(NativeErrorType type, const char* code, const char* message) override;

    virtual void* CreateRuntime() override;
    bool CheckTransferList(JSValue transferList);
    bool DetachTransferList(JSValue transferList);
    virtual NativeValue* Serialize(NativeEngine* context, NativeValue* value, NativeValue* transfer) override;
    virtual NativeValue* Deserialize(NativeEngine* context, NativeValue* recorder) override;
    virtual void DeleteSerializationData(NativeValue* value) const override;
    virtual ExceptionInfo* GetExceptionForWorker() const override;
    virtual NativeValue* LoadModule(NativeValue* str, const std::string& fileName) override;

    static NativeValue* JSValueToNativeValue(QuickJSNativeEngine* engine, JSValue value);
    virtual NativeValue* ValueToNativeValue(JSValueWrapper& value) override;
    JSValue GetModuleFromName(
        const std::string& moduleName, bool isAppModule, const std::string& id, const std::string& param,
        const std::string& instanceName, void** instance);
    JSValue LoadModuleByName(
        const std::string& moduleName, bool isAppModule, const std::string& param,
        const std::string& instanceName, void* instance);

    virtual NativeValue* CreateDate(double time) override;
    virtual NativeValue* CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words) override;
    virtual bool TriggerFatalException(NativeValue* error) override;
    virtual bool AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue) override;

    void StartCpuProfiler() override {}
    void StopCpuProfiler() override {}
private:
    JSRuntime* runtime_;
    JSContext* context_;
};

#endif /* FOUNDATION_ACE_NAPI_NATIVE_ENGINE_IMPL_QUICKJS_QUICKJS_NATIVE_ENGINE_H */
