LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := jni

LOCAL_C_INCLUDES := ../../include/private \
    ../../../../nbpal/$(BRANCH)/include \
	../../../../nbpal/$(BRANCH)/include/android_ndk \
	../../../../abpal/include \
	../../../../abpal/include/private \
	../../../../abpal/include/android_ndk \
	../../../../coreservices/$(BRANCH)/include \
	../../../../coreservices/$(BRANCH)/include/private \
	../../../../coreservices/$(BRANCH)/include/protected \
	../../../../coreservices/$(BRANCH)/include/android_ndk \
	../../../../nbservices/$(BRANCH)/include \
	../../../../nbservices/$(BRANCH)/include/protected \
	../../../../nbservices/$(BRANCH)/include/android_ndk \
	../../../../abservices/$(BRANCH)/include \
	../../../../abservices/$(BRANCH)/include/private \
	../../../../abservices/$(BRANCH)/include/android_ndk \

LOCAL_CFLAGS := -DANDROID_NDK
#LOCAL_CPP_FEATURES += exceptions
#LOCAL_CPP_FEATURES += rtti

SRCS := ../../../src/jni_map_native_view.cpp  \
    ../../../src/jni_mapview.cpp  \
    ../../../src/jni_mapview_interface.cpp  \
    ../../../src/jni_mapview_ui_interface.cpp  \
    ../../../src/android_ndk/AndroidMapController.cpp  \
    ../../../src/android_ndk/AndroidMapConfig.cpp  \
    ../../../src/android_ndk/AndroidMapNativeView.cpp  \
    ../../../src/com_navbuilder_nb_geocode_internal_NativeGeocodeHandler.cpp  \
	../../../src/com_navbuilder_nb_internal_NBInternal.cpp  \
	../../../src/jniaddress.cpp  \
	../../../src/jnihelper.cpp  \
	../../../src/jninbcontext.cpp  \
	../../../src/jninbgeocodehandler.cpp  \
	../../../src/jninbgeocodeinformation.cpp  \
	../../../src/jninbgeocodeparameters.cpp  \
	../../../src/jninblocation.cpp  \
	../../../src/jninbnetworkconfiguration.cpp  \
	../../../src/jniresultslice.cpp  \
    ../../../src/jni_common.cpp \

LOCAL_SRC_FILES = $(SRCS)

LOCAL_LDFLAGS :=  ../../../../nbpal/$(BRANCH)/projects/android_ndk/libs/armeabi/libnbpal.so \
	../../../../abpal/$(BRANCH)/projects/android_ndk/libs/armeabi/libabpal.so \
	../../../../coreservices/$(BRANCH)/projects/android_ndk/libs/armeabi/libcoreservices.so \
	../../../../nbservices/$(BRANCH)/projects/android_ndk/libs/armeabi/libnbservices.so \
	../../../../abservices/$(BRANCH)/projects/android_ndk/libs/armeabi/libabservices.so \

LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)

