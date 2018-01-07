

#######################################################

######### Environment setting ############

BRANCH = $$dirname(PWD)
BRANCH = $$dirname(BRANCH)
BRANCH = $$basename(BRANCH)
LTK_ROOT = ../../../..
CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}

OS_PLATFORM_SPECIFIC = qt

CORESDK_ROOT = $$LTK_ROOT/build/$$BRANCH/CoreSDK

NIM_ABPAL_ROOT = $$CORESDK_ROOT/include/abpal
NIM_ABPAL_INCLUDE = $$NIM_ABPAL_ROOT \
        $$NIM_ABPAL_ROOT/$$OS_PLATFORM_SPECIFIC

NIM_MAP_ROOT = $$CORESDK_ROOT/include/map
NIM_MAP_INCLUDE = $$NIM_MAP_ROOT

NIM_NBPAL_ROOT = $$CORESDK_ROOT/include/nbpal
NIM_NBPAL_INCLUDE += $$NIM_NBPAL_ROOT \
        $$NIM_NBPAL_ROOT/$$OS_PLATFORM_SPECIFIC \
        $$NIM_NBPAL_ROOT/boost \
        $$NIM_NBPAL_ROOT/boost/smart_ptr

NIM_NBGM_ROOT = $$CORESDK_ROOT/include/nbgm
NIM_NBGM_INCLUDE += $$NIM_NBGM_ROOT \
        $$NIM_NBGM_ROOT/protected \
        $$NIM_NBGM_ROOT/$$OS_PLATFORM_SPECIFIC

NIM_NBUI_ROOT = $$CORESDK_ROOT/include/nbui
NIM_NBUI_INCLUDE += $$NIM_NBUI_ROOT \
        $$NIM_NBUI_ROOT/$$OS_PLATFORM_SPECIFIC \
        $$NIM_NBUI_ROOT/$$OS_PLATFORM_SPECIFIC/map \
        $$NIM_NBUI_ROOT/$$OS_PLATFORM_SPECIFIC/template

NIM_CORESERVICES_ROOT = $$CORESDK_ROOT/include/coreservices
NIM_CORESERVICES_INCLUDE = $$NIM_CORESERVICES_ROOT \
                        $$NIM_CORESERVICES_ROOT/$$OS_PLATFORM_SPECIFIC

NIM_NBSERVICES_ROOT = $$CORESDK_ROOT/include/nbservices
NIM_NBSERVICES_INCLUDE = $$NIM_NBSERVICES_ROOT \
                        $$NIM_NBSERVICES_ROOT/protected \
                        $$NIM_NBSERVICES_ROOT/$$OS_PLATFORM_SPECIFIC

CORESDK_INCLUDE = $$NIM_ABPAL_INCLUDE \
        $$NIM_MAP_INCLUDE \
        $$NIM_NBPAL_INCLUDE \
        $$NIM_NBGM_INCLUDE \
        $$NIM_NBUI_INCLUDE \
        $$NIM_CORESERVICES_INCLUDE \
        $$NIM_NBSERVICES_INCLUDE

CORESDK_LIBRARY = -L$$CORESDK_ROOT/lib/qt/$$BUILD_CONFIG \
        -lcommon -lmap -labservices -lnbservices -lnbui -lcoreservices -lnbgm -labpal -ltaskqueue -ltinyxml \
        -lnbpal -lglew32 -lNBM -lOpenGL32 -lgdi32 -lzlib -lpng

############# LTK ######################
LOCATIONKIT_ROOT = $$LTK_ROOT/locationkit/$$BRANCH/include
LTKCOMMON_ROOT = $$LTK_ROOT/common/$$BRANCH/include
LTKMAP3D_ROOT =  $$LTK_ROOT/mapkit3d/$$BRANCH/include

LIBOUTPUT_ROOT = $$LTK_ROOT/build/$$BRANCH/output

LTK_COMMON_INCLUDE = $$LTKCOMMON_ROOT \
        $$LTKCOMMON_ROOT/private
        #$$LTKCOMMON_ROOT/private/qt \
        #$$LTKCOMMON_ROOT/temp_navbuilder_nb \
        #$$LTKCOMMON_ROOT/qa \
        #$$LTKCOMMON_ROOT/temp
LTK_LOCATIONKIT_INCLUDE = $$LOCATIONKIT_ROOT
LTK_MAP3D_INCLUDE = $$LTKMAP3D_ROOT $$LTKMAP3D_ROOT\model

LTK_INCLUDE =  $$LTK_COMMON_INCLUDE  \
        $$LTK_LOCATIONKIT_INCLUDE \
        $$LTK_MAP3D_INCLUDE
LTK_LIBRARY = -L$$LIBOUTPUT_ROOT/$$BUILD_CONFIG/lib \
        -lmapkit3d -lltkcommon -llocationkit

#######################################################
QT       += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


MOC_DIR += ../src/moc
OBJECTS_DIR += ./obj/$$BUILD_CONFIG
DEPENDPATH += ../src
TARGET = ../$$LIBOUTPUT_ROOT/$$BUILD_CONFIG/bin/test_mapkit3d
TEMPLATE = app

DEFINES += MAKE_STATIC_LIBRARIES
LIBS += $$LTK_LIBRARY \
        $$CORESDK_LIBRARY \
        -lOpenGL32 -lgdi32

INCLUDEPATH += ../include
INCLUDEPATH += $$CORESDK_INCLUDE
INCLUDEPATH += $$LTK_INCLUDE

SOURCES += \
    ../src/main.cpp \
    ../src/mainwindow.cpp \
    ../src/useroptions.cpp

HEADERS  += \
    ../include/mainwindow.h \
    ../include/useroptions.h

FORMS    += \
    ../Forms/mainwindow.ui \
    ../Forms/useroptions.ui

