LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#MYCORE_DIR	:= $(LOCAL_PATH)/../../../../..
#
#include $(MYCORE_DIR)/nbpal/main/projects/android_ndk/jni/Android.mk
#include $(CLEAR_VARS)
#LOCAL_PATH := /cygdrive/c/p4_ccc_jni/client/c/core/coreservices/main/projects/android_ndk/jni
#$(warning Value of LOCAL_PATH is $(LOCAL_PATH))

LOCAL_MODULE := coreservices



# where header files are located.
PROJ_ROOT := $(LOCAL_PATH)/../../..
$(warning Value of PROJ_ROOT is $(PROJ_ROOT))
P4PATH :=    $(LOCAL_PATH)/../../../../../../../..
$(warning Value of P4PATH is $(P4PATH))
INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/protected
INCLUDE_DIRS += $(PROJ_ROOT)/include/android_ndk
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include
INCLUDE_DIRS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/include/android_ndk



LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_CFLAGS += $(foreach d,$(INCLUDE_DIRS),-I$(d))

LOCAL_LDFLAGS += $(P4PATH)/client/c/core/nbpal/$(BRANCH)/projects/android_ndk/libs/armeabi/libnbpal.so

SRCS := ../../../src/logging/logimpl.c  \
	../../../src/network/cslnetwork.c  \
	../../../src/qalog/cslqalog.c  \
	../../../src/qalog/cslqarecord.c  \
	../../../src/tps/tpsdebug.c  \
	../../../src/tps/tpseltgen.c  \
	../../../src/tps/tpseltmut.c  \
	../../../src/tps/tpslib_preload.c  \
	../../../src/tps/tpspack.c  \
	../../../src/tps/tpsunpack.c  \
	../../../src/util/bq.c  \
	../../../src/util/crc32.c  \
	../../../src/util/csdict.c  \
	../../../src/util/cslaes.c  \
	../../../src/util/cslcache.c  \
	../../../src/util/cslerrorutil.c  \
	../../../src/util/cslhashtable.c  \
	../../../src/util/cslquadkey.c  \
	../../../src/util/cslutil.c  \
	../../../src/util/cslversion.c  \
	../../../src/util/dynbuf.c  \
	../../../src/util/fileutil.c  \
	../../../src/util/fsalloc.c  \
	../../../src/util/heapsort.c  \
	../../../src/util/ht.c  \
	../../../src/util/inflate.c  \
	../../../src/util/intpack.c  \
	../../../src/util/sha1.c  \
	../../../src/util/vec.c  \
	../../../src/util/zinflate.c       
		
LOCAL_SRC_FILES = $(SRCS)

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
