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

#include "jerryscript_native_engine.h"

#include "jerryscript-ext/handler.h"
#include "jerryscript_native_deferred.h"
#include "jerryscript_native_reference.h"

#include "native_value/jerryscript_native_array.h"
#include "native_value/jerryscript_native_array_buffer.h"
#include "native_value/jerryscript_native_big_int.h"
#include "native_value/jerryscript_native_boolean.h"
#include "native_value/jerryscript_native_buffer.h"
#include "native_value/jerryscript_native_data_view.h"
#include "native_value/jerryscript_native_date.h"
#include "native_value/jerryscript_native_external.h"
#include "native_value/jerryscript_native_function.h"
#include "native_value/jerryscript_native_number.h"
#include "native_value/jerryscript_native_object.h"
#include "native_value/jerryscript_native_string.h"
#include "native_value/jerryscript_native_typed_array.h"
#include "utils/log.h"

JerryScriptNativeEngine::JerryScriptNativeEngine(void* jsEngine) : NativeEngine(jsEngine)
{
    HILOG_INFO("JerryScriptNativeEngine::JerryScriptNativeEngine begin");
    jerry_add_external();
    jerry_value_t global = jerry_get_global_object();
    jerry_value_t require = jerry_create_external_function([](const jerry_value_t function, const jerry_value_t thisVal,
                                                               const jerry_value_t args[],
                                                               const jerry_length_t argc) -> jerry_value_t {
        JerryScriptNativeEngine* that = nullptr;
        jerry_get_object_native_pointer(function, (void**)&that, nullptr);
        jerry_value_t result = jerry_create_undefined();

        if (!(argc >= 1 && jerry_value_is_string(args[0]))) {
            return result;
        }

        jerry_size_t moduleNameSize = jerry_get_utf8_string_size(args[0]);

        if (moduleNameSize == 0) {
            return result;
        }

        char* moduleName = new char[moduleNameSize + 1] { 0 };
        uint32_t moduleNameLength = jerry_string_to_char_buffer(args[0], (jerry_char_t*)moduleName, moduleNameSize + 1);
        moduleName[moduleNameLength] = '\0';
        NativeModule* module = that->GetModuleManager()->LoadNativeModule(moduleName, nullptr, false);

        if (module != nullptr) {
            NativeValue* value = that->CreateObject();
            module->registerCallback(that, value);
            result = jerry_acquire_value(*value);
        }
        return result;
    });
    jerry_set_object_native_pointer(require, this, nullptr);
    jerryx_set_property_str(global, "requireNapi", require);

    jerry_release_value(require);
    jerry_release_value(global);
    HILOG_INFO("JerryScriptNativeEngine::JerryScriptNativeEngine end");
    Init();
}

JerryScriptNativeEngine::~JerryScriptNativeEngine()
{
    Deinit();
}

void JerryScriptNativeEngine::Loop(LoopMode mode, bool needSync)
{
    NativeEngine::Loop(mode, needSync);
    jerry_value_t retVal = jerry_run_all_enqueued_jobs();
    jerry_release_value(retVal);
}

NativeValue* JerryScriptNativeEngine::GetGlobal()
{
    return new JerryScriptNativeObject(this, jerry_get_global_object());
}

NativeValue* JerryScriptNativeEngine::CreateNull()
{
    return new JerryScriptNativeValue(this, jerry_create_null());
}

NativeValue* JerryScriptNativeEngine::CreateUndefined()
{
    return new JerryScriptNativeValue(this, jerry_create_undefined());
}

NativeValue* JerryScriptNativeEngine::CreateBoolean(bool value)
{
    return new JerryScriptNativeBoolean(this, value);
}

NativeValue* JerryScriptNativeEngine::CreateNumber(int32_t value)
{
    return new JerryScriptNativeNumber(this, (double)value);
}

NativeValue* JerryScriptNativeEngine::CreateNumber(uint32_t value)
{
    return new JerryScriptNativeNumber(this, (double)value);
}

NativeValue* JerryScriptNativeEngine::CreateNumber(int64_t value)
{
    return new JerryScriptNativeNumber(this, (double)value);
}

NativeValue* JerryScriptNativeEngine::CreateNumber(double value)
{
    return new JerryScriptNativeNumber(this, (double)value);
}

NativeValue* JerryScriptNativeEngine::CreateString(const char* value, size_t length)
{
    return new JerryScriptNativeString(this, value, length);
}

NativeValue* JerryScriptNativeEngine::CreateSymbol(NativeValue* value)
{
    return new JerryScriptNativeValue(this, jerry_create_symbol(*value));
}

NativeValue* JerryScriptNativeEngine::CreateExternal(void* value, NativeFinalize callback, void* hint)
{
    return new JerryScriptNativeExternal(this, value, callback, hint);
}

NativeValue* JerryScriptNativeEngine::CreateObject()
{
    return new JerryScriptNativeObject(this);
}

NativeValue* JerryScriptNativeEngine::CreateFunction(const char* name, size_t length, NativeCallback cb, void* value)
{
    return new JerryScriptNativeFunction(this, name, cb, value);
}

NativeValue* JerryScriptNativeEngine::CreateArray(size_t length)
{
    return new JerryScriptNativeArray(this, (int)length);
}

NativeValue* JerryScriptNativeEngine::CreateArrayBuffer(void** value, size_t length)
{
    return new JerryScriptNativeArrayBuffer(this, value, length);
}

NativeValue* JerryScriptNativeEngine::CreateArrayBufferExternal(
    void* value, size_t length, NativeFinalize cb, void* hint)
{
    return new JerryScriptNativeArrayBuffer(this, (unsigned char*)value, length, cb, hint);
}

NativeValue* JerryScriptNativeEngine::CreateBuffer(void** value, size_t length)
{
    return new JerryScriptNativeBuffer(this, (uint8_t**)value, length);
}

NativeValue* JerryScriptNativeEngine::CreateBufferCopy(void** value, size_t length, const void* data)
{
    return new JerryScriptNativeBuffer(this, (uint8_t**)value, length, (uint8_t*)data);
}

NativeValue* JerryScriptNativeEngine::CreateBufferExternal(void* value, size_t length, NativeFinalize cb, void* hint)
{
    return new JerryScriptNativeBuffer(this, (uint8_t*)value, length, cb, hint);
}

NativeValue* JerryScriptNativeEngine::CreateTypedArray(
    NativeTypedArrayType type, NativeValue* value, size_t length, size_t offset)
{
    return new JerryScriptNativeTypedArray(this, type, value, length, offset);
}

NativeValue* JerryScriptNativeEngine::CreateDataView(NativeValue* value, size_t length, size_t offset)
{
    return new JerryScriptNativeDataView(this, value, length, offset);
}

NativeValue* JerryScriptNativeEngine::CreatePromise(NativeDeferred** deferred)
{
    jerry_value_t promise = jerry_create_promise();
    *deferred = new JerryScriptNativeDeferred(promise);
    return new JerryScriptNativeValue(this, promise);
}

NativeValue* JerryScriptNativeEngine::CreateError(NativeValue* code, NativeValue* message)
{
    jerry_value_t jerror = 0;

    jerror = jerry_create_error_sz(JERRY_ERROR_COMMON, nullptr, 0);
    jerror = jerry_get_value_from_error(jerror, true);

    if (message) {
        jerry_value_t jreturn = jerryx_set_property_str(jerror, "message", *message);
        jerry_release_value(jreturn);
    }
    if (code) {
        jerry_value_t jreturn = jerryx_set_property_str(jerror, "code", *code);
        jerry_release_value(jreturn);
    }
    jerror = jerry_create_error_from_value(jerror, true);

    return new JerryScriptNativeObject(this, jerror);
}

NativeValue* JerryScriptNativeEngine::CallFunction(
    NativeValue* thisVar, NativeValue* function, NativeValue* const* argv, size_t argc)
{
    jerry_value_t* args = nullptr;
    if (argc > 0) {
        args = new jerry_value_t[argc];
        for (size_t i = 0; i < argc; i++) {
            if (argv[i] == nullptr) {
                args[i] = jerry_create_undefined();
            } else {
                args[i] = *argv[i];
            }
        }
    }
    NativeScope* scope = scopeManager_->Open();
    jerry_value_t result = jerry_call_function(*function, thisVar ? *thisVar : 0, (const jerry_value_t*)args, argc);
    scopeManager_->Close(scope);
    if (args != nullptr) {
        delete[] args;
    }

    if (jerry_value_is_error(result)) {
        jerry_value_t errorObj = jerry_get_value_from_error(result, true);
        jerry_value_t propName = jerry_create_string_from_utf8((const jerry_char_t*)"message");
        jerry_property_descriptor_t propDescriptor = { 0 };
        jerry_get_own_property_descriptor(errorObj, propName, &propDescriptor);
        jerry_value_t setResult = jerry_set_property(errorObj, propName, propDescriptor.value);
        jerry_release_value(propName);
        jerry_release_value(setResult);
        Throw(JerryValueToNativeValue(this, errorObj));
        return JerryValueToNativeValue(this, jerry_create_undefined());
    } else {
        return JerryValueToNativeValue(this, result);
    }
}

NativeValue* JerryScriptNativeEngine::RunScript(NativeValue* script)
{
    NativeString* pscript = (NativeString*)script->GetInterface(NativeString::INTERFACE_ID);

    size_t length = pscript->GetLength();
    if (length == 0) {
        return nullptr;
    }
    char* strScript = new char[length] { 0 };
    pscript->GetCString(strScript, length, &length);
    jerry_value_t result = jerry_eval((const unsigned char*)strScript, pscript->GetLength(), JERRY_PARSE_NO_OPTS);
    if (jerry_value_is_error(result)) {
        result = jerry_get_value_from_error(result, true);
    }
    delete[] strScript;
    return JerryValueToNativeValue(this, result);
}

NativeValue* JerryScriptNativeEngine::RunBufferScript(std::vector<uint8_t>& buffer)
{
    return nullptr;
}

NativeValue* JerryScriptNativeEngine::DefineClass(
    const char* name, NativeCallback callback, void* data, const NativePropertyDescriptor* properties, size_t length)
{
    auto classConstructor = new JerryScriptNativeFunction(this, name, callback, data);
    auto classProto = new JerryScriptNativeObject(this);

    jerryx_set_property_str(*classConstructor, "prototype", *classProto);

    for (size_t i = 0; i < length; ++i) {
        if (properties[i].attributes & NATIVE_STATIC) {
            classConstructor->DefineProperty(properties[i]);
        } else {
            classProto->DefineProperty(properties[i]);
        }
    }
    return classConstructor;
}

NativeValue* JerryScriptNativeEngine::CreateInstance(NativeValue* constructor, NativeValue* const* argv, size_t argc)
{
    return JerryValueToNativeValue(this, jerry_construct_object(*constructor, (const jerry_value_t*)argv, argc));
}

NativeReference* JerryScriptNativeEngine::CreateReference(NativeValue* value, uint32_t initialRefcount,
    NativeFinalize callback, void* data, void* hint)
{
    return new JerryScriptNativeReference(this, value, initialRefcount, callback, data, hint);
}

bool JerryScriptNativeEngine::Throw(NativeValue* error)
{
    this->lastException_ = error;
    return true;
}

bool JerryScriptNativeEngine::Throw(NativeErrorType type, const char* code, const char* message)
{
    jerry_value_t jerror = 0;
    jerry_error_t jtype;
    switch (type) {
        case NATIVE_COMMON_ERROR:
            jtype = JERRY_ERROR_COMMON;
            break;
        case NATIVE_TYPE_ERROR:
            jtype = JERRY_ERROR_TYPE;
            break;
        case NATIVE_RANGE_ERROR:
            jtype = JERRY_ERROR_RANGE;
            break;
        default:
            return false;
    }
    jerror = jerry_create_error(jtype, (const unsigned char*)message);
    jerror = jerry_get_value_from_error(jerror, true);
    if (code) {
        jerry_value_t jcode = jerry_create_string_from_utf8((const unsigned char*)code);
        jerryx_set_property_str(jerror, "code", jcode);
    }
    jerror = jerry_create_error_from_value(jerror, true);
    this->lastException_ = new JerryScriptNativeObject(this, jerror);
    return true;
}

void* JerryScriptNativeEngine::CreateRuntime()
{
    return nullptr;
}

NativeValue* JerryScriptNativeEngine::Serialize(NativeEngine* context, NativeValue* value,
    NativeValue* transfer)
{
    return nullptr;
}

NativeValue* JerryScriptNativeEngine::Deserialize(NativeEngine* context, NativeValue* recorder)
{
    return nullptr;
}

ExceptionInfo* JerryScriptNativeEngine::GetExceptionForWorker() const
{
    return nullptr;
}

NativeValue* JerryScriptNativeEngine::LoadModule(NativeValue* str, const std::string& fileName)
{
    return nullptr;
}

NativeValue* JerryScriptNativeEngine::JerryValueToNativeValue(JerryScriptNativeEngine* engine, jerry_value_t value)
{
    NativeValue* result = nullptr;
    switch (jerry_value_get_type(value)) {
        case JERRY_TYPE_NONE:
            result = new JerryScriptNativeValue(engine, value);
            break;
        case JERRY_TYPE_UNDEFINED:
            result = new JerryScriptNativeValue(engine, value);
            break;
        case JERRY_TYPE_NULL:
            result = new JerryScriptNativeValue(engine, value);
            break;
        case JERRY_TYPE_BOOLEAN:
            result = new JerryScriptNativeBoolean(engine, value);
            break;
        case JERRY_TYPE_NUMBER:
            result = new JerryScriptNativeNumber(engine, value);
            break;
        case JERRY_TYPE_STRING:
            result = new JerryScriptNativeString(engine, value);
            break;
        case JERRY_TYPE_OBJECT:
            if (jerry_value_is_array(value)) {
                result = new JerryScriptNativeArray(engine, value);
            } else if (jerry_value_is_arraybuffer(value)) {
                result = new JerryScriptNativeArrayBuffer(engine, value);
            } else if (jerry_value_is_dataview(value)) {
                result = new JerryScriptNativeDataView(engine, value);
            } else if (jerry_value_is_typedarray(value)) {
                result = new JerryScriptNativeTypedArray(engine, value);
            } else if (jerry_value_is_external(value)) {
                result = new JerryScriptNativeExternal(engine, value);
            } else if (jerry_is_date(value)) {
                result = new JerryScriptNativeDate(engine, value);
            } else {
                result = new JerryScriptNativeObject(engine, value);
            }
            break;
        case JERRY_TYPE_FUNCTION:
            result = new JerryScriptNativeFunction(engine, value);
            break;
        case JERRY_TYPE_ERROR:
            result = new JerryScriptNativeObject(engine, value);
            break;
        case JERRY_TYPE_SYMBOL:
            result = new JerryScriptNativeValue(engine, value);
            break;
        case JERRY_TYPE_BIGINT:
#if JERRY_API_MINOR_VERSION > 3
                result = new JerryScriptNativeBigInt(engine, value);
                break;
#endif
        default:;
    }
    return result;
}

NativeValue* JerryScriptNativeEngine::ValueToNativeValue(JSValueWrapper& value)
{
    jerry_value_t jerryValue = value;
    return JerryValueToNativeValue(this, jerryValue);
}

bool JerryScriptNativeEngine::TriggerFatalException(NativeValue* error)
{

    return false;
}

bool JerryScriptNativeEngine::AdjustExternalMemory(int64_t ChangeInBytes, int64_t* AdjustedValue)
{
    HILOG_INFO("L1: napi_adjust_external_memory not supported!");
    return true;
}

NativeValue* JerryScriptNativeEngine::CreateDate(double time)
{
    jerry_value_t value = jerry_strict_date(time);
    return JerryValueToNativeValue(this, value);
}

void JerryScriptNativeEngine::SetPromiseRejectCallback(NativeReference* rejectCallbackRef,
                                                       NativeReference* checkCallbackRef) {}


NativeValue* JerryScriptNativeEngine::CreateBigWords(int sign_bit, size_t word_count, const uint64_t* words)
{
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    constexpr int bigintMod = 2;
    bool sign = false;
    if ((sign_bit % bigintMod) == 1) {
        sign = true;
    }
    uint32_t size = (uint32_t)word_count;

    jerry_value_t jerryValue = jerry_create_bigint(words, size, sign);

    return new JerryScriptNativeBigInt(this, jerryValue);
#else
    return nullptr;
#endif
}

NativeValue* JerryScriptNativeEngine::CreateBigInt(int64_t value)
{
    return new JerryScriptNativeBigInt(this, value);
}

NativeValue* JerryScriptNativeEngine::CreateBigInt(uint64_t value)
{
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    return new JerryScriptNativeBigInt(this, value, true);
#else
    return nullptr;
#endif
}

NativeValue* JerryScriptNativeEngine::CreateString16(const char16_t* value, size_t length)
{
#if JERRY_API_MINOR_VERSION > 3 // jerryscript2.3: 3,  jerryscript2.4: 4
    return new JerryScriptNativeString(this, value, length);
#else
    return nullptr;
#endif
}
