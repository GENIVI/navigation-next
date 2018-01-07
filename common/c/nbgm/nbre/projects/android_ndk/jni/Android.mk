LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := NBRE

# where header files are located.
PROJ_ROOT := $(LOCAL_PATH)/../../../../../..
NIMNBIPAL_HOME := $(PROJ_ROOT)/nbpal/feature_130313_map3d_rel2_1
NIMNBGM_HOME := $(PROJ_ROOT)/nbgm/feature_130313_map3d_rel2_1
NIMTHIRDPARTY_HOME = $(PROJ_ROOT)/thirdparty/feature_130313_map3d_rel2_1

NIMNBGM_MAIN := $(NIMNBGM_HOME)/nbgmmain
NIMNBGM_NBRE := $(NIMNBGM_HOME)/nbre
NIMNBGM_RENDERSYSTEM := $(NIMNBGM_HOME)/rendersystem/gles

#pal header files
INCLUDE_DIRS := $(NIMNBIPAL_HOME)/include
INCLUDE_DIRS += $(NIMNBIPAL_HOME)/include/android_ndk
#thirdparty header files
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
#nbgm main header files
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include/protected
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include/private
INCLUDE_DIRS += $(NIMNBGM_MAIN)/include/android_ndk

LOCAL_CFLAGS := -DANDROID_NDK -DDEBUG_LOG_ALL
LOCAL_C_INCLUDES += $(foreach d, $(INCLUDE_DIRS), $(d))

SRC_PATH = ../../../src
SRCS := $(SRC_PATH)/core/nbrememorystream.cpp \
        $(SRC_PATH)/core/nbrefilestream.cpp \
        $(SRC_PATH)/core/nbreiostream.cpp \
        $(SRC_PATH)/core/nbrelog.cpp \
        $(SRC_PATH)/core/nbremath.cpp \
        $(SRC_PATH)/core/nbrememory.cpp \
        $(SRC_PATH)/resource/nbredefaultshadergroup.cpp \
        $(SRC_PATH)/resource/nbrefont.cpp \
        $(SRC_PATH)/resource/nbrefontmanager.cpp \
        $(SRC_PATH)/resource/nbrefreetypefont.cpp \
        $(SRC_PATH)/resource/nbreglyph.cpp \
        $(SRC_PATH)/resource/nbreglyphatlas.cpp \
        $(SRC_PATH)/resource/nbreimage.cpp \
        $(SRC_PATH)/resource/nbrematerial.cpp \
        $(SRC_PATH)/resource/nbremesh.cpp \
        $(SRC_PATH)/resource/nbrepackingtree.cpp \
        $(SRC_PATH)/resource/nbrepacktextureimage.cpp \
        $(SRC_PATH)/resource/nbrepngcodec.cpp \
        $(SRC_PATH)/resource/nbrepngtextureimage.cpp \
        $(SRC_PATH)/resource/nbreshader.cpp \
        $(SRC_PATH)/resource/nbreshadermanager.cpp \
        $(SRC_PATH)/resource/nbresubmesh.cpp \
        $(SRC_PATH)/resource/nbretexture.cpp \
        $(SRC_PATH)/resource/nbretexturemanager.cpp \
        $(SRC_PATH)/resource/nbretextureptr.cpp \
        $(SRC_PATH)/scene/nbrebillboardentity.cpp \
        $(SRC_PATH)/scene/nbrebillboardnode.cpp \
        $(SRC_PATH)/scene/nbredefaultspatialdividemanager.cpp \
        $(SRC_PATH)/scene/nbreentity.cpp \
        $(SRC_PATH)/scene/nbrenode.cpp \
        $(SRC_PATH)/scene/nbreoverlay.cpp \
        $(SRC_PATH)/scene/nbrerenderoperation.cpp \
        $(SRC_PATH)/scene/nbrerendersurface.cpp \
        $(SRC_PATH)/scene/nbrescenemanager.cpp \
        $(SRC_PATH)/scene/nbresubentity.cpp \
        $(SRC_PATH)/scene/nbresurfacemanager.cpp \
        $(SRC_PATH)/scene/nbresurfacesubview.cpp \
        $(SRC_PATH)/scene/nbretextentity.cpp \
        $(SRC_PATH)/scene/nbretextpath3dentity.cpp \
        $(SRC_PATH)/scene/nbreveiwport.cpp \
        $(SRC_PATH)/scene/nbrezone.cpp \
        $(SRC_PATH)/renderpal/nbrehardwareindexbuffer.cpp \
        $(SRC_PATH)/renderpal/nbrerendertarget.cpp \
        $(SRC_PATH)/renderpal/nbrevertexindexdata.cpp \
        $(SRC_PATH)/nbre/nbrecamera.cpp \
        $(SRC_PATH)/nbre/nbrefrustum.cpp \
        $(SRC_PATH)/nbre/nbrelightnode.cpp \
        $(SRC_PATH)/nbre/nbrerenderengine.cpp \
        $(SRC_PATH)/debug/nbrecameradebug.cpp \
        $(SRC_PATH)/debug/nbredebugboundingbox.cpp \
        $(SRC_PATH)/debug/nbredebugfrustum.cpp \
        $(SRC_PATH)/debug/nbredebugray.cpp \
        $(SRC_PATH)/debug/nbrespatialdividemanagerdebug.cpp
		
LOCAL_SRC_FILES = $(SRCS)

include $(BUILD_STATIC_LIBRARY)
