LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#header file
PROJECT_ROOT := $(LOCAL_PATH)/../../../../..
NIMNBIPAL_HOME := $(PROJECT_ROOT)/nbpal/$(BRANCH)
NIMNBGM_HOME := $(PROJECT_ROOT)/nbgm/$(BRANCH)
NIMTHIRDPARTY_HOME = $(PROJECT_ROOT)/thirdparty/$(BRANCH)
NIMNBGM_NBRE := $(NIMNBGM_HOME)/nbre
NIMNBGM_RENDERSYSTEM := $(NIMNBGM_HOME)/rendersystem
NIMNBGM_MAIN := $(NIMNBGM_HOME)/nbgmmain

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
                $(NIMTHIRDPARTY_HOME)/freetype-2.4.4/include/freetype \
                $(NIMNBGM_NBRE)/include \
                $(NIMNBGM_NBRE)/include/protected \
                $(NIMNBGM_NBRE)/include/private \
                $(NIMNBGM_NBRE)/include/android_ndk \
                $(NIMNBGM_NBRE)/include/debug \
                $(NIMNBGM_RENDERSYSTEM)/gles/include \
                $(NIMNBGM_RENDERSYSTEM)/gles/include/protected \
                $(NIMNBGM_RENDERSYSTEM)/gles/include/private \
                $(NIMNBGM_RENDERSYSTEM)/gles2/include \
                $(NIMNBGM_RENDERSYSTEM)/gles2/include/protected \
                $(NIMNBGM_RENDERSYSTEM)/gles2/include/private \
                $(NIMNBGM_MAIN)/include \
                $(NIMNBGM_MAIN)/include/protected \
                $(NIMNBGM_MAIN)/include/private \
                $(NIMNBGM_MAIN)/include/android_ndk

#source file
PROJECT_ROOT := ../../../../..
NIMNBGM_HOME := $(PROJECT_ROOT)/nbgm/$(BRANCH)
NIMNBGM_NBRE := $(NIMNBGM_HOME)/nbre
NIMNBGM_RENDERSYSTEM := $(NIMNBGM_HOME)/rendersystem
NIMNBGM_MAIN := $(NIMNBGM_HOME)/nbgmmain
NBGMMAIN_SRC_FILES :=   $(NIMNBGM_MAIN)/src/util/nbgmversion.cpp \
                        $(NIMNBGM_MAIN)/src/android_ndk/nbgmimpl.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmanimation.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmavatar.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmbuildutility.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmdebugpointlabel.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmflag.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmgridentity.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmicon.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmlabeldebugutil.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmlabellayer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmlabellayertile.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmlayoutbuffer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmlayoutelement.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmlayoutmanager.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmlayoutpolyline.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmmaproadlabel.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmnavecmroutelayer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmnaviconlayer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmnavvectorroutelayer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmoptionallayerimpl.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmpatharrowlayer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmpatharrowlayertile.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmpoilayer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmpoilayertile.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmpointlabel.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmpolyline2entity.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmrasterize.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmroadlabel.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmshield.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmsky.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmskywall.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmstaticlabeltile.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmtileanimationlayerimpl.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmtilemaplayer.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmtilemaplayertile.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmtransparentoverlay.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmvectorareabuilder.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmvectorroadbuilder.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmvectortile.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmservice/nbgmvectorvertexscale.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmarray.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmbinloader.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmdom.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmmaphorizontallightsourcematerial.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmmapmaterial.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmmapmaterialfactory.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmmapmaterialmanager.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmmapmaterialutility.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmmapoutlinedcolormaterial.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmmappatharrowmaterial.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmpatterndashlinematerial.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmvectortiledata.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmvectortiledatacreator.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmapdata/nbgmvectortilemeshcreator.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmfakemapview.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmmapview.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmmapviewdatacache.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmmapviewimpl.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmmapviewimpltask.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmmapviewprofiler.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmnavecmview.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmnavvectorview.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmnavview.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbgmtaskqueue.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbrebinaryspatialdividemanager.cpp \
                        $(NIMNBGM_MAIN)/src/nbgmmanager/nbrelinearspatialdividemanager.cpp

NBRE_SRC_FILES :=   $(NIMNBGM_NBRE)/src/core/nbrefilelogger.cpp \
                    $(NIMNBGM_NBRE)/src/core/nbrefilestream.cpp \
                    $(NIMNBGM_NBRE)/src/core/nbreiostream.cpp \
                    $(NIMNBGM_NBRE)/src/core/nbrelog.cpp \
                    $(NIMNBGM_NBRE)/src/core/nbremath.cpp \
                    $(NIMNBGM_NBRE)/src/core/nbrememory.cpp \
                    $(NIMNBGM_NBRE)/src/core/nbrememorystream.cpp \
                    $(NIMNBGM_NBRE)/src/core/nbreutility.cpp \
                    $(NIMNBGM_NBRE)/src/debug/nbrecameradebug.cpp \
                    $(NIMNBGM_NBRE)/src/debug/nbredebugboundingbox.cpp \
                    $(NIMNBGM_NBRE)/src/debug/nbredebugfrustum.cpp \
                    $(NIMNBGM_NBRE)/src/debug/nbredebugray.cpp \
                    $(NIMNBGM_NBRE)/src/debug/nbrespatialdividemanagerdebug.cpp \
                    $(NIMNBGM_NBRE)/src/nbre/nbrecamera.cpp \
                    $(NIMNBGM_NBRE)/src/nbre/nbrefrustum.cpp \
                    $(NIMNBGM_NBRE)/src/nbre/nbrelightnode.cpp \
                    $(NIMNBGM_NBRE)/src/nbre/nbrerenderengine.cpp \
                    $(NIMNBGM_NBRE)/src/renderpal/nbrehardwareindexbuffer.cpp \
                    $(NIMNBGM_NBRE)/src/renderpal/nbrerendertarget.cpp \
                    $(NIMNBGM_NBRE)/src/renderpal/nbrevertexindexdata.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbredefaultshadergroup.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrefont.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrefontmanager.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrefreetypefont.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbreglyph.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbreglyphatlas.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbreimage.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrematerial.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbremesh.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrepackingtree.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrepacktextureimage.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrepngcodec.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbrepngtextureimage.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbreshader.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbreshadermanager.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbresubmesh.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbretexture.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbretextureatlas.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbretexturemanager.cpp \
                    $(NIMNBGM_NBRE)/src/resource/nbretextureptr.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrebillboardentity.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrebillboardnode.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbredefaultspatialdividemanager.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbreentity.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrenode.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbreoverlay.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrerenderoperation.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrerendersurface.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrescenemanager.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbresubentity.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbresurfacemanager.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbresurfacesubview.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbretextentity.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbretextpath3dentity.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbretransformutil.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbreveiwport.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrevertexscale.cpp \
                    $(NIMNBGM_NBRE)/src/scene/nbrezone.cpp

RENDERSYSTEM_SRC_FILES :=   $(NIMNBGM_RENDERSYSTEM)/gles/src/nbregleshardwareindexbuffer.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles/src/nbregleshardwarevertexbuffer.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles/src/nbreglesrenderpal.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles/src/nbreglesrendertexture.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles/src/nbreglesrenderwindow.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles/src/nbreglestexture.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2gpuprogram.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2gpuprogramfactory.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2hardwareindexbuffer.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2hardwarevertexbuffer.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2renderpal.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2rendertexture.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2renderwindow.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2texture.cpp \
                            $(NIMNBGM_RENDERSYSTEM)/gles2/src/nbregles2vertexdeclaration.cpp


LOCAL_MODULE := nbgm
LOCAL_CFLAGS := -DANDROID_NDK -DDEBUG_LOG_ALL \
				-DGL_GLEXT_PROTOTYPES=1 \
				-DUSE_STLPORT -DBOOST_EXCEPTION_DISABLE  -DBOOST_NO_EXCEPTION

LOCAL_C_INCLUDES += $(foreach d, $(INCLUDE_DIRS), $(d))
LOCAL_SRC_FILES += $(foreach d, $(NBGMMAIN_SRC_FILES), $(d))
LOCAL_SRC_FILES += $(foreach d, $(NBRE_SRC_FILES), $(d))
LOCAL_SRC_FILES += $(foreach d, $(RENDERSYSTEM_SRC_FILES), $(d))
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -lz -lEGL -landroid
LOCAL_SHARED_LIBRARIES := libnbpal

include $(BUILD_SHARED_LIBRARY)
