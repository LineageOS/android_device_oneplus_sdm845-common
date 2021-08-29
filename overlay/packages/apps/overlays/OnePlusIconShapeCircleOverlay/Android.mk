LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_IS_RUNTIME_RESOURCE_OVERLAY := true

LOCAL_CERTIFICATE := platform

LOCAL_PACKAGE_NAME := OnePlusIconShapeCircleOverlay
LOCAL_SDK_VERSION := current

LOCAL_MODULE_PATH := $(TARGET_OUT_PRODUCT)/vendor_overlay/$(PRODUCT_TARGET_VNDK_VERSION)/overlay/oneplus_shape_circle

LOCAL_POST_INSTALL_CMD := mkdir -p $(TARGET_OUT_PRODUCT)/vendor_overlay/29/overlay/oneplus_shape_circle
LOCAL_POST_INSTALL_CMD += ; cp $(LOCAL_MODULE_PATH)/$(LOCAL_PACKAGE_NAME).apk $(TARGET_OUT_PRODUCT)/vendor_overlay/29/overlay/oneplus_shape_circle/$(LOCAL_PACKAGE_NAME).apk

include $(BUILD_PACKAGE)
