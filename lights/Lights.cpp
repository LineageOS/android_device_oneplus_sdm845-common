/*
 * Copyright (C) 2014, 2017-2018 The  Linux Foundation. All rights reserved.
 * Not a contribution
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2018 The LineageOS Project
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

#define LOG_TAG "LightsService"

#include "Lights.h"
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <fstream>
#include <map>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

Lights::Lights() {
    mLights.emplace(LightType::ATTENTION, std::bind(&Lights::handleRgb, this, std::placeholders::_1, 0));
    mLights.emplace(LightType::BATTERY, std::bind(&Lights::handleRgb, this, std::placeholders::_1, 2));
    mLights.emplace(LightType::NOTIFICATIONS, std::bind(&Lights::handleRgb, this, std::placeholders::_1, 1));
}

void Lights::handleRgb(const HwLightState& state, size_t index) {
    mLightStates.at(index) = state;

    HwLightState stateToUse = mLightStates.front();
    for (const auto& lightState : mLightStates) {
        if (lightState.color & 0xffffff) {
            stateToUse = lightState;
            break;
        }
    }

    std::map<std::string, int> colorValues;
    colorValues["red"] = (stateToUse.color >> 16) & 0xff;
    // lower green and blue brightness to adjust for the (lower) brightness of red
    colorValues["green"] = ((stateToUse.color >> 8) & 0xff) / 2;
    colorValues["blue"] = (stateToUse.color & 0xff) / 2;

    int onMs = stateToUse.flashMode == FlashMode::TIMED ? stateToUse.flashOnMs : 0;
    int offMs = stateToUse.flashMode == FlashMode::TIMED ? stateToUse.flashOffMs : 0;

    // LUT has 63 entries, we could theoretically use them as 3 (colors) * 21 (steps).
    // However, the last LUT entries don't seem to behave correctly for unknown
    // reasons, so we use 17 steps for a total of 51 LUT entries only.
    static constexpr int kRampSteps = 16;
    static constexpr int kRampMaxStepDurationMs = 15;

    auto makeLedPath = [](const std::string& led, const std::string& op) -> std::string {
        return "/sys/class/leds/" + led + "/" + op;
    };
    auto getScaledDutyPercent = [](int brightness) -> std::string {
        std::string output;
        for (int i = 0; i <= kRampSteps; i++) {
            if (i != 0) {
                output += ",";
            }
            output += std::to_string(i * 512 * brightness / (255 * kRampSteps));
        }
        return output;
    };

    // Disable all blinking before starting
    for (const auto& entry : colorValues) {
        set(makeLedPath(entry.first, "blink"), 0);
    }

    if (onMs > 0 && offMs > 0) {
        int pauseLo, pauseHi, stepDuration, index = 0;
        if (kRampMaxStepDurationMs * kRampSteps > onMs) {
            stepDuration = onMs / kRampSteps;
            pauseHi = 0;
            pauseLo = offMs;
        } else {
            stepDuration = kRampMaxStepDurationMs;
            pauseHi = onMs - kRampSteps * stepDuration;
            pauseLo = offMs - kRampSteps * stepDuration;
        }

        for (const auto& entry : colorValues) {
            set(makeLedPath(entry.first, "lut_flags"), 95);
            set(makeLedPath(entry.first, "start_idx"), index);
            set(makeLedPath(entry.first, "duty_pcts"), getScaledDutyPercent(entry.second));
            set(makeLedPath(entry.first, "pause_lo"), pauseLo);
            set(makeLedPath(entry.first, "pause_hi"), pauseHi);
            set(makeLedPath(entry.first, "ramp_step_ms"), stepDuration);
            index += kRampSteps + 1;
        }

        // Start blinking
        for (const auto& entry : colorValues) {
            set(makeLedPath(entry.first, "blink"), entry.second);
        }
    } else {
        for (const auto& entry : colorValues) {
            set(makeLedPath(entry.first, "brightness"), entry.second);
        }
    }

    LOG(DEBUG) << ::android::base::StringPrintf(
        "handleRgb: mode=%d, color=%08X, onMs=%d, offMs=%d",
        static_cast<std::underlying_type<FlashMode>::type>(stateToUse.flashMode), stateToUse.color,
        onMs, offMs);
}

ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    auto it = mLights.find(static_cast<LightType>(id));

    if (it == mLights.end()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    /*
     * Lock global mutex until light state is updated.
     */
    std::lock_guard<std::mutex> lock(mLock);

    it->second(state);

    return ndk::ScopedAStatus::ok();
}

#define AutoHwLight(light) {.id = (int32_t)light, .ordinal = 0, .type = light}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight> *_aidl_return) {
    for (auto const& light : mLights) {
        _aidl_return->push_back(AutoHwLight(light.first));
    }

    return ndk::ScopedAStatus::ok();
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
