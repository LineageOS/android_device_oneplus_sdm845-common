/*
 * Copyright (C) 2019-2025 The LineageOS Project
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

#define LOG_TAG "vendor.lineage.livedisplay-service.oneplus_sdm845"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include "DisplayModes.h"

using ::aidl::vendor::lineage::livedisplay::DisplayModes;

int main() {
    android::ProcessState::self()->setThreadPoolMaxThreadCount(1);
    android::ProcessState::self()->startThreadPool();

    std::shared_ptr<DisplayModes> dm = ndk::SharedRefBase::make<DisplayModes>();

    std::string instance = std::string() + DisplayModes::descriptor + "/default";
    binder_status_t status = AServiceManager_addService(dm->asBinder().get(), instance.c_str());
    CHECK_EQ(status, STATUS_OK);

    LOG(INFO) << "LiveDisplay HAL service ready.";

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
