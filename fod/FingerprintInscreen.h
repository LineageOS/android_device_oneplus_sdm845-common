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
#ifndef VENDOR_LINEAGE_BIOMETRICS_FINGERPRINT_INSCREEN_V1_0_FINGERPRINTINSCREEN_H
#define VENDOR_LINEAGE_BIOMETRICS_FINGERPRINT_INSCREEN_V1_0_FINGERPRINTINSCREEN_H

#include <vendor/lineage/biometrics/fingerprint/inscreen/1.0/IFingerprintInscreen.h>
#include <vendor/oneplus/fingerprint/extension/1.0/IVendorFingerprintExtensions.h>
#include <vendor/oneplus/hardware/display/1.0/IOneplusDisplay.h>

namespace vendor {
namespace lineage {
namespace biometrics {
namespace fingerprint {
namespace inscreen {
namespace V1_0 {
namespace implementation {

using ::android::sp;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::vendor::oneplus::fingerprint::extension::V1_0::IVendorFingerprintExtensions;
using ::vendor::oneplus::hardware::display::V1_0::IOneplusDisplay;

class FingerprintInscreen : public IFingerprintInscreen {
  public:
    FingerprintInscreen();

    Return<void> onStartEnroll() override;
    Return<void> onFinishEnroll() override;
    Return<void> onPress() override;
    Return<void> onRelease() override;
    Return<void> onShowFODView() override;
    Return<void> onHideFODView() override;
    Return<bool> shouldHandleError(int32_t error) override;
    Return<void> setLongPressEnabled(bool enabled) override;
    Return<int32_t> getDimAmount(int32_t cur_brightness) override;
    Return<bool> shouldBoostBrightness() override;
    Return<void> setCallback(const sp<IFingerprintInscreenCallback>& callback) override;

  private:
    sp<IOneplusDisplay> mVendorDisplayService;
    sp<IVendorFingerprintExtensions> mVendorFpService;

    std::mutex mCallbackLock;
    sp<IFingerprintInscreenCallback> mCallback;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace inscreen
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace lineage
}  // namespace vendor

#endif  // VENDOR_LINEAGE_BIOMETRICS_FINGERPRINT_INSCREEN_V1_0_FINGERPRINTINSCREEN_H
