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

#include "Light.h"
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <fstream>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

namespace android {
namespace hardware {
namespace light {
namespace V2_0 {
namespace implementation {

/*
 * Write value to path and close file.
 */
template <typename T>
static void set(const std::string& path, const T& value) {
    std::ofstream file(path);
    file << value;
}

template <typename T>
static T get(const std::string& path, const T& def) {
    std::ifstream file(path);
    T result;

    file >> result;
    return file.fail() ? def : result;
}

static int rgbToBrightness(const LightState& state) {
    int color = state.color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0x00ff))
            + (150 * ((color >> 8) & 0x00ff))
            + (29 * (color & 0x00ff))) >> 8;
}

Light::Light() {
    mLights.emplace(Type::ATTENTION, std::bind(&Light::handleRgb, this, std::placeholders::_1, 0));
    mLights.emplace(Type::BACKLIGHT, std::bind(&Light::handleBacklight, this, std::placeholders::_1));
    mLights.emplace(Type::BATTERY, std::bind(&Light::handleRgb, this, std::placeholders::_1, 1));
    mLights.emplace(Type::NOTIFICATIONS, std::bind(&Light::handleRgb, this, std::placeholders::_1, 2));
}

void Light::handleBacklight(const LightState& state) {
    int maxBrightness = get("/sys/class/backlight/panel0-backlight/max_brightness", -1);
    if (maxBrightness < 0) {
        maxBrightness = 255;
    }
    int sentBrightness = rgbToBrightness(state);
    int brightness = sentBrightness * maxBrightness / 255;
    LOG(DEBUG) << "Writing backlight brightness " << brightness
               << " (orig " << sentBrightness << ")";
    set("/sys/class/backlight/panel0-backlight/brightness", brightness);
}

void Light::handleRgb(const LightState& state, size_t index) {
    mLightStates.at(index) = state;

    LightState stateToUse = mLightStates.front();
    for (const auto& lightState : mLightStates) {
        if (lightState.color & 0xffffff) {
            stateToUse = lightState;
            break;
        }
    }

    std::map<std::string, int> colorValues;
    colorValues["red"] = (stateToUse.color >> 16) & 0xff;
    colorValues["green"] = (stateToUse.color >> 8) & 0xff;
    colorValues["blue"] = stateToUse.color & 0xff;

    int onMs = stateToUse.flashMode == Flash::TIMED ? stateToUse.flashOnMs : 0;
    int offMs = stateToUse.flashMode == Flash::TIMED ? stateToUse.flashOffMs : 0;

    static constexpr int kRampSize = 8;
    static constexpr int kBrightnessRamp[] = { 0, 12, 25, 37, 50, 72, 85, 100 };
    static constexpr int kRampStepDuration = 50;

    auto makeLedPath = [](const std::string& led, const std::string& op) -> std::string {
        return "/sys/class/leds/" + led + "/" + op;
    };
    auto getScaledDutyPercent = [](int brightness) -> std::string {
        std::string output;
        for (size_t i = 0; i < ARRAY_SIZE(kBrightnessRamp); i++) {
            if (i != 0) {
                output += ",";
            }
            output += std::to_string(kBrightnessRamp[i] * brightness / 255);
        }
        return output;
    };

    // Disable all blinking before starting
    for (const auto& entry : colorValues) {
        set(makeLedPath(entry.first, "blink"), 0);
    }

    if (onMs > 0 && offMs > 0) {
        int pauseHi, stepDuration;
        if (kRampStepDuration * kRampSize * 2 > onMs) {
            stepDuration = onMs / 2 * kRampSize;
            pauseHi = 0;
        } else {
            stepDuration = kRampStepDuration;
            pauseHi = onMs - 2 * kRampSize * stepDuration;
        }

        for (const auto& entry : colorValues) {
            set(makeLedPath(entry.first, "start_idx"), 0);
            set(makeLedPath(entry.first, "duty_pcts"), getScaledDutyPercent(entry.second));
            set(makeLedPath(entry.first, "pause_lo"), offMs);
            // The led driver is configured to ramp up then ramp
            // down the lut. This effectively doubles the ramp duration.
            set(makeLedPath(entry.first, "pause_hi"), pauseHi);
            set(makeLedPath(entry.first, "ramp_step_ms"), stepDuration);
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

    LOG(DEBUG) << base::StringPrintf(
        "handleRgb: mode=%d, color=%08X, onMs=%d, offMs=%d",
        static_cast<std::underlying_type<Flash>::type>(stateToUse.flashMode), stateToUse.color,
        onMs, offMs);
}

Return<Status> Light::setLight(Type type, const LightState& state) {
    auto it = mLights.find(type);

    if (it == mLights.end()) {
        return Status::LIGHT_NOT_SUPPORTED;
    }

    /*
     * Lock global mutex until light state is updated.
     */
    std::lock_guard<std::mutex> lock(mLock);

    it->second(state);

    return Status::SUCCESS;
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb _hidl_cb) {
    std::vector<Type> types;

    for (auto const& light : mLights) {
        types.push_back(light.first);
    }

    _hidl_cb(types);

    return Void();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace light
}  // namespace hardware
}  // namespace android
