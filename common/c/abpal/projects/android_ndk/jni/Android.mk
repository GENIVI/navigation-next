LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := abpal

# where header files are located.
PROJ_ROOT := $(LOCAL_PATH)/../../..
$(warning Value of PROJ_ROOT is $(PROJ_ROOT))
P4PATH :=    $(LOCAL_PATH)/../../../../../../../..
$(warning Value of P4PATH is $(P4PATH))
INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include/android_ndk

$(warning Value of BRANCH is $(BRANCH))


LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_CFLAGS += $(foreach d,$(INCLUDE_DIRS),-I$(d))

	
SRCS := ../../../src/android_ndk/audio/palaudio.c  \
		../../../src/android_ndk/gps/palgps.c  \

LOCAL_SRC_FILES = $(SRCS)

include $(BUILD_SHARED_LIBRARY)

