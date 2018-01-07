LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

PROJECT_ROOT := $(LOCAL_PATH)/../../../../..
NIMNBIPAL_HOME := $(PROJECT_ROOT)/nbpal/$(BRANCH)
NIMNBGM_HOME := $(PROJECT_ROOT)/nbgm/$(BRANCH)/nbgmmain
NIMTHIRDPARTY_HOME = $(PROJECT_ROOT)/thirdparty/$(BRANCH)

INCLUDE_DIRS := $(LOCAL_PATH)/include \
                $(NIMNBIPAL_HOME)/include \
                $(NIMNBIPAL_HOME)/include/private \
                $(NIMNBIPAL_HOME)/include/android_ndk \
                $(NIMNBGM_HOME)/include \
                $(NIMNBGM_HOME)/include/protected \
                $(NIMNBGM_HOME)/include/private \
                $(NIMNBGM_HOME)/include/android_ndk


LOCAL_C_INCLUDES += $(foreach d, $(INCLUDE_DIRS), $(d))
			
LOCAL_SRC_FILES :=  src/nativemapview.cpp \
                    src/nbgmandroidrendercontext.cpp \
                    src/nbgmandroidegl.cpp \
                    src/jniutility.cpp \
                    src/mapviewcontroller.cpp

LOCAL_MODULE := nbgmuitest
LOCAL_CFLAGS := -DANDROID_NDK -DDEBUG_LOG_ALL -DUSE_STLPORT -DBOOST_EXCEPTION_DISABLE  -DBOOST_NO_EXCEPTION
LOCAL_LDLIBS := -llog -lEGL -landroid -lGLESv1_CM -lGLESv2
LOCAL_SHARED_LIBRARIES := libnbpal libnbgm

include $(BUILD_SHARED_LIBRARY)

