
######### Environment setting ############
BRANCH = $$dirname(PWD)
BRANCH = $$basename(BRANCH)
include (../../../build/$$BRANCH/build_library.pro)

######### Project setting ############l
QT       += gui
INCLUDEPATH += ../include/private
INCLUDEPATH += ../include

SOURCES += \
    ../src/regionallistcontroller.cpp \
    ../src/regionallistitemwidget.cpp \
    ../src/regionallistwidget.cpp

HEADERS  += \
    ../include/regionallistwidget.h \
    ../include/private/regionallistcontroller.h \
    ../include/private/regionallistitemwidget.h

OTHER_FILES += \
    ../resource/images/cancel.png \
    ../resource/images/download.png
