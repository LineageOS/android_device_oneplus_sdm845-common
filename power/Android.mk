LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := power.qcom
LOCAL_MODULE_PATH_32 := $(TARGET_OUT_PRODUCT)/vendor_overlay/$(PRODUCT_TARGET_VNDK_VERSION)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_PRODUCT)/vendor_overlay/$(PRODUCT_TARGET_VNDK_VERSION)/lib64
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    hint-data.c \
    list.c \
    metadata-parser.c \
    power-845.c \
    power.c \
    powerhintparser.c \
    utils.c

LOCAL_C_INCLUDES := external/libxml2/include \
                    external/icu/icu4c/source/common

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libxml2

LOCAL_CFLAGS += -Werror -Wall -Wno-unused-parameter
LOCAL_CFLAGS += -DINTERACTION_BOOST

LOCAL_POST_INSTALL_CMD := mkdir -p $(TARGET_OUT_PRODUCT)/vendor_overlay/29/lib/hw $(TARGET_OUT_PRODUCT)/vendor_overlay/29/lib64/hw
LOCAL_POST_INSTALL_CMD += ; cp $(LOCAL_MODULE_PATH_32)/$(LOCAL_MODULE_RELATIVE_PATH)/$(LOCAL_MODULE).so $(TARGET_OUT_PRODUCT)/vendor_overlay/29/lib/$(LOCAL_MODULE_RELATIVE_PATH)/$(LOCAL_MODULE).so
LOCAL_POST_INSTALL_CMD += ; cp $(LOCAL_MODULE_PATH_64)/$(LOCAL_MODULE_RELATIVE_PATH)/$(LOCAL_MODULE).so $(TARGET_OUT_PRODUCT)/vendor_overlay/29/lib64/$(LOCAL_MODULE_RELATIVE_PATH)/$(LOCAL_MODULE).so

include $(BUILD_SHARED_LIBRARY)
