LOCAL_PATH := $(call my-dir)

# Static library for the target. Used by update_engine_sideload from recovery.
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := bootable/recovery \
		    $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_CFLAGS += -Wall -Werror
LOCAL_SRC_FILES := gpt-utils.cpp sparse_crc32.cpp
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_MODULE := libgptutils
include $(BUILD_STATIC_LIBRARY)