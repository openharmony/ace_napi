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

#include "native_reference_manager.h"

struct NativeReferenceHandler {
    NativeReference* reference = nullptr;
    NativeReferenceHandler* next = nullptr;
};

NativeReferenceManager::NativeReferenceManager() : referenceHandlers_(nullptr) {}

NativeReferenceManager::~NativeReferenceManager()
{
    for (auto handler = referenceHandlers_; handler != nullptr; handler = referenceHandlers_) {
        referenceHandlers_ = handler->next;
        delete handler->reference;
        delete handler;
    }
}

void NativeReferenceManager::CreateHandler(NativeReference* reference)
{
    NativeReferenceHandler* temp = new NativeReferenceHandler();
    temp->reference = reference;
    temp->next = referenceHandlers_;
    referenceHandlers_ = temp;
}

void NativeReferenceManager::ReleaseHandler(NativeReference* reference)
{
    NativeReferenceHandler* tmp = nullptr;
    for (auto handler = referenceHandlers_; handler != nullptr; handler = handler->next) {
        if (handler->reference == reference) {
            if (tmp) {
                tmp->next = handler->next;
            } else {
                referenceHandlers_ = handler->next;
            }
            delete handler;
            break;
        }
        tmp = handler;
    }
}