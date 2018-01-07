
debug {
    DEFINES += DEBUG_LOG_ALL
    DEFINES += _DEBUG_LOGGING_ENABLED_
}

qnx {
    QMAKE_CFLAGS -= -Wc,-std=c++11
    QMAKE_CXXFLAGS += -Wno-unused-parameter
    QMAKE_CXXFLAGS += -Wno-unused-variable
} else:unix {
    QMAKE_CXXFLAGS += -std=c++11
    QMAKE_CXXFLAGS += -Wno-unused-parameter
    QMAKE_CXXFLAGS += -Wno-unused-variable
}

CONFIG(debug, debug|release) {
    BUILD_CONFIG = debug
}else{
    BUILD_CONFIG = release
}

MOC_DIR += moc
CONFIG += debug_and_release debug_and_release_target

ltkdev {
    qnx {
      PLATFORM=qnx
    } else:unix {
      PLATFORM=unix
    } else {
      PLATFORM=unknown
    }
    OBJECTS_DIR += ./obj/$$PLATFORM/$$BUILD_CONFIG/$$QT_VERSION
    DESTDIR = $$CCC_ROOT/build/$$BRANCH/output/lib/qt/$$PLATFORM/$$BUILD_CONFIG/$$QT_VERSION
} else {
    OBJECTS_DIR += ./obj/$$BUILD_CONFIG
    DESTDIR = $$CCC_ROOT/build/$$BRANCH/output/lib/qt/$$BUILD_CONFIG
}
