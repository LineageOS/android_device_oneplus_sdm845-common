/*
 * Copyright (C) 2021 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "lineage.pocketmode@1.0-service.oneplus_sdm845"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>
#include "FingerprintDisabler.h"

using ::vendor::lineage::pocketmode::V1_0::IFingerprintDisabler;
using ::vendor::lineage::pocketmode::V1_0::implementation::FingerprintDisabler;

int main() {
    android::sp<IFingerprintDisabler> fingerprintDisabler = new FingerprintDisabler();

    android::hardware::configureRpcThreadpool(1, true /*callerWillJoin*/);

    if (fingerprintDisabler->registerAsService() != android::OK) {
        LOG(ERROR) << "Cannot register fingerprint disabler HAL service.";
        return 1;
    }

    LOG(INFO) << "PocketMode HAL service ready.";

    android::hardware::joinRpcThreadpool();

    LOG(ERROR) << "PocketMode HAL service failed to join thread pool.";
    return 1;
}
