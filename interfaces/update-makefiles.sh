#!/bin/bash

source $ANDROID_BUILD_TOP/system/tools/hidl/update-makefiles-helper.sh

do_makefiles_update \
  "vendor.oneplus.hardware:device/oneplus/sdm845-common/interfaces/hardware"

do_makefiles_update \
  "vendor.oneplus.fingerprint:device/oneplus/sdm845-common/interfaces/fingerprint"
