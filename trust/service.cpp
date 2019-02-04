/*
 * Copyright (C) 2019 The LineageOS Project
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

#define LOG_TAG "vendor.lineage.trust@1.0-service"

#include <android-base/logging.h>
#include <binder/ProcessState.h>
#include <hidl/HidlTransportSupport.h>

#include "UsbRestrict.h"

using android::OK;
using android::sp;
using android::status_t;

// libhwbinder:
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

using ::vendor::lineage::trust::V1_0::IUsbRestrict;
using ::vendor::lineage::trust::V1_0::implementation::UsbRestrict;

int main() {
    sp<IUsbRestrict> usbRestrict;
    status_t status;

    LOG(INFO) << "Trust HAL service is starting.";

    usbRestrict = new UsbRestrict();
    if (usbRestrict == nullptr) {
        LOG(ERROR) << "Can not create an instance of Trust HAL UsbRestricted Iface, exiting.";
        goto shutdown;
    }

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    status = usbRestrict->registerAsService();
    if (status != OK) {
        LOG(ERROR) << "Could not register service for Trust HAL UsbRestricted Iface (" << status
                   << ").";
    }

    LOG(INFO) << "Trust HAL service is ready.";
    joinRpcThreadpool();
    // Should not pass this line

shutdown:
    // In normal operation, we don't expect the thread pool to shutdown
    LOG(ERROR) << "Trust HAL service is shutting down.";
    return 1;
}
