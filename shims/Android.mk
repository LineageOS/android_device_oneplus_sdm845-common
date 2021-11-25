LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    jni_nativehelper.c

LOCAL_SHARED_LIBRARIES := libnativehelper
LOCAL_MODULE := libjni_shim
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
