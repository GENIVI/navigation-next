LOCAL_PATH := $(call my-dir)
PROJ_ROOT := $(LOCAL_PATH)/../../..
P4PATH :=    $(LOCAL_PATH)/../../../../../../../..
include $(CLEAR_VARS)

LOCAL_MODULE := nbpal
#LOCAL_MODULE_FILENAME:= libnbpal
# where header files are located.

INCLUDE_DIRS := $(PROJ_ROOT)/include
INCLUDE_DIRS += $(PROJ_ROOT)/include/private
INCLUDE_DIRS += $(PROJ_ROOT)/include/android_ndk
INCLUDE_DIRS += $(PROJ_ROOT)/src/android_ndk/timer
INCLUDE_DIRS += $(PROJ_ROOT)/src/android_ndk/pal
INCLUDE_DIRS += $(PROJ_ROOT)/src/android_ndk/database
INCLUDE_DIRS += $(PROJ_ROOT)/src/android_ndk/net
INCLUDE_DIRS += $(PROJ_ROOT)/src/android_ndk/taskqueue
INCLUDE_DIRS += $(PROJ_ROOT)/src/android_ndk/util
INCLUDE_DIRS += $(PROJ_ROOT)/src/android_ndk
INCLUDE_DIRS += $(PROJ_ROOT)/src/common


LOCAL_CFLAGS := -DANDROID_NDK
LOCAL_CFLAGS += $(foreach d,$(INCLUDE_DIRS),-I$(d))


SRCS := ../../../src/android_ndk/clock/palclock.cpp  \
	../../../src/android_ndk/database/sqlite3.c  \
	../../../src/android_ndk/database/sqlite3_command.cpp  \
	../../../src/android_ndk/database/sqlite3_connection.cpp  \
	../../../src/android_ndk/database/sqlite3_reader.cpp  \
	../../../src/android_ndk/database/sqlite3_transaction.cpp  \
	../../../src/android_ndk/database/SqliteConnector.cpp  \
	../../../src/android_ndk/file/palfile.cpp  \
	../../../src/android_ndk/gl/palgl.c  \
	../../../src/android_ndk/net/netconnection.cpp  \
	../../../src/android_ndk/net/palnet.cpp  \
	../../../src/android_ndk/net/tcpconnection.cpp  \
	../../../src/android_ndk/net/httpconnection.cpp  \
	../../../src/android_ndk/net/httprequest.cpp  \
	../../../src/android_ndk/net/httpresponse.cpp  \
	../../../src/android_ndk/pal/finishfunctionbase.cpp  \
	../../../src/android_ndk/pal/finishfunctiondnsresult.cpp  \
	../../../src/android_ndk/pal/finishfunctionhttpdata.cpp  \
	../../../src/android_ndk/pal/finishfunctionmanager.cpp  \
	../../../src/android_ndk/pal/finishfunctionnetworkdata.cpp  \
	../../../src/android_ndk/pal/finishfunctionnetworkstatus.cpp  \
	../../../src/android_ndk/pal/finishfunctionhttpstatus.cpp  \
	../../../src/android_ndk/pal/finishfunctionnormal.cpp  \
	../../../src/android_ndk/pal/palabstractfactoryandroid.cpp  \
	../../../src/android_ndk/pal/palimpl.cpp  \
	../../../src/android_ndk/pal/wm_global.cpp  \
	../../../src/android_ndk/radio/palradio.cpp  \
	../../../src/android_ndk/synchronization/palevent.c  \
	../../../src/android_ndk/synchronization/pallock.cpp  \
	../../../src/android_ndk/synchronization/palatomic.cpp  \
	../../../src/android_ndk/taskqueue/paltaskqueue.cpp  \
	../../../src/android_ndk/taskqueue/threadandroid.cpp  \
	../../../src/android_ndk/timer/paltimer.cpp  \
	../../../src/android_ndk/timer/timerimpl.cpp  \
	../../../src/android_ndk/unzip/palunzip.cpp  \
	../../../src/android_ndk/unzip/quicksort.cpp  \
	../../../src/android_ndk/unzip/unzip.cpp  \
	../../../src/android_ndk/util/paldebuglog.c  \
	../../../src/android_ndk/util/paldisplay.c  \
	../../../src/android_ndk/util/palstdlib.c    \
	../../../src/common/task.cpp  \
	../../../src/common/taskmanager.cpp  \
	../../../src/common/taskmanagermultithreaded.cpp  \
	../../../src/common/taskmanagersinglethreaded.cpp  \
	../../../src/common/taskqueue.cpp  \
	../../../src/common/thread.cpp  \
	../../../src/util/bq.c  \
	../../../src/util/palstdlibcommon.c  \
	../../../src/util/palversion.c  \

LOCAL_SRC_FILES = $(SRCS)

$(warning Value of INCLUDE_DIRS is $(INCLUDE_DIRS))

LOCAL_LDLIBS := -lGLESv1_CM \
                -llog

include $(BUILD_SHARED_LIBRARY)
