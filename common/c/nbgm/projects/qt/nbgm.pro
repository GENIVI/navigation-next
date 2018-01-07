CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/nbgm
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty

INCLUDEPATH += $$PROJ_ROOT/nbgmmain/include
INCLUDEPATH += $$PROJ_ROOT/nbgmmain/include/qt
INCLUDEPATH += $$PROJ_ROOT/nbgmmain/include/protected
INCLUDEPATH += $$PROJ_ROOT/nbgmmain/include/private
INCLUDEPATH += $$PROJ_ROOT/nbre/include
INCLUDEPATH += $$PROJ_ROOT/nbre/include/private
INCLUDEPATH += $$PROJ_ROOT/nbre/include/debug

HEADERS += $$PROJ_ROOT/nbgmmain/include/*.h
HEADERS += $$PROJ_ROOT/nbgmmain/include/qt/*.h
HEADERS += $$PROJ_ROOT/nbgmmain/include/protected/*.h
HEADERS += $$PROJ_ROOT/nbgmmain/include/private/*.h
HEADERS += $$PROJ_ROOT/nbre/include/*.h
HEADERS += $$PROJ_ROOT/nbre/include/private/*.h
HEADERS += $$PROJ_ROOT/nbre/include/debug/*.h

unix {
INCLUDEPATH += $$PROJ_ROOT/rendersystem/gles/include
INCLUDEPATH += $$PROJ_ROOT/rendersystem/gles/include/private
INCLUDEPATH += $$PROJ_ROOT/rendersystem/gles2/include
INCLUDEPATH += $$PROJ_ROOT/rendersystem/gles2/include/private
}
win32 {
INCLUDEPATH += $$PROJ_ROOT/rendersystem/d3d9/include
INCLUDEPATH += $$PROJ_ROOT/rendersystem/d3d9/include/private
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/glew-1.7.0/include
INCLUDEPATH += $$PROJ_ROOT/rendersystem/gl/include
INCLUDEPATH += $$PROJ_ROOT/rendersystem/gl/include/private
}
INCLUDEPATH += $$PROJ_ROOT/nbre/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt

linux {
}
qnx {
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/libpng-1.4
}
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/freetype-2.4.4/include
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/libjpeg_turbo/include
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/libjpeg/include
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/libjpeg/include/qt

SOURCES += $$PROJ_ROOT/nbgmmain/src/*.cpp
SOURCES += $$PROJ_ROOT/nbgmmain/src/util/*.cpp
SOURCES += $$PROJ_ROOT/nbgmmain/src/nbgmmanager/*.cpp
SOURCES += $$PROJ_ROOT/nbgmmain/src/nbgmmapdata/*.cpp
SOURCES += $$PROJ_ROOT/nbgmmain/src/nbgmservice/*.cpp
win32 {
SOURCES += $$PROJ_ROOT/rendersystem/gl/src/nbreglhardwareindexbuffer.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gl/src/nbreglhardwarevertexbuffer.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gl/src/nbreglrenderpal.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gl/src/nbreglrendertexture.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gl/src/nbreglrenderwindow.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gl/src/nbregltexture.cpp
}
unix {
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2gpuprogram.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2gpuprogramfactory.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2hardwareindexbuffer.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2hardwarevertexbuffer.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2renderpal.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2rendertexture.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2renderwindow.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2texture.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2vertexdeclaration.cpp
SOURCES += $$PROJ_ROOT/rendersystem/gles2/src/nbregles2ssaoprogram.cpp
}
SOURCES += $$PROJ_ROOT/nbre/src/core/*.cpp
SOURCES += $$PROJ_ROOT/nbre/src/debug/*.cpp
SOURCES += $$PROJ_ROOT/nbre/src/nbre/*.cpp
SOURCES += $$PROJ_ROOT/nbre/src/renderpal/*.cpp
SOURCES += $$PROJ_ROOT/nbre/src/resource/*.cpp
SOURCES += $$PROJ_ROOT/nbre/src/scene/*.cpp
SOURCES += $$NIM_THIRDPARTY_ROOT/libjpeg/src/*.c
SOURCES += $$NIM_THIRDPARTY_ROOT/libjpeg_turbo/src/*.c

TEMPLATE = lib
TARGET = nbgm
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
DEFINES += MAKE_STATIC_LIBRARIES

unix: DEFINES += SUPPORT_GLES20
win32: DEFINES += SUPPORT_GL GLEW_STATIC NOMINMAX
debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)

