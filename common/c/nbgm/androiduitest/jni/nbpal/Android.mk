LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#header file
PROJECT_ROOT := $(LOCAL_PATH)/../../../../..
NIMNBIPAL_HOME := $(PROJECT_ROOT)/nbpal/$(BRANCH)
NIMTHIRDPARTY_HOME = $(PROJECT_ROOT)/thirdparty/$(BRANCH)


INCLUDE_DIRS := $(NIMNBIPAL_HOME)/include \
                $(NIMNBIPAL_HOME)/include/protected \
                $(NIMNBIPAL_HOME)/include/private \
                $(NIMNBIPAL_HOME)/include/android_ndk \
                $(NIMNBIPAL_HOME)/src/android_ndk \
                $(NIMNBIPAL_HOME)/src/android_ndk/pal \
                $(NIMNBIPAL_HOME)/src/android_ndk/taskqueue \
                $(NIMNBIPAL_HOME)/src/android_ndk/util \
                $(NIMNBIPAL_HOME)/src/posix/util \
                $(NIMNBIPAL_HOME)/src/common \
                $(NIMNBIPAL_HOME)/src/common/database \
                $(NIMNBIPAL_HOME)/src/common/finishfunction \
                $(NIMNBIPAL_HOME)/src/common/taskqueue \
                $(NIMNBIPAL_HOME)/src/posix/taskqueue \
                $(NIMNBIPAL_HOME)/src/common/unzip \
                $(NIMNBIPAL_HOME)/src/common/posix \
                $(NIMNBIPAL_HOME)/src/posix/net \
                $(NIMNBIPAL_HOME)/src/posix/taskqueue \
                $(NIMNBIPAL_HOME)/src/posix/timer \
                $(NIMTHIRDPARTY_HOME)/lp150b15 \
                $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/include \
                $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/include/freetype

#source file
PROJECT_ROOT := ../../../../..
NIMNBIPAL_HOME := $(PROJECT_ROOT)/nbpal/$(BRANCH)
NIMTHIRDPARTY_HOME = $(PROJECT_ROOT)/thirdparty/$(BRANCH)

NBPAL_SRC_FILES := $(NIMNBIPAL_HOME)/src/android_ndk/font/palfont.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/font/palfreetypeengine.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/imageblender/palimageblender.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/jni/jni_load.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/pal/palabstractfactoryandroid.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/pal/palimpl.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/pal/wm_global.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/synchronization/palatomic.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/taskqueue/nbuitaskqueue.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/taskqueue/paluitaskqueue.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/taskqueue/threadandroid.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/taskqueue/uitaskmanager.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/taskqueue/uithreadandroid.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/util/paldebuglog.c \
                   $(NIMNBIPAL_HOME)/src/android_ndk/util/paldisplay.c \
                   $(NIMNBIPAL_HOME)/src/android_ndk/util/palstdlib.c \
                   $(NIMNBIPAL_HOME)/src/android_ndk/radio/com_navbuilder_pal_android_ndk_RadioInterface.cpp \
                   $(NIMNBIPAL_HOME)/src/android_ndk/radio/palradio.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctionbase.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctiondnsresult.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctionhttpdata.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctionhttpstatus.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctionmanager.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctionnetworkdata.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctionnetworkstatus.cpp \
                   $(NIMNBIPAL_HOME)/src/common/finishfunction/finishfunctionnormal.cpp \
                   $(NIMNBIPAL_HOME)/src/common/gl/palgl.c \
                   $(NIMNBIPAL_HOME)/src/common/posix/task.cpp \
                   $(NIMNBIPAL_HOME)/src/common/posix/taskqueue.cpp \
                   $(NIMNBIPAL_HOME)/src/common/posix/thread.cpp \
                   $(NIMNBIPAL_HOME)/src/common/taskqueue/nbtaskqueue.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/clock/palclock.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/file/palfile.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callback.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callbackdnsresult.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callbackhttpdata.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callbackhttpresponseheaders.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callbackhttpresponsestatus.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callbackmanager.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callbacknetworkdata.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/callbacknetworkstatus.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/httpconnection.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/httprequest.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/httpresponse.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/netconnection.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/palnet.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/net/tcpconnection.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/synchronization/palevent.c \
                   $(NIMNBIPAL_HOME)/src/posix/synchronization/pallock.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/taskqueue/paltaskqueue.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/taskqueue/taskmanager.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/taskqueue/taskmanagermultithreaded.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/taskqueue/threadposix.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/timer/paltimer.cpp \
                   $(NIMNBIPAL_HOME)/src/posix/timer/timerimpl.cpp \
                   $(NIMNBIPAL_HOME)/src/util/bq.c \
                   $(NIMNBIPAL_HOME)/src/util/netutils.cpp \
                   $(NIMNBIPAL_HOME)/src/util/palstdlibcommon.c \
                   $(NIMNBIPAL_HOME)/src/util/palversion.c

THIRDPARTY_SRC_FILES := $(NIMTHIRDPARTY_HOME)/sqlite/Sources/sqlite3.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/autofit/autofit.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/bdf/bdf.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/cff/cff.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftbase.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftbitmap.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/cache/ftcache.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftfstype.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftgasp.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftglyph.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/gzip/ftgzip.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftinit.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/lzw/ftlzw.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftstroke.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftsystem.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/smooth/smooth.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftbbox.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftgxval.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftlcdfil.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftmm.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftotval.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftpatent.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftpfr.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftsynth.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/fttype1.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftwinfnt.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/base/ftxf86.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/pcf/pcf.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/pfr/pfr.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/psaux/psaux.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/pshinter/pshinter.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/psnames/psmodule.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/raster/raster.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/sfnt/sfnt.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/truetype/truetype.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/type1/type1.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/cid/type1cid.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/type42/type42.c \
                        $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/src/winfonts/winfnt.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/png.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngerror.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngget.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngmem.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngpread.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngread.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngrio.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngrtran.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngrutil.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngset.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngtrans.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngwio.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngwrite.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngwtran.c \
                        $(NIMTHIRDPARTY_HOME)/lp150b15/pngwutil.c

LOCAL_MODULE := nbpal
LOCAL_CFLAGS := -DANDROID_NDK -DDEBUG_LOG_ALL \
                -DFT2_BUILD_LIBRARY -DFT_OPTION_AUTOFIT2 -DFLOATING_POINT \
                -DUSE_STLPORT -DBOOST_EXCEPTION_DISABLE  -DBOOST_NO_EXCEPTION

LOCAL_C_INCLUDES += $(foreach d, $(INCLUDE_DIRS), $(d))
LOCAL_SRC_FILES += $(foreach d, $(NBPAL_SRC_FILES), $(d))
LOCAL_SRC_FILES += $(foreach d, $(THIRDPARTY_SRC_FILES), $(d))
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -lz -landroid

include $(BUILD_SHARED_LIBRARY)
