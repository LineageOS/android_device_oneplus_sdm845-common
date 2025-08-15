/*
 * SPDX-FileCopyrightText: 2019-2025 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.lineage.livedisplay-service-oneplus_sdm845"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/ProcessState.h>
#include <livedisplay/oneplus/AntiFlicker.h>
#include <livedisplay/sdm/PictureAdjustment.h>
#include "DisplayModes.h"
#include "SunlightEnhancement.h"

using ::aidl::vendor::lineage::livedisplay::AntiFlicker;
using ::aidl::vendor::lineage::livedisplay::DisplayModes;
using ::aidl::vendor::lineage::livedisplay::SunlightEnhancement;
using ::aidl::vendor::lineage::livedisplay::sdm::PictureAdjustment;
using ::aidl::vendor::lineage::livedisplay::sdm::SDMController;

int main() {
    android::ProcessState::self()->setThreadPoolMaxThreadCount(1);
    android::ProcessState::self()->startThreadPool();

    LOG(INFO) << "LiveDisplay HAL service is starting.";

    std::shared_ptr<SDMController> controller = std::make_shared<SDMController>();

    std::shared_ptr<AntiFlicker> af = ndk::SharedRefBase::make<AntiFlicker>();
    std::shared_ptr<DisplayModes> dm = ndk::SharedRefBase::make<DisplayModes>();
    std::shared_ptr<PictureAdjustment> pa = ndk::SharedRefBase::make<PictureAdjustment>(controller);
    std::shared_ptr<SunlightEnhancement> se = ndk::SharedRefBase::make<SunlightEnhancement>();

    if (af) {
        std::string instance = std::string() + AntiFlicker::descriptor + "/default";
        binder_status_t status = AServiceManager_addService(af->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK);
    }

    if (dm) {
        std::string instance = std::string() + DisplayModes::descriptor + "/default";
        binder_status_t status = AServiceManager_addService(dm->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK);
    }

    if (pa) {
        std::string instance = std::string() + PictureAdjustment::descriptor + "/default";
        binder_status_t status = AServiceManager_addService(pa->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK);
    }

    if (se) {
        std::string instance = std::string() + SunlightEnhancement::descriptor + "/default";
        binder_status_t status = AServiceManager_addService(se->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK);
    }

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
