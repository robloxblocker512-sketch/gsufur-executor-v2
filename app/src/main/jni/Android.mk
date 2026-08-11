LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := bgfx_hook
LOCAL_SRC_FILES := bgfx_hook.cpp
LOCAL_LDLIBS := -llog -ldl -pthread
LOCAL_CPPFLAGS := -std=c++11 -fexceptions
include $(BUILD_SHARED_LIBRARY)
