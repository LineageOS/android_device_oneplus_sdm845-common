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

#define LOG_TAG "FingerprintDisablerService"

#include "FingerprintDisabler.h"
#include <android-base/logging.h>
#include <fstream>

namespace vendor {
namespace lineage {
namespace pocketmode {
namespace V1_0 {
namespace implementation {

static constexpr const char* kProximityStatePath =
    "/sys/devices/platform/soc/soc:goodix_fp/proximity_state";

Return<bool> FingerprintDisabler::setEnabled(bool enabled) {
    std::ofstream file(kProximityStatePath);
    file << (enabled ? "1" : "0");
    LOG(DEBUG) << "setEnabled fail " << file.fail();
    return !file.fail();
}

Return<void> FingerprintDisabler::getSensorType(getSensorType_cb _hidl_cb) {
    _hidl_cb("oneplus.sensor.pocket");
    return Void();
}

Return<float> FingerprintDisabler::getSensorValue() {
    return 1.0f;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace pocketmode
}  // namespace lineage
}  // namespace vendor
