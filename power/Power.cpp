/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "android.hardware.power@1.0-impl"

#include <fstream>

#include <log/log.h>

#include "Power.h"

namespace android {
namespace hardware {
namespace power {
namespace V1_0 {
namespace implementation {

Power::Power() {
    const hw_module_t* hw_module = nullptr;
    int err = hw_get_module(POWER_HARDWARE_MODULE_ID, &hw_module);
    if (err) {
        ALOGE("hw_get_module %s failed: %d", POWER_HARDWARE_MODULE_ID, err);
        return;
    }

    if (!hw_module->methods || !hw_module->methods->open) {
        mModule = reinterpret_cast<power_module_t*>(
            const_cast<hw_module_t*>(hw_module));
    } else {
        err = hw_module->methods->open(
            hw_module, POWER_HARDWARE_MODULE_ID,
            reinterpret_cast<hw_device_t**>(&mModule));
        if (err) {
            ALOGE("Passthrough failed to load legacy HAL.");
            return;
        }
    }

    if (mModule)
        mModule->init(mModule);
}

Power::~Power() {
    delete(mModule);
}

// Methods from ::android::hardware::power::V1_0::IPower follow.
Return<void> Power::setInteractive(bool interactive)  {
    if (mModule->setInteractive)
        mModule->setInteractive(mModule, interactive ? 1 : 0);
    return Void();
}

Return<void> Power::powerHint(PowerHint hint, int32_t data)  {
    int32_t param = data;
    if (mModule->powerHint) {
        if (data)
            mModule->powerHint(mModule, static_cast<power_hint_t>(hint), &param);
        else
            mModule->powerHint(mModule, static_cast<power_hint_t>(hint), NULL);
    }
    return Void();
}

Return<void> Power::setFeature(Feature feature, bool activate)  {
    if (feature == Feature::POWER_FEATURE_DOUBLE_TAP_TO_WAKE) {
        std::ofstream("/proc/touchpanel/double_tap_enable") << (activate ? 1 : 0);
    }
    return Void();
}

Return<void> Power::getPlatformLowPowerStats(getPlatformLowPowerStats_cb _hidl_cb)  {
    hidl_vec<PowerStatePlatformSleepState> states;
    _hidl_cb(states, Status::SUCCESS);
    return Void();
}

} // namespace implementation
}  // namespace V1_0
}  // namespace power
}  // namespace hardware
}  // namespace android
