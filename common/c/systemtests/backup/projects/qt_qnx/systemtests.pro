TEMPLATE = app
TARGET = systemtests

CONFIG += qt warn_on debug_and_release cascades
QT     += declarative script svg sql network
LIBS   += -lsocket -lscreen -lEGL -lGLESv1_CM -lpng -lbps -lz -lsqlite3 -lstrm -lmmrndclient -lbbsystem -lQtLocationSubset

INCLUDEPATH += ../../../src ../../../include/qt_qnx $(NIMARGTABLE_INC) $(NIMCUNIT_INC) \
$(QNX_TARGET)/usr/include/freetype2 ${QNX_TARGET}/usr/include/qt4/QtGui \
${QNX_TARGET}/usr/include/qt4/QtCore \
${QNX_TARGET}/usr/include/qt4/QtDeclarative \
${QNX_TARGET}/usr/include/qt4/QtScript \
${QNX_TARGET}/usr/include/qt4/QtSvg \
${QNX_TARGET}/usr/include/qt4/QtSql \
${QNX_TARGET}/usr/include/qt4/QtXmlPatterns \
${QNX_TARGET}/usr/include/qt4/QtXml \
${QNX_TARGET}/usr/include/qt4/QtNetwork \
${QNX_TARGET}/usr/include/GLES \
${QNX_TARGET}/usr/include/GLES2 \
$(QNX_TARGET)/usr/include/EGL \
$(QNX_TARGET)/usr/include/input \
$(QNX_TARGET)/usr/include $(NIMARGTABLE_INC) \
${QNX_TARGET}/usr/include/google \
${QNX_TARGET}/usr/include/google/protobuf \
${QNX_TARGET}/usr/include/google/protobuf/compiler \
${QNX_TARGET}/usr/include/google/protobuf/io \
${QNX_TARGET}/usr/include/google/protobuf/stubs \
${QNX_TARGET}/usr/include/google/protobuf/compiler/cpp \
${QNX_TARGET}/usr/include/google/protobuf/compiler/java \
${QNX_TARGET}/usr/include/google/protobuf/compiler/python \
$(NIMNBIPAL_INC) \
$(NIMNBIPAL_INC)/qt_qnx  \
$(NIMNBIPAL_INC)/private  \
$(NIMNBIPAL_INC)/nbpal  \
$(NIMNBIPAL_INC)/qt_qnx/unzip  \
$(NIMABPAL_INC) $(NIMABPAL_INC)/qt_qnx $(NIMCORE_INC) $(NIMCORE_INC)/qt_qnx $(NIMCORE_INC)/protected \
$(NIMNB_INC) $(NIMNB_INC)/qt_qnx $(NIMNB_INC)/protected $(NIMAB_INC) $(NIMAB_INC)/qt_qnx \
$(NIMNBUI_INC) $(NIMNBUI_INC)/qt_qnx \
$(NIMPLACE_INC) \
$(NIMNBGMMAIN_INC) $(NIMNBGMMAIN_INC)/qt_qnx \


SOURCES += 	../../../src/argtableutil.c \
			../../../src/main.c \
			../../../src/namebitmaskmap.c \
			../../../src/testanalytics.c \
			../../../src/testbatchtool.cpp \
			../../../src/testbatchtoolprocessor.cpp \
			../../../src/testdatastore.c \
			../../../src/testdirection.c \
			../../../src/testenhancedcontent.c \
			../../../src/testers.c \
			../../../src/testfileset.c \
			../../../src/testgeocode.c \
			../../../src/testgpstypes.c \
			../../../src/testlicense.c \
			../../../src/testlocationservices.c \
			../../../src/testmotd.c \
			../../../src/testnavigation.c \
			../../../src/testnetwork.c \
			../../../src/testpoiimagemanager.cpp \
			../../../src/testprofile.c \
			../../../src/testpublictransit.c \
			../../../src/testqalog.c \
			../../../src/testrastermap.c \
			../../../src/testrastertile.c \
			../../../src/testreversegeocode.c \
			../../../src/testsearchbuilder.c \
			../../../src/testshare.c \
			../../../src/testsinglesearch.c \
			../../../src/TestSingleSearchServices.cpp \
			../../../src/testsms.c \
			../../../src/testspatial.c \
			../../../src/testspeech.c \
			../../../src/testspeedcameras.c \
			../../../src/testsubscription.c \
			../../../src/testsync.c \
			../../../src/testtileservice.c \
			../../../src/testtristrip.c \
			../../../src/testvectormap.c \
			../../../src/utility.c \
			../../../src/TestLocation.cpp \
            ../../../src/TestInstance.cpp \
            ../../../src/TestLayerManager.cpp \
            ../../../src/testmapview.cpp \
			../../../src/qt_qnx/*.cpp

HEADERS += ../../../src/*.h ../../../src/qt_qnx/*.h


device {
	PROFILE = arm
	CONFIG(release, debug|release) {
		PATHSUFFIX = a-le-v7
		DESTDIR = o.le-v7
	}
	CONFIG(debug, debug|release) {
		PATHSUFFIX = a-le-v7-g
		LIBSUFFIX = _g
		DESTDIR = o.le-v7-g
	}
	LIBS += -L$(QNX_TARGET)/armle-v7/usr/lib \
	-L$(CORENBM_LIB)/$${PROFILE}/$${PATHSUFFIX} \
	-L$(NIMPLACE_LIB)/$${PROFILE}
}

simulator {
	PROFILE = x86
	CONFIG(release, debug|release) {
		PATHSUFFIX = a
		DESTDIR = o
	}
	CONFIG(debug, debug|release) {
		PATHSUFFIX = a-g
		LIBSUFFIX = _g
		DESTDIR = o-g
	}
	DEFINES += ON_SIMULATOR

	LIBS += -L$(QNX_TARGET)/x86/usr/lib \
	-L$(CORENBM_LIB)/$${PROFILE}/$${DESTDIR} \
	-L$(NIMPLACE_LIB)/$${PROFILE}

	
}

#*****Warning Do not move Rim Libs since it will crash at boot up*****
device {
  QMAKE_LFLAGS = -L${MVC_SDK_ARM_LIB} $$QMAKE_LFLAGS
  LIBS += -lRenderEngine3d -lbbcascadesmaps
  
}
simulator {
  QMAKE_LFLAGS = -L${MVC_SDK_X86_LIB} $$QMAKE_LFLAGS
  LIBS += -lRenderEngine3d -lbbcascadesmaps
  
}



LIBS += -L../../../../../thirdparty/feature_130313_map3d_rel2_1/projects/qt_qnx/CUnit/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../thirdparty/feature_130313_map3d_rel2_1/projects/qt_qnx/ArgTable/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../thirdparty/feature_130313_map3d_rel2_1/projects/qt_qnx/speex/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../thirdparty/feature_130313_map3d_rel2_1/projects/qt_qnx/jpeg_turbo/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbpal/feature_130313_map3d_rel2_1/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../abpal/feature_130313_map3d_rel2_1/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbservices/feature_130313_map3d_rel2_1/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../coreservices/feature_130313_map3d_rel2_1/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../abservices/feature_130313_map3d_rel2_1/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbui/feature_130313_map3d_rel2_1/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \
-L../../../../../nbgm/feature_130313_map3d_rel2_1/nbgmmain/projects/qt_qnx/$${PROFILE}/$${PATHSUFFIX} \



LIBS += -labservices$${LIBSUFFIX} -lcunit$${LIBSUFFIX} -lnbservices$${LIBSUFFIX} \
-lcoreservices$${LIBSUFFIX} -labpal$${LIBSUFFIX} -lnbpal$${LIBSUFFIX} -largtable$${LIBSUFFIX} \
-lNBM$${LIBSUFFIX}  -lnbui$${LIBSUFFIX} -lspeex$${LIBSUFFIX} -lnbgm$${LIBSUFFIX} \
-lcurl -lpps -lbbsystem  -lbbplatformplaces  -lbbplatform -lbbcascadesplaces -lfreetype -lfontconfig -lssl \
-lcrypto -lslog2 -lGLESv2 -ljpeg_turbo$${LIBSUFFIX}

OBJECTS_DIR = $${DESTDIR}
