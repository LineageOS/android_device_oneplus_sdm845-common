/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <aidl/vendor/lineage/livedisplay/BnDisplayModes.h>
#include <map>

namespace aidl {
namespace vendor {
namespace lineage {
namespace livedisplay {

class DisplayModes : public BnDisplayModes {
  public:
    DisplayModes();

    // Methods from ::aidl::vendor::lineage::livedisplay::BnDisplayModes follow.
    ndk::ScopedAStatus getDisplayModes(std::vector<DisplayMode>* _aidl_return) override;
    ndk::ScopedAStatus getCurrentDisplayMode(DisplayMode* _aidl_return) override;
    ndk::ScopedAStatus getDefaultDisplayMode(DisplayMode* _aidl_return) override;
    ndk::ScopedAStatus setDisplayMode(int32_t modeID, bool makeDefault) override;

  private:
    struct ModeInfo {
        const char* name;
        const char* value;
    };
    static const std::map<int32_t, ModeInfo> kModeMap;
    int32_t mDefaultModeId;
};

}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
}  // namespace aidl
