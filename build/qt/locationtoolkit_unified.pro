######### CoreSDK ############


ltkdev {
    qnx {
      PLATFORM=qnx
    } else:unix {
      PLATFORM=unix
    } else {
      PLATFORM=unknown
    }

    CONFIG(debug, debug|release) {
        BUILD_CONFIG = debug
    }else{
        BUILD_CONFIG = release
    }

    CORESDK_LIBRARY += -L$$LTK_ROOT/common/c/build/output/lib/qt/$$BUILD_CONFIG/ -L$$LTK_ROOT/common/c/build/ncdb/libs -L$$LTK_ROOT/common/c/build/nbm/libs
    LTK_LIBRARY +=     -L$$LTK_ROOT/build/qt/output/$$BUILD_CONFIG/lib/
    #CORESDK_LIBRARY += -L$$LTK_ROOT/common/c/build/output/lib/qt/$$PLATFORM/$$BUILD_CONFIG/$$QT_VERSION
    #LTK_LIBRARY +=     -L$$LTK_ROOT/build/output/qt/$$PLATFORM/$$BUILD_CONFIG/lib/$$QT_VERSION
} else {
    CORESDK_LIBRARY += -L$$LTK_ROOT/lib
}


# TODO: factor out common lib of all platforms
qnx {
    CORESDK_LIBRARY += -lnavui -lnav -lmap -lsearch -lcommon -labservices -lnbservices -lnbui -lcoreservices -lnbgm -labpal -ltinyxml -lNBM -lxml2
    CORESDK_LIBRARY += -lnbpal -lfreetype -lasound -lz -lsqlite3 -lEGL -lpng -lcurl -lcrypto -lncdb -lscreen
    # no libnavigationuikit
    LTK_LIBRARY += -lnavigationkit -lmapkit3d -lltkcommon -llocationkit -lsearchkit -lscreen
    message("qnx")
} else:unix {
    CORESDK_LIBRARY += -L/usr/local/libpng/lib
    CORESDK_LIBRARY += -lnavui -lnav -lmap -lsearch -lcommon -labservices -lnbservices -lnbui -lcoreservices -lnbgm -labpal -ltinyxml -lxml2 -lncdb
    CORESDK_LIBRARY += -lflite_cmu_us_slt -lflite_cmu_us_rms -lflite_usenglish -lflite_cmulex -lflite -lnbpal -lfreetype -lz -lsqlite3 -lEGL -ldl -lrt
    LTK_LIBRARY += -lnavigationkit -lmapkit3d -lsearchkit -lltkcommon -llocationkit
    message("unix")
} else:win32{
    CORESDK_LIBRARY += -lnavui -lnav -lmap -lsearch -lcommon -labservices -lnbservices -lnbui -lcoreservices -lnbgm -labpal -ltinyxml -lNBM -lxml2 -lncdb -lpng15
    CORESDK_LIBRARY += -lflite -lwinmm -ltaskqueue -lglew32 -lOpenGL32 -lgdi32 -lzlib -lnbpal
    LTK_LIBRARY += -lnavigationkit -lmapkit3d -lltkcommon -llocationkit -lsearchkit
    message("win32")
} else {
    message("unknown")
}


######### LTK ############
ltkdev {
    LTK_COMMON_INCLUDE = $$LTK_ROOT/common/qt/include \
                         $$LTK_ROOT/common/qt/include/protected \
                         $$LTK_ROOT/common/qt/include/data
    LTK_LOCATIONKIT_INCLUDE = $$LTK_ROOT/locationkit/qt/include
    LTK_MAP3D_INCLUDE = $$LTK_ROOT/mapkit3d/qt/include \
                        $$LTK_ROOT/mapkit3d/qt/include/model \
                        $$LTK_ROOT/mapkit3d/qt/include/private
    LTK_NAVKIT_INCLUDE = $$LTK_ROOT/navigationkit/qt/include \
                         $$LTK_ROOT/navigationkit/qt/include/data \
                        $$LTK_ROOT/navigationkit/qt/include/signal
    LTK_NAVUIKIT_INCLUDE = $$LTK_ROOT/navigationuikit/qt/include \
                         $$LTK_ROOT/navigationuikit/qt/include/data \
                        $$LTK_ROOT/navigationuikit/qt/include/signal
    LTK_SEARCHKIT_INCLUDE = $$LTK_ROOT/searchkit/qt/include

} else {
    LTK_COMMON_INCLUDE = $$LTK_ROOT/include/common \
                         $$LTK_ROOT/include/common/data
    LTK_LOCATIONKIT_INCLUDE = $$LTK_ROOT/include/locationkit
    LTK_MAP3D_INCLUDE = $$LTK_ROOT/include/mapkit3d \
                        $$LTK_ROOT/include/mapkit3d/model
    LTK_NAVKIT_INCLUDE = $$LTK_ROOT/include/navigationkit \
                         $$LTK_ROOT/include/navigationkit/data \
                        $$LTK_ROOT/include/navigationkit/signal
    LTK_NAVUIKIT_INCLUDE = $$LTK_ROOT/include/navigationuikit \
                         $$LTK_ROOT/include/navigationuikit/data \
                        $$LTK_ROOT/include/navigationuikit/signal
    LTK_SEARCHKIT_INCLUDE = $$LTK_ROOT/include/searchkit
}

LTK_INCLUDE = $$LTK_COMMON_INCLUDE  \
        $$LTK_LOCATIONKIT_INCLUDE \
        $$LTK_MAP3D_INCLUDE \
        $$LTK_NAVKIT_INCLUDE \
        $$LTK_SEARCHKIT_INCLUDE

######### APP Setting ############
QT       += core gui network qml quick

LIBS += $$LTK_LIBRARY \
        $$CORESDK_LIBRARY
INCLUDEPATH += $$LTK_INCLUDE
CONFIG += debug_and_release debug_and_release_target
