######### Environment setting ############
LTK_ROOT = ../../..
include ($$LTK_ROOT/build/qt/build_library.pro)


######### Project setting ############
QT       += gui
INCLUDEPATH += ../include/private
INCLUDEPATH += ../include/private/nbui
INCLUDEPATH += ../include/model

SOURCES += \
    ../src/mapwidget.cpp \
    ../src/model/animationparameters.cpp \
    ../src/model/cameraparameters.cpp \
    ../src/model/latlngbound.cpp \
    ../src/model/pinparameters.cpp \
    ../src/model/radiusparameters.cpp \
    ../src/model/polylineparameters.cpp \
    ../src/private/mapwidgetinternal.cpp \
    ../src/private/avatarimpl.cpp \
    ../src/private/pinimpl.cpp \
    ../src/private/bubbleinternal.cpp \
    ../src/private/polylineimpl.cpp \
    ../src/private/defaultmapbubbleresolver.cpp \
    ../src/private/defaultbubble.cpp \
    ../src/private/mapprojectionimpl.cpp \
    ../src/model/camerafactory.cpp \
    ../src/mapdecoration.cpp \
    ../src/model/mapoptions.cpp

HEADERS  += \
    ../include/mapwidgetlistener.h \
    ../include/mapdecoration.h \
    ../include/mapoptions.h \
    ../include/mapwidget.h \
    ../include/model/cameraparameters.h \
    ../include/model/latlngbound.h \
    ../include/model/animationparameters.h \
    ../include/model/avatar.h \
    ../include/model/pinparameters.h \
    ../include/model/bubble.h \
    ../include/model/radiusparameters.h \
    ../include/model/pin.h \
    ../include/model/polyline.h \
    ../include/model/polylineparameters.h \
    ../include/private/avatarimpl.h \
    ../include/private/mapwidgetinternal.h \
    ../include/private/pinimpl.h \
    ../include/private/bubbleinternal.h \
    ../include/private/polylineimpl.h \
    ../include/private/defaultmapbubbleresolver.h \
    ../include/private/defaultbubble.h \
    ../include/model/mapprojection.h \
    ../include/private/mapprojectionimpl.h \
    ../include/model/camerasetting.h \
    ../include/model/camerafactory.h \
    ../include/model/prefetchconfiguration.h
