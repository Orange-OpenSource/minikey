LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := minikey-common

LOCAL_SRC_FILES := \
	minikey.c \

LOCAL_STATIC_LIBRARIES := \
	libevdev \

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

# Enable PIE manually. Will get reset on $(CLEAR_VARS).
LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS += -fPIE -pie

LOCAL_MODULE := minikey

LOCAL_STATIC_LIBRARIES := minikey-common

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := minikey-nopie

LOCAL_STATIC_LIBRARIES := minikey-common

include $(BUILD_EXECUTABLE)
