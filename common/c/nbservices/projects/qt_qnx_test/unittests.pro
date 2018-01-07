TEMPLATE = app
TARGET = unittests

target.path = $$[QT_INSTALL_EXAMPLES]/unittests
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS unittests.pro
sources.path += $$[QT_INSTALL_EXAMPLES]/unittests
CONFIG += debug_and_release
INSTALLS += target sources

INCLUDEPATH += ../../../src/unittests/ ../../../src/unittests/qt_qnx \
../../../../../nbservices/$(CCI_QNX_BRANCH)/include \
../../../../../nbservices/$(CCI_QNX_BRANCH)/include/qt_qnx \
../../../../../nbservices/$(CCI_QNX_BRANCH)/include/generated \
../../../../../nbservices/$(CCI_QNX_BRANCH)/src/enhancedcontent \
../../../../../nbservices/$(CCI_QNX_BRANCH)/include/private \
../../../../../nbservices/$(CCI_QNX_BRANCH)/include/protected \
../../../../../nbservices/$(CCI_QNX_BRANCH)/src/map \
../../../../../nbservices/$(CCI_QNX_BRANCH)/src/map/mapview \
../../../../../nbservices/$(CCI_QNX_BRANCH)/src/map/layermanager \
../../../../../nbservices/$(CCI_QNX_BRANCH)/src/map/tilemanager \
../../../../../nbpal/$(CCI_QNX_BRANCH)/include \
../../../../../nbpal/$(CCI_QNX_BRANCH)/include/qt_qnx \
../../../../../thirdparty/$(CCI_QNX_BRANCH)/CUnit/include \
../../../../../coreservices/$(CCI_QNX_BRANCH)/include \
../../../../../coreservices/$(CCI_QNX_BRANCH)/include/private \
../../../../../coreservices/$(CCI_QNX_BRANCH)/include/protected \
../../../../../coreservices/$(CCI_QNX_BRANCH)/include/qt_qnx \
$(CORENBM_INC)


SOURCES += ../../../src/unittests/*.c \
../../../src/unittests/*.cpp \
../../../src/unittests/qt_qnx/*.cpp

HEADERS += ../../../src/unittests/*.h ../../../src/unittests/qt_qnx/*.h

LIBS += -lsocket -lsqlite3  -lbps

device {
	PROFILE = arm
	QMAKE_LFLAGS = -L${MVC_SDK_LIB} $$QMAKE_LFLAGS
	CONFIG(release, debug|release) {
		PATHSUFFIX = a-le-v7
		DESTDIR = o.le-v7
	}
	CONFIG(debug, debug|release) {
		PATHSUFFIX = a-le-v7-g
		LIBSUFFIX = _g
		DESTDIR = o.le-v7-g
	}

	LIBS += -L$(QNX_TARGET)/../target-override/armle-v7/usr/lib \
	-L$(QNX_TARGET)/armle-v7/usr/lib \
	-L$(NIMPLACE_LIB)/$${PROFILE} \
	-L$(CORENBM_LIB)/$${PROFILE}/$${PATHSUFFIX}

	LIBS += -lcunit$${LIBSUFFIX} -lnbservices$${LIBSUFFIX} -lnbgm$${LIBSUFFIX} -labservices$${LIBSUFFIX} \
	-lcoreservices$${LIBSUFFIX} -labpal$${LIBSUFFIX} -lnbpal$${LIBSUFFIX} -largtable$${LIBSUFFIX} \
	-lpng14 -lz -lNBM$${LIBSUFFIX}  -lnbui$${LIBSUFFIX} \
	-lGLESv1_CL -lGLESv1_CM -lGLESv2 -lglview -lbbcascadesplaces -lbbplatformplaces\
	-lpps -lgoogleurl -lbbsystem  -lbbcascadesmaps -lscreen\
    -lbbcascades -lQtDeclarative -lQtScript -lcurl -lslog2 -lcrypto\
	-lQtSvg -lQtSql -lQtXml -lQtXmlPatterns -lQtNetwork
}

simulator {
	PROFILE = x86
	QMAKE_LFLAGS = -L${MVC_SDK_X86_LIB} $$QMAKE_LFLAGS
	CONFIG(release, debug|release) {
		PATHSUFFIX = a
		DESTDIR = o
	}
	CONFIG(debug, debug|release) {
		PATHSUFFIX = a-g
		LIBSUFFIX = _g
		DESTDIR = o-g
	}
	LIBS += -L$(QNX_TARGET)/../target-override/x86/usr/lib \
	-L$(NIMPLACE_LIB)/$${PROFILE} \
	-L$(CORENBM_LIB)/$${PROFILE}/$${DESTDIR}
    LIBS += -lcunit$${LIBSUFFIX} -lnbservices$${LIBSUFFIX} -lnbgm$${LIBSUFFIX} -labservices$${LIBSUFFIX} \
    -lcoreservices$${LIBSUFFIX} -labpal$${LIBSUFFIX} -lnbpal$${LIBSUFFIX} -largtable$${LIBSUFFIX} \
    -lpng14 -lz -lNBM$${LIBSUFFIX}  -lnbui$${LIBSUFFIX} \
    -lGLESv1_CL -lGLESv1_CM -lGLESv2 -lglview -lbbcascadesplaces -lbbplatformplaces\
    -lpps -lgoogleurl -lbbsystem  -lbbcascadesmaps -lscreen\
    -lbbcascades -lQtDeclarative -lQtScript -lcurl -lslog2 -lcrypto\
    -lQtSvg -lQtSql -lQtXml -lQtXmlPatterns -lQtNetwork
}

LIBS += -L../../../../../thirdparty/feature_130725_map3d_main/projects/qt_qnx/CUnit/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../thirdparty/feature_130725_map3d_main/projects/qt_qnx/ArgTable/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbpal/feature_130725_map3d_main/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../abpal/feature_130725_map3d_main/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbservices/feature_130725_map3d_main/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../coreservices/feature_130725_map3d_main/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../abservices/feature_130725_map3d_main/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbui/feature_130725_map3d_main/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbgm/feature_130725_map3d_main/nbgmmain/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX}

OBJECTS_DIR = $${DESTDIR}
