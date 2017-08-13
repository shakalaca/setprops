LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := setprops
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/async_safe/include
LOCAL_CPPFLAGS := -std=c++11
LOCAL_CFLAGS := -Wno-implicit-exception-spec-mismatch
LOCAL_SRC_FILES := \
  bionic/android_set_abort_message.cpp \
  bionic/system_properties.cpp \
  bionic/system_properties_compat.c \
  async_safe/async_safe_log.cpp \
  setprops.cpp
LOCAL_LDLIBS += -latomic
include $(BUILD_EXECUTABLE)
