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

#ifndef VENDOR_LINEAGE_POCKETMODE_V1_0_FINGERPRINTDISABLER_H
#define VENDOR_LINEAGE_POCKETMODE_V1_0_FINGERPRINTDISABLER_H

#include <vendor/lineage/pocketmode/1.0/IFingerprintDisabler.h>

namespace vendor {
namespace lineage {
namespace pocketmode {
namespace V1_0 {
namespace implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

class FingerprintDisabler : public IFingerprintDisabler {
  public:
    // Methods from ::vendor::lineage::pocketmode::V1_0::IFingerprintDisabler follow.
    Return<bool> setEnabled(bool enabled) override;
    Return<void> getSensorType(getSensorType_cb _hidl_cb) override;
    Return<float> getSensorValue() override;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace pocketmode
}  // namespace lineage
}  // namespace vendor

#endif  // VENDOR_LINEAGE_POCKETMODE_V1_0_FINGERPRINTDISABLER_H
