CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}

######### CoreSDK ############
ltkdev {
    CORESDK_ROOT = $$LTK_ROOT/common/c

    NIM_ABPAL_ROOT = $$CORESDK_ROOT/abpal
    NIM_ABPAL_INCLUDE = $$NIM_ABPAL_ROOT/include \
            $$NIM_ABPAL_ROOT/include/qt

    NIM_COMMON_ROOT = $$CORESDK_ROOT/common/include
    NIM_COMMON_INCLUDE = $$NIM_COMMON_ROOT \
            $$NIM_COMMON_ROOT/qt

    NIM_MAP_ROOT = $$CORESDK_ROOT/map/include
    NIM_MAP_INCLUDE = $$NIM_MAP_ROOT

    NIM_NAV_ROOT = $$CORESDK_ROOT/nav/include
    NIM_NAV_INCLUDE = $$NIM_NAV_ROOT

    NIM_NAVUI_ROOT = $$CORESDK_ROOT/navui/include
    NIM_NAVUI_INCLUDE = $$NIM_NAVUI_ROOT \
                        $$NIM_NAVUI_ROOT/widgets

    NIM_NBPAL_ROOT = $$CORESDK_ROOT/nbpal/include
    NIM_NBPAL_INCLUDE += $$NIM_NBPAL_ROOT \
            $$NIM_NBPAL_ROOT/qt

    NIM_NBGM_ROOT = $$CORESDK_ROOT/nbgm/nbgmmain/include
    NIM_NBGM_INCLUDE += $$NIM_NBGM_ROOT \
            $$NIM_NBGM_ROOT/qt

    NIM_NBUI_ROOT = $$CORESDK_ROOT/nbui/include
    NIM_NBUI_INCLUDE += $$NIM_NBUI_ROOT \
            $$NIM_NBUI_ROOT/qt \
            $$NIM_NBUI_ROOT/qt/map \
            $$NIM_NBUI_ROOT/qt/template

    NIM_ABSERVICES_ROOT = $$CORESDK_ROOT/abservices/include
    NIM_ABSERVICES_INCLUDE = $$NIM_ABSERVICES_ROOT \
                             $$NIM_ABSERVICES_ROOT/qt

    NIM_CORESERVICES_ROOT = $$CORESDK_ROOT/coreservices/include
    NIM_CORESERVICES_INCLUDE = $$NIM_CORESERVICES_ROOT \
                            $$NIM_CORESERVICES_ROOT/qt

    NIM_NBSERVICES_ROOT = $$CORESDK_ROOT/nbservices/include
    NIM_NBSERVICES_INCLUDE = $$NIM_NBSERVICES_ROOT \
                            $$NIM_NBSERVICES_ROOT/qt

    NIM_SEARCH_ROOT = $$CORESDK_ROOT/search/include
    NIM_SEARCH_INCLUDE = $$NIM_SEARCH_ROOT

} else {
    CORESDK_ROOT = $$LTK_ROOT/build/qt/CoreSDK

    NIM_ABPAL_ROOT = $$CORESDK_ROOT/include/abpal
    NIM_ABPAL_INCLUDE = $$NIM_ABPAL_ROOT \
            $$NIM_ABPAL_ROOT/qt

    NIM_COMMON_ROOT = $$CORESDK_ROOT/include/common
    NIM_COMMON_INCLUDE = $$NIM_COMMON_ROOT \
            $$NIM_COMMON_ROOT/qt

    NIM_MAP_ROOT = $$CORESDK_ROOT/include/map
    NIM_MAP_INCLUDE = $$NIM_MAP_ROOT

    NIM_NAV_ROOT = $$CORESDK_ROOT/include/nav
    NIM_NAV_INCLUDE = $$NIM_NAV_ROOT

    NIM_NAVUI_ROOT = $$CORESDK_ROOT/include/navui
    NIM_NAVUI_INCLUDE = $$NIM_NAVUI_ROOT \
                        $$NIM_NAVUI_ROOT/widgets

    NIM_NBPAL_ROOT = $$CORESDK_ROOT/include/nbpal
    NIM_NBPAL_INCLUDE += $$NIM_NBPAL_ROOT
    NIM_NBPAL_INCLUDE += $$NIM_NBPAL_ROOT/qt

    NIM_NBGM_ROOT = $$CORESDK_ROOT/include/nbgm
    NIM_NBGM_INCLUDE += $$NIM_NBGM_ROOT \
            $$NIM_NBGM_ROOT/qt

    NIM_NBUI_ROOT = $$CORESDK_ROOT/include/nbui
    NIM_NBUI_INCLUDE += $$NIM_NBUI_ROOT \
            $$NIM_NBUI_ROOT/qt \
            $$NIM_NBUI_ROOT/qt/map \
            $$NIM_NBUI_ROOT/qt/template

    NIM_ABSERVICES_ROOT = $$CORESDK_ROOT/include/abservices
    NIM_ABSERVICES_INCLUDE = $$NIM_ABSERVICES_ROOT \
                             $$NIM_ABSERVICES_ROOT/qt

    NIM_CORESERVICES_ROOT = $$CORESDK_ROOT/include/coreservices
    NIM_CORESERVICES_INCLUDE = $$NIM_CORESERVICES_ROOT \
                            $$NIM_CORESERVICES_ROOT/qt

    NIM_NBSERVICES_ROOT = $$CORESDK_ROOT/include/nbservices
    NIM_NBSERVICES_INCLUDE = $$NIM_NBSERVICES_ROOT \
                            $$NIM_NBSERVICES_ROOT/qt

    NIM_SEARCH_ROOT = $$CORESDK_ROOT/include/search
    NIM_SEARCH_INCLUDE = $$NIM_SEARCH_ROOT
}

CORESDK_INCLUDE = $$NIM_ABPAL_INCLUDE \
    $$NIM_COMMON_INCLUDE \
    $$NIM_NAVUI_INCLUDE \
    $$NIM_NAV_INCLUDE\
    $$NIM_MAP_INCLUDE \
    $$NIM_NBPAL_INCLUDE \
    $$NIM_NBGM_INCLUDE \
    $$NIM_NBUI_INCLUDE \
    $$NIM_ABSERVICES_INCLUDE \
    $$NIM_CORESERVICES_INCLUDE \
    $$NIM_NBSERVICES_INCLUDE \
    $$NIM_SEARCH_INCLUDE


######### LTK ############
LTK_COMMON_INCLUDE = $$LTK_ROOT/common/qt/include \
                     $$LTK_ROOT/common/qt/include/protected \
                     $$LTK_ROOT/common/qt/include/data
LTK_LOCATIONKIT_INCLUDE = $$LTK_ROOT/locationkit/qt/include
LTK_MAP3D_INCLUDE = $$LTK_ROOT/mapkit3d/qt/include \
                    $$LTK_ROOT/mapkit3d/qt/include/model \
                    $$LTK_ROOT/mapkit3d/qt/include/private

LTK_INCLUDE = $$LTK_COMMON_INCLUDE  \
    $$LTK_LOCATIONKIT_INCLUDE \
    $$LTK_MAP3D_INCLUDE


######### Library Setting ############
QT       = core gui qml quick

INCLUDEPATH += $$LTK_INCLUDE $$CORESDK_INCLUDE
MOC_DIR += ../src/moc
DEPENDPATH += ../src


TEMPLATE = lib
CONFIG += staticlib debug_and_release debug_and_release_target
DEFINES += MAKE_STATIC_LIBRARIES


unix {
  QMAKE_CXXFLAGS += -std=c++11
  QMAKE_CXXFLAGS += -Wno-unused-parameter
  QMAKE_CXXFLAGS += -Wno-unused-variable
}

ltkdev {
    qnx {
      PLATFORM=qnx
    } else:unix {
      PLATFORM=unix
    } else {
      PLATFORM=unknown
    }
    OBJECTS_DIR += ./obj/$$PLATFORM/$$BUILD_CONFIG/$$QT_VERSION
    DESTDIR = $$LTK_ROOT/build/output/qt/$$PLATFORM/$$BUILD_CONFIG/lib/$$QT_VERSION
} else {
    OBJECTS_DIR += ./obj/$$BUILD_CONFIG
    DESTDIR = $$LTK_ROOT/build/qt/output/$$BUILD_CONFIG/lib
}
