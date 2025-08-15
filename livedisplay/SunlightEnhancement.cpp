/*
 * SPDX-FileCopyrightText: 2022-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "SunlightEnhancementService"

#include <android-base/logging.h>
#include <fstream>
#include "SunlightEnhancement.h"

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

static constexpr const char* kHbmPath =
    "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/main_display/hbm";

ndk::ScopedAStatus SunlightEnhancement::getEnabled(bool* _aidl_return) {
    std::ifstream file(kHbmPath);
    int result = -1;
    file >> result;
    LOG(DEBUG) << "Got result " << result << " fail " << file.fail();
    *_aidl_return = result > 0;
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus SunlightEnhancement::setEnabled(bool enabled) {
    std::ofstream file(kHbmPath);
    file << (enabled ? "3" : "0");
    LOG(DEBUG) << "setEnabled fail " << file.fail();
    return ndk::ScopedAStatus::ok();
}

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
