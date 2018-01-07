LTK_ROOT = ../../../../sampleapp/library
include ($$LTK_ROOT/locationtoolkit.pro)

#############################
MOC_DIR += ../src/moc
TEMPLATE = app
TARGET = mapkit3d_sample

CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}
OBJECTS_DIR += ./obj/$$BUILD_CONFIG

#####################################################
INCLUDEPATH += ../include \
               ../../util/include \
               ../include/mainmenu \
               ../include/basicmap \
               ../include/poipin \
               ../include/location \
               ../include/followme \
               ../include/optionallayers \
               ../include/poilist \
               ../include/about \
               ../include/devtool \
               ../include/camera \
               ../include/avatar

SOURCES += \
    ../src/main.cpp \
    ../src/mainwindow.cpp \
    ../src/toolbarcommon.cpp \
    ../src/basicmap/basicmapwidget.cpp \
    ../src/basicmap/toolbarbasicmap.cpp \
    ../src/mainmenu/listdelegate.cpp \
    ../src/mainmenu/menuwidget.cpp \
    ../src/mainmenu/toolbarmenu.cpp \
    ../src/poipin/toolbarpoipin.cpp \
    ../src/poipin/poipinmapwidget.cpp \
    ../src/location/locationmap.cpp \
    ../src/location/toolbarlocation.cpp \
    ../src/followme/followmemapwidget.cpp \
    ../src/followme/toolbarfollowme.cpp \
    ../src/followme/interpolator.cpp \
    ../src/optionallayers/optionallayersmapwidget.cpp \
    ../src/optionallayers/toolbaroptionallayers.cpp \
    ../src/poilist/poilistmapwidget.cpp \
    ../src/poilist/toolbarpoilist.cpp \
    ../src/poilist/positioninputdialog.cpp \
    ../src/devtool/devtooldialog.cpp \
    ../src/mapcontainer.cpp \
    ../src/camera/cameramapwidget.cpp \
    ../src/camera/toolbarcamera.cpp \
    ../src/avatar/avatarmapwidget.cpp \
    ../src/avatar/toolbaravatar.cpp \
    ../../util/src/globalsetting.cpp \
    ../../util/src/util.cpp \
    ../../util/src/qaloguploaddialog.cpp

HEADERS  += \
    ../include/mainwindow.h \
    ../include/toolbarcommon.h \
    ../include/basicmap/basicmapwidget.h \
    ../include/basicmap/toolbarbasicmap.h \
    ../include/mainmenu/listdelegate.h \
    ../include/mainmenu/menuwidget.h \
    ../include/mainmenu/toolbarmenu.h \
    ../include/poipin/toolbarpoipin.h \
    ../include/poipin/poipinmapwidget.h \
    ../include/location/toolbarlocation.h \
    ../include/location/locationmapwidget.h \
    ../include/followme/followmemapwidget.h \
    ../include/followme/toolbarfollowme.h \
    ../include/followme/interpolator.h \
    ../include/optionallayers/optionallayersmapwidget.h \
    ../include/optionallayers/toolbaroptionallayers.h \
    ../include/poilist/poilistmapwidget.h \
    ../include/poilist/toolbarpoilist.h \
    ../include/poilist/positioninputdialog.h \
    ../include/devtool/devtooldialog.h \
    ../include/mapcontainer.h \
    ../include/camera/cameramapwidget.h \
    ../include/camera/toolbarcamera.h \
    ../include/avatar/avatarmapwidget.h \
    ../include/avatar/toolbaravatar.h \
    ../../util/include/globalsetting.h \
    ../../util/include/inifile.h \
    ../../util/include/util.h \
    ../../util/include/qaloguploaddialog.h \
    ../../util/include/serverconfig.h

FORMS    += \
    ../Forms/mainwindow.ui \
    ../Forms/positioninputdialog.ui \
    ../Forms/devtooldialog.ui \
    ../../util/Forms/qaloguploaddialog.ui
