/*
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
#ifndef AIDL_ANDROID_HARDWARE_LIGHT_LIGHTS_H
#define AIDL_ANDROID_HARDWARE_LIGHT_LIGHTS_H

#include <aidl/android/hardware/light/BnLights.h>
#include <functional>
#include <unordered_map>
#include <mutex>

namespace aidl {
namespace android {
namespace hardware {
namespace light {

using ::aidl::android::hardware::light::HwLightState;
using ::aidl::android::hardware::light::HwLight;

class Lights : public BnLights {
  public:
    Lights();

    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight> *_aidl_return) override;

  private:
    void handleRgb(const HwLightState& state, size_t index);

    std::mutex mLock;
    std::unordered_map<LightType, std::function<void(const HwLightState&)>> mLights;
    std::array<HwLightState, 3> mLightStates;
};

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl

#endif  // AIDL_ANDROID_HARDWARE_LIGHT_LIGHTS_H
