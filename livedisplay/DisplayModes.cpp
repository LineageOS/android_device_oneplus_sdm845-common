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

#define LOG_TAG "DisplayModesService"

#include "DisplayModes.h"
#include <android-base/logging.h>
#include <fstream>

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

static constexpr const char* kModePath =
        "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/main_display/display_mode";
static constexpr const char* kDefaultPath = "/data/vendor/display/default_display_mode";

const std::map<int32_t, DisplayModes::ModeInfo> DisplayModes::kModeMap = {
        {0, {"Standard", "default"}},
        {1, {"sRGB", "srgb"}},
        {2, {"DCI P3", "dci-p3"}},
        {3, {"Wide Color", "widecolor"}},
};

DisplayModes::DisplayModes() : mDefaultModeId(0) {
    std::ifstream defaultFile(kDefaultPath);
    std::string value;

    defaultFile >> value;
    LOG(DEBUG) << "Default file read result " << value << " fail " << defaultFile.fail();
    if (defaultFile.fail()) {
        return;
    }

    for (const auto& entry : kModeMap) {
        if (value == entry.second.value) {
            mDefaultModeId = entry.first;
            break;
        }
    }

    setDisplayMode(mDefaultModeId, false);
}

// Methods from ::aidl::vendor::lineage::livedisplay::BnDisplayModes follow.
ndk::ScopedAStatus DisplayModes::getDisplayModes(std::vector<DisplayMode>* _aidl_return) {
    std::vector<DisplayMode> modes;
    for (const auto& entry : kModeMap) {
        modes.push_back({entry.first, entry.second.name});
    }
    *_aidl_return = modes;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus DisplayModes::getCurrentDisplayMode(DisplayMode* _aidl_return) {
    int32_t currentModeId = mDefaultModeId;
    std::ifstream modeFile(kModePath);
    std::string value;

    modeFile >> value;
    if (!modeFile.fail()) {
        for (const auto& entry : kModeMap) {
            if (value == entry.second.value) {
                currentModeId = entry.first;
                break;
            }
        }
    }
    *_aidl_return = {currentModeId, kModeMap.at(currentModeId).name};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus DisplayModes::getDefaultDisplayMode(DisplayMode* _aidl_return) {
    *_aidl_return = {mDefaultModeId, kModeMap.at(mDefaultModeId).name};
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus DisplayModes::setDisplayMode(int32_t modeID, bool makeDefault) {
    const auto iter = kModeMap.find(modeID);
    if (iter == kModeMap.end()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    std::ofstream modeFile(kModePath);
    modeFile << iter->second.value;
    if (modeFile.fail()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    if (makeDefault) {
        std::ofstream defaultFile(kDefaultPath);
        defaultFile << iter->second.value;
        if (defaultFile.fail()) {
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
        }
        mDefaultModeId = iter->first;
    }
    return ndk::ScopedAStatus::ok();
}

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
