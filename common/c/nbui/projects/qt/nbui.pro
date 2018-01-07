CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/nbui
NIM_NBPAL_ROOT = $$CCC_ROOT/nbpal
NIM_NBGM_ROOT = $$CCC_ROOT/nbgm
NIM_NBSERVICES_ROOT = $$CCC_ROOT/nbservices
NIM_CORESERVICES_ROOT = $$CCC_ROOT/coreservices
NIM_THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty

INCLUDEPATH += $$NIM_NBPAL_ROOT/include
INCLUDEPATH += $$NIM_NBPAL_ROOT/include/qt
INCLUDEPATH += $$NIM_NBPAL_ROOT/src/qt/threading
INCLUDEPATH += $$NIM_NBPAL_ROOT/src/common/taskqueue

INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include/protected
INCLUDEPATH += $$NIM_NBGM_ROOT/nbgmmain/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_NBSERVICES_ROOT/include/protected
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include
INCLUDEPATH += $$NIM_CORESERVICES_ROOT/include/qt
INCLUDEPATH += $$NIM_THIRDPARTY_ROOT/glew-1.7.0/include

INCLUDEPATH += $$PROJ_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/include/qt/map
INCLUDEPATH += $$PROJ_ROOT/include/qt/template
INCLUDEPATH += $$PROJ_ROOT/include/protected

HEADERS += $$PROJ_ROOT/include/MapNativeViewInterfaces.h \
    ../../include/qt/map/MapViewProtocol.h \
    ../../include/qt/map/MapNativeViewImpl.h \
    ../../include/qt/map/mapdecorationsignals.h \
#    ../../include/qt/map/navigationoptionbutton.h \
#    ../../include/qt/map/pangesturerecognizer.h \
#    ../../include/qt/map/doubletapgesturerecognizer.h \
#    ../../include/qt/map/singletapgesturerecognizer.h \
#    ../../include/qt/map/twofingerstapgesturerecognizer.h
    ../../src/qt/map/gesturedetector.h

HEADERS += $$PROJ_ROOT/include/MapWidgetGetter.h
HEADERS += $$PROJ_ROOT/include/qt/map/BubbleQt.h
HEADERS += $$PROJ_ROOT/include/qt/map/QtNativeHostView.h
HEADERS += $$PROJ_ROOT/include/qt/map/NativeBubbleView.h
HEADERS += $$PROJ_ROOT/include/qt/map/NativeRenderContext.h
HEADERS += $$PROJ_ROOT/include/qt/template/PinBubbleResolverQt.h
#HEADERS += $$PROJ_ROOT/include/qt/map/TrafficMapWidget.h
#HEADERS += $$PROJ_ROOT/include/qt/map/TiltGestureRecognizer.h
#HEADERS += $$PROJ_ROOT/include/qt/map/LocateMeButton.h
#HEADERS += $$PROJ_ROOT/include/qt/map/maplegendcolorwidget.h
#HEADERS += $$PROJ_ROOT/include/qt/map/maplegendwidget.h
HEADERS += $$PROJ_ROOT/include/protected/NBUIConfig.h
HEADERS += $$PROJ_ROOT/include/protected/NBUITemplateInstantiator.h
HEADERS += $$PROJ_ROOT/include/protected/TrafficBubbleProvider.h

SOURCES += $$PROJ_ROOT/src/qt/map/BubbleQt.cpp \
#    ../../src/qt/map/pangesturerecognizer.cpp \
#    ../../src/qt/map/doubletapgesturerecognizer.cpp \
#    ../../src/qt/map/singletapgesturerecognizer.cpp \
#    ../../src/qt/map/twofingerstapgesturerecognizer.cpp
    ../../src/qt/template/PinBubbleResolverQt.cpp \
    ../../src/qt/map/gesturedetector.cpp

#SOURCES += $$PROJ_ROOT/src/qt/map/navigationoptionbutton.cpp
#SOURCES += $$PROJ_ROOT/src/qt/map/TiltGestureRecognizer.cpp
SOURCES += $$PROJ_ROOT/src/qt/map/QtNativeHostView.cpp
SOURCES += $$PROJ_ROOT/src/qt/map/MapNativeView.cpp
SOURCES += $$PROJ_ROOT/src/qt/map/NativeRenderContext.cpp
SOURCES += $$PROJ_ROOT/src/qt/map/NBUIConfig.cpp
#SOURCES += $$PROJ_ROOT/src/qt/map/LocateMeButton.cpp
#SOURCES += $$PROJ_ROOT/src/qt/map/maplegendcolorwidget.cpp
#SOURCES += $$PROJ_ROOT/src/qt/map/maplegendwidget.cpp

win32: SOURCES += $$PROJ_ROOT/src/qt/map/NBUIConfigOpenGL.cpp
unix: SOURCES += $$PROJ_ROOT/src/qt/map/NBUIConfigOpenGLES2.cpp

SOURCES += $$PROJ_ROOT/src/qt/map/TrafficBubbleProvider.cpp
#SOURCES += $$PROJ_ROOT/src/qt/map/TrafficMapWidget.cpp
SOURCES += $$PROJ_ROOT/src/qt/template/NBUITemplateInstantiator.cpp
#SOURCES += $$PROJ_ROOT/src/qt/template/PinBubbleResolverQt.cpp
SOURCES += $$PROJ_ROOT/src/qt/template/TemplateInstantiation.cpp

QT += qml quick
TEMPLATE = lib
TARGET = nbui
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
DEFINES += MAKE_STATIC_LIBRARIES GLEW_STATIC
qnx: DEFINES += SUPPORTS_QT_GESTURE

debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/


include ($$CCC_ROOT/build/projects/qt/config.pro)

