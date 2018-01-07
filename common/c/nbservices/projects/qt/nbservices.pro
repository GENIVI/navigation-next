CCC_ROOT = ../../..
PROJ_ROOT = $$CCC_ROOT/nbservices

THIRDPARTY_ROOT = $$CCC_ROOT/thirdparty
ABIPAL_ROOT = $$CCC_ROOT/abpal
NBIPAL_ROOT = $$CCC_ROOT/nbpal
NIMCORESERVICES_ROOT = $$CCC_ROOT/coreservices
NBIGM_ROOT = $$CCC_ROOT/nbgm
NBUI_ROOT = $$CCC_ROOT/nbui
BUILD_ROOT = $$CCC_ROOT/build
COMMON_ROOT = $$CCC_ROOT/common

INCLUDEPATH = $$PROJ_ROOT/include
INCLUDEPATH += $$PROJ_ROOT/include/private
INCLUDEPATH += $$PROJ_ROOT/include/qt
INCLUDEPATH += $$PROJ_ROOT/include/protected
INCLUDEPATH += $$PROJ_ROOT/include/generated
INCLUDEPATH += $$PROJ_ROOT/src/map/mapview
INCLUDEPATH += $$PROJ_ROOT/src/map/layermanager
INCLUDEPATH += $$PROJ_ROOT/src/map/tilemanager
INCLUDEPATH += $$PROJ_ROOT/src/map/unified
INCLUDEPATH += $$NBIPAL_ROOT/include
INCLUDEPATH += $$NBIPAL_ROOT/include/qt
INCLUDEPATH += $$ABIPAL_ROOT/include
INCLUDEPATH += $$ABIPAL_ROOT/include/qt
INCLUDEPATH += $$NIMCORESERVICES_ROOT/include
INCLUDEPATH += $$NIMCORESERVICES_ROOT/include/qt
INCLUDEPATH += $$NIMCORESERVICES_ROOT/include/protected
INCLUDEPATH += $$NBIGM_ROOT/nbgmmain/include
INCLUDEPATH += $$NBIGM_ROOT/nbgmmain/include/protected
INCLUDEPATH += $$NBIGM_ROOT/nbgmmain/include/qt
INCLUDEPATH += $$NBUI_ROOT/include/protected
INCLUDEPATH += $$THIRDPARTY_ROOT/sqlite/include
INCLUDEPATH += $$THIRDPARTY_ROOT
INCLUDEPATH += $$BUILD_ROOT/nbm/include
INCLUDEPATH += $$BUILD_ROOT/ncdb/include
INCLUDEPATH += $$THIRDPARTY_ROOT/tinyxml
INCLUDEPATH += $$THIRDPARTY_ROOT/libxml2
INCLUDEPATH += $$THIRDPARTY_ROOT/libxml2/include
linux {
}
qnx {
INCLUDEPATH += $$THIRDPARTY_ROOT/libpng-1.4
}

INCLUDEPATH += $$COMMON_ROOT/include
INCLUDEPATH += $$COMMON_ROOT/include/qt
INCLUDEPATH += $$COMMON_ROOT/include/private
INCLUDEPATH += $$COMMON_ROOT/include/protected
INCLUDEPATH += $$PROJ_ROOT/src/analytics

HEADERS += $$PROJ_ROOT/include/*.h
HEADERS += $$PROJ_ROOT/include/private/*.h
HEADERS += $$PROJ_ROOT/include/qt/*.h
HEADERS += $$PROJ_ROOT/include/protected/*.h
HEADERS += $$PROJ_ROOT/include/generated/*.h
HEADERS += $$PROJ_ROOT/src/map/mapview/*.h
HEADERS += $$PROJ_ROOT/src/map/layermanager/*.h
HEADERS += $$PROJ_ROOT/src/map/tilemanager/*.h
HEADERS += $$PROJ_ROOT/src/map/unified/*.h
HEADERS += $$PROJ_ROOT/src/analytics/*.h

SOURCES += $$PROJ_ROOT/src/analytics/*.c
SOURCES += $$PROJ_ROOT/src/analytics/*.cpp
SOURCES += $$PROJ_ROOT/src/data/*.c
SOURCES += $$PROJ_ROOT/src/enhancedcontent/*.c
SOURCES += $$PROJ_ROOT/src/geocode/*.c
SOURCES += $$PROJ_ROOT/src/gps/*.c
SOURCES += $$PROJ_ROOT/src/location/*.c
SOURCES += $$PROJ_ROOT/src/locationservices/*.cpp
SOURCES += $$PROJ_ROOT/src/map/layermanager/*.cpp
SOURCES += $$PROJ_ROOT/src/map/mapview/*.cpp
SOURCES += $$PROJ_ROOT/src/map/tilemanager/*.cpp
SOURCES += $$PROJ_ROOT/src/map/unified/*.cpp
SOURCES += $$PROJ_ROOT/src/mobilecoupons/*.c
SOURCES += $$PROJ_ROOT/src/nav/*.c
SOURCES += $$PROJ_ROOT/src/nav/*.cpp
SOURCES += $$PROJ_ROOT/src/nav/enhanced_content/*.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/nbhybridmanager.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/HybridManager.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/HybridStrategy.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/ncdbobjectsmanager.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/Cache.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/CacheTasks.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/CachingEntry.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/CachingEntryDatabaseProcessor.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/CachingIndex.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/CommonTypes.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/datastream.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/datastreamimplementation.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/downloadrequest.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/FileOperatingTaskManager.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/httpdownloadmanager.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/nbcontext.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/nbformat.c
SOURCES += $$PROJ_ROOT/src/nbcommon/nbpersistentdata.c
SOURCES += $$PROJ_ROOT/src/nbcommon/nbpointiteration.c
SOURCES += $$PROJ_ROOT/src/nbcommon/nbrouteid.c
SOURCES += $$PROJ_ROOT/src/nbcommon/nbsolarcalculator.c
SOURCES += $$PROJ_ROOT/src/nbcommon/nbtristrip.c
SOURCES += $$PROJ_ROOT/src/nbcommon/nbutility.c
SOURCES += $$PROJ_ROOT/src/nbcommon/StringUtility.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/urlparser.cpp
SOURCES += $$PROJ_ROOT/src/nbcommon/workerqueuemanager.cpp
SOURCES += $$PROJ_ROOT/src/nbimage/*.c
SOURCES += $$PROJ_ROOT/src/network/*.c
SOURCES += $$PROJ_ROOT/src/network/nbnetworkconfiguration.cpp
SOURCES += $$PROJ_ROOT/src/poiimagemanager/*.cpp
SOURCES += $$PROJ_ROOT/src/protocol/*.cpp
SOURCES += $$PROJ_ROOT/src/proxpoi/*.c
SOURCES += $$PROJ_ROOT/src/publictransitmanager/*.c
SOURCES += $$PROJ_ROOT/src/qalog/*.c
SOURCES += $$PROJ_ROOT/src/qalog/nbqalog.cpp
SOURCES += $$PROJ_ROOT/src/rastermap/*.c
SOURCES += $$PROJ_ROOT/src/rastertile/*.c
SOURCES += $$PROJ_ROOT/src/singlesearch/*.c
SOURCES += $$PROJ_ROOT/src/singlesearch/SingleSearch.cpp
SOURCES += $$PROJ_ROOT/src/spatial/*.c
SOURCES += $$PROJ_ROOT/src/tileservice/*.c
SOURCES += $$PROJ_ROOT/src/util/*.c
SOURCES += $$PROJ_ROOT/src/util/*.cpp
SOURCES += $$PROJ_ROOT/src/vectortile/*.c
SOURCES += $$PROJ_ROOT/src/wifiprobes/*.c
SOURCES += $$PROJ_ROOT/src/onboardcontentmanager/*.cpp

win32: INCLUDEPATH += $$THIRDPARTY_ROOT/libxml2/include/platforms/win32
unix: INCLUDEPATH += $$THIRDPARTY_ROOT/libxml2/include/platforms/linux
win32: DEFINES += MAKE_STATIC_LIBRARIES

debug:linux{
    DEFINES += MPERF
    DEFINES += DNCDB
}

TEMPLATE = lib
TARGET = nbservices
CONFIG += staticlib
CONFIG += debug_and_release debug_and_release_target
CONFIG += rtti
DEFINES += MAKE_STATIC_LIBRARIES
DEFINES += IN_LIBXML
QMAKE_CFLAGS += -std=c99

debug:DESTDIR = $$CCC_ROOT/build/output/lib/qt/debug/
release:DESTDIR = $$CCC_ROOT/build/output/lib/qt/release/

include ($$CCC_ROOT/build/projects/qt/config.pro)

