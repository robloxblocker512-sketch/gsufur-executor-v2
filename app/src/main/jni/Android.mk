LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := bgfx_hook
LOCAL_SRC_FILES := bgfx_hook.c
LOCAL_LDLIBS := -llog -ldl
LOCAL_CFLAGS := -std=c11
include $(BUILD_SHARED_LIBRARY)
