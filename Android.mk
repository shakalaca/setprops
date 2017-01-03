LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := setprops
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CPPFLAGS := -std=c++11
LOCAL_CFLAGS := -Wno-implicit-exception-spec-mismatch
LOCAL_SRC_FILES := \
  bionic/libc_logging.cpp \
  system_properties_compat.c \
  system_properties.cpp \
  setprops.cpp
LOCAL_LDLIBS += -latomic
include $(BUILD_EXECUTABLE)
