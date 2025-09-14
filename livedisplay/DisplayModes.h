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

#ifndef VENDOR_LINEAGE_LIVEDISPLAY_DISPLAYMODES_H
#define VENDOR_LINEAGE_LIVEDISPLAY_DISPLAYMODES_H

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

#endif  // VENDOR_LINEAGE_LIVEDISPLAY_DISPLAYMODES_H
