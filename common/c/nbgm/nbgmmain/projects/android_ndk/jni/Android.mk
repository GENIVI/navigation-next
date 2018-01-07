LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := nbgm

# where header files are located.
PROJ_ROOT := $(LOCAL_PATH)/../../../../../..
NIMNBIPAL_HOME := $(PROJ_ROOT)/nbpal/feature_130313_map3d_rel2_1
NIMNBGM_HOME := $(PROJ_ROOT)/nbgm/feature_130313_map3d_rel2_1
NIMTHIRDPARTY_HOME = $(PROJ_ROOT)/thirdparty/feature_130313_map3d_rel2_1

NIMNBGM_NBRE := $(NIMNBGM_HOME)/nbre
NIMNBGM_RENDERSYSTEM := $(NIMNBGM_HOME)/rendersystem/gles
NIMNBGM_MAIN := $(NIMNBGM_HOME)/nbgmmain

#pal header files
INCLUDE_DIRS := $(NIMNBIPAL_HOME)/include
INCLUDE_DIRS += $(NIMNBIPAL_HOME)/include/android_ndk
#dependence header files
INCLUDE_DIRS += $(NIMTHIRDPARTY_HOME)/lp150b15
INCLUDE_DIRS += $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/include
INCLUDE_DIRS += $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/include/freetype
#nbre header files
INCLUDE_DIRS += $(NIMNBGM_NBRE)/include
INCLUDE_DIRS += $(NIMNBGM_NBRE)/include/protected
INCLUDE_DIRS += $(NIMNBGM_NBRE)/include/private
INCLUDE_DIRS += $(NIMNBGM_NBRE)/include/android_ndk
INCLUDE_DIRS += $(NIMNBGM_NBRE)/include/debug
#OpenGL ES render pal
INCLUDE_DIRS += $(NIMNBGM_RENDERSYSTEM)/include
INCLUDE_DIRS += $(NIMNBGM_RENDERSYSTEM)/include/protected
INCLUDE_DIRS += $(NIMNBGM_RENDERSYSTEM)/include/private
#mbg, header files
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include/protected
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include/private
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include/android_ndk

LOCAL_CFLAGS := -DANDROID_NDK -DDEBUG_LOG_ALL
LOCAL_C_INCLUDES += $(foreach d, $(INCLUDE_DIRS), $(d))

SRC_PATH = ../../../src
SRCS := $(SRC_PATH)/android_ndk/nbgmimpl.cpp \
        $(SRC_PATH)/util/nbgmversion.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmarray.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmbinloader.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmdom.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmmaphorizontallightsourcematerial.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmmapmaterial.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmmapmaterialfactory.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmmapmaterialmanager.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmmapmaterialutility.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmmapoutlinedcolormaterial.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmpatterndashlinematerial.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmvectortiledata.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmvectortiledatacreator.cpp \
        $(SRC_PATH)/nbgmmapdata/nbgmvectortilemeshcreator.cpp \
        $(SRC_PATH)/nbgmservice/nbgmanimation.cpp \
        $(SRC_PATH)/nbgmservice/nbgmavatar.cpp \
        $(SRC_PATH)/nbgmservice/nbgmbuildutility.cpp \
        $(SRC_PATH)/nbgmservice/nbgmflag.cpp \
        $(SRC_PATH)/nbgmservice/nbgmgridentity.cpp \
        $(SRC_PATH)/nbgmservice/nbgmicon.cpp \
        $(SRC_PATH)/nbgmservice/nbgmlabellayer.cpp \
        $(SRC_PATH)/nbgmservice/nbgmlayoutbuffer.cpp \
        $(SRC_PATH)/nbgmservice/nbgmlayoutelement.cpp \
        $(SRC_PATH)/nbgmservice/nbgmlayoutmanager.cpp \
        $(SRC_PATH)/nbgmservice/nbgmlayoutpolyline.cpp \
        $(SRC_PATH)/nbgmservice/nbgmmaproadlabel.cpp \
        $(SRC_PATH)/nbgmservice/nbgmnavecmroutelayer.cpp \
        $(SRC_PATH)/nbgmservice/nbgmnaviconlayer.cpp \
        $(SRC_PATH)/nbgmservice/nbgmnavvectorroutelayer.cpp \
        $(SRC_PATH)/nbgmservice/nbgmpoilayer.cpp \
        $(SRC_PATH)/nbgmservice/nbgmpointlabel.cpp \
        $(SRC_PATH)/nbgmservice/nbgmpolyline2entity.cpp \
        $(SRC_PATH)/nbgmservice/nbgmrasterize.cpp \
        $(SRC_PATH)/nbgmservice/nbgmroadlabel.cpp \
        $(SRC_PATH)/nbgmservice/nbgmshield.cpp \
        $(SRC_PATH)/nbgmservice/nbgmsky.cpp \
        $(SRC_PATH)/nbgmservice/nbgmskywall.cpp \
        $(SRC_PATH)/nbgmservice/nbgmtilemaplayer.cpp \
        $(SRC_PATH)/nbgmservice/nbgmtransparentoverlay.cpp \
        $(SRC_PATH)/nbgmservice/nbgmvectorareabuilder.cpp \
        $(SRC_PATH)/nbgmservice/nbgmvectorroadbuilder.cpp \
        $(SRC_PATH)/nbgmservice/nbgmvectortile.cpp \
        $(SRC_PATH)/nbgmmanager/nbgmmapview.cpp \
        $(SRC_PATH)/nbgmmanager/nbgmmapviewdatacache.cpp \
        $(SRC_PATH)/nbgmmanager/nbgmmapviewimpl.cpp \
        $(SRC_PATH)/nbgmmanager/nbgmmapviewprofiler.cpp \
        $(SRC_PATH)/nbgmmanager/nbgmnavecmview.cpp \
        $(SRC_PATH)/nbgmmanager/nbgmnavvectorview.cpp \
        $(SRC_PATH)/nbgmmanager/nbgmnavview.cpp \
        $(SRC_PATH)/nbgmmanager/nbrebinaryspatialdividemanager.cpp \
        $(SRC_PATH)/nbgmmanager/nbrelinearspatialdividemanager.cpp

LOCAL_SRC_FILES = $(SRCS)

include $(BUILD_STATIC_LIBRARY)
