/*
 * Copyright (C) 2019 The LineageOS Project
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
#ifndef VENDOR_LINEAGE_TRUST_V1_0_USBRESTRICT_H
#define VENDOR_LINEAGE_TRUST_V1_0_USBRESTRICT_H

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/lineage/trust/1.0/IUsbRestrict.h>

namespace vendor {
namespace lineage {
namespace trust {
namespace V1_0 {
namespace implementation {

using ::android::sp;
using ::android::hardware::Return;
using ::android::hardware::Void;

class UsbRestrict : public IUsbRestrict {
  public:
    UsbRestrict() = default;

    // Methods from ::vendor::lineage::trust::V1_0::IUsbRestrict follow.
    Return<bool> isEnabled() override;
    Return<void> setEnabled(bool enabled) override;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace trust
}  // namespace lineage
}  // namespace vendor

#endif  // VENDOR_LINEAGE_TRUST_V1_0_USBRESTRICT_H
