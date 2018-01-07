#-------------------------------------------------
#
# Project created by QtCreator 2014-09-23T17:39:08
#
#-------------------------------------------------

LTK_ROOT = ../../../../sampleapp/library
include ($$LTK_ROOT/locationtoolkit.pro)

#############################
MOC_DIR += ../src/moc
TEMPLATE = app
TARGET = navkit_sample

CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}

OBJECTS_DIR += ./obj/$$BUILD_CONFIG

#####################################################
INCLUDEPATH += ../include
INCLUDEPATH += ../ttsplayer/include
unix{
INCLUDEPATH += ../playwave/include
}
INCLUDEPATH += ../ttsplayer/include/flite
INCLUDEPATH += ../../util/include

SOURCES += \
    ../src/main.cpp\
    ../src/mainwindow.cpp \
    ../src/private/waitdialog.cpp \
    ../src/mainmenu/navigationmenu.cpp \
    ../src/maneuverlist/maneuverslist.cpp \
    ../src/private/interpolator.cpp \
    ../src/private/manlistdelegate.cpp \
    ../src/private/navigationsession.cpp \
    ../ttsplayer/src/*.cpp \
    ../../util/src/globalsetting.cpp \
    ../../util/src/qaloguploaddialog.cpp \
    ../../util/src/util.cpp

unix{
SOURCES += ../playwave/src/*.c
}

HEADERS  += \
    ../include/mainwindow.h \
    ../include/private/waitdialog.h \
    ../include/mainmenu/navigationmenu.h \
    ../include/maneuverlist/maneuverslist.h \
    ../include/private/interpolator.h \
    ../include/private/manlistdelegate.h \
    ../include/private/navigationsession.h \
    ../ttsplayer/include/*.h \
    ../../util/include/globalsetting.h \
    ../../util/include/inifile.h \
    ../../util/include/qaloguploaddialog.h \
    ../../util/include/util.h \
    ../../util/include/serverconfig.h
unix{
HEADERS += ../playwave/include/*h
}
FORMS    += \
    ../Forms/mainwindow.ui \
    ../Forms/waitdialog.ui \
    ../Forms/navigationmenu.ui \
    ../Forms/maneuverlist.ui \
    ../../util/Forms/qaloguploaddialog.ui

RESOURCES += \
    ../resource/images/images.qrc

OTHER_FILES +=
