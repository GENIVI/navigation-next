CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}

######### CoreSDK ############
CORESDK_ROOT = $$LTK_ROOT/build/$$BRANCH/CoreSDK
Debug:CORESDK_LIBRARY += -L$$CORESDK_ROOT/lib/qt/debug
Release:CORESDK_LIBRARY += -L$$CORESDK_ROOT/lib/qt/release
CORESDK_LIBRARY += -lmap -labservices -lnbservices -lcommon -lnbui -lcoreservices -lnbgm -labpal -ltinyxml -lNBM -lpng -lxml2
win32{
CORESDK_LIBRARY += -ltaskqueue -lglew32  -lOpenGL32 -lgdi32 -lzlib -lnbpal
}unix{
CORESDK_LIBRARY += -lnbpal -lfreetype -lz -lsqlite3 -lX11 -lEGL -ldl -lrt
}

######### LTK ############
LTK_COMMON_INCLUDE = $$LTK_ROOT/common/$$BRANCH/include
LTK_LOCATIONKIT_INCLUDE = $$LTK_ROOT/locationkit/$$BRANCH/include
LTK_MAP3D_INCLUDE = $$LTK_ROOT/mapkit3d/$$BRANCH/include \
                    $$LTK_ROOT/mapkit3d/$$BRANCH/include/model
LTK_NAVKIT_INCLUDE = $$LTK_ROOT/include/navigationkit \
                     $$LTK_ROOT/include/navigationkit/data \
                    $$LTK_ROOT/include/navigationkit/signal
LTK_NAVUIKIT_INCLUDE = $$LTK_ROOT/include/navigationuikit \
                       $$LTK_ROOT/include/navigationuikit/data \
                       $$LTK_ROOT/include/navigationuikit/signal

LTK_INCLUDE = $$LTK_COMMON_INCLUDE  \
        $$LTK_LOCATIONKIT_INCLUDE \
        $$LTK_MAP3D_INCLUDE \
        $$LTK_NAVKIT_INCLUDE \
        $$LTK_NAVUIKIT_INCLUDE
LIBOUTPUT_ROOT = $$LTK_ROOT/build/$$BRANCH/output
Debug: LTK_LIBRARY = -L$$LIBOUTPUT_ROOT/debug/lib
Release: LTK_LIBRARY = -L$$LIBOUTPUT_ROOT/release/lib
LTK_LIBRARY += -lmapkit3d -lltkcommon -llocationkit

BIN_OUTPUT_PATH = $$LIBOUTPUT_ROOT/$$BUILD_CONFIG/bin
######### APP Setting ############
QT       += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
LIBS += $$LTK_LIBRARY \
        $$CORESDK_LIBRARY
INCLUDEPATH += $$LTK_INCLUDE
MOC_DIR += ../src/moc
OBJECTS_DIR += ./obj/$$BUILD_CONFIG
DEPENDPATH += ../src
TEMPLATE = app
CONFIG += debug_and_release debug_and_release_target
