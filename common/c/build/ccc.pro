BRANCH = $$basename(PWD)

TEMPLATE = subdirs

SUBDIRS += ../abpal/projects/qt/abpal.pro
SUBDIRS += ../abservices/projects/qt/abservices.pro
SUBDIRS += ../common/projects/qt/common.pro
SUBDIRS += ../coreservices/projects/qt/coreservices.pro
SUBDIRS += ../map/projects/qt/map.pro
SUBDIRS += ../nav/projects/qt/nav.pro
SUBDIRS += ../navui/projects/qt/navui.pro
#SUBDIRS += ../../nbgm/$$BRANCH/bbuitest/nbgmbbuitest.pro
SUBDIRS += ../nbgm/projects/qt/nbgm.pro
SUBDIRS += ../nbpal/projects/qt/nbpal.pro
SUBDIRS += ../nbservices/projects/qt/nbservices.pro
#SUBDIRS += ../../nbservices/$$BRANCH/projects/qt_qnx_test/unittests.pro
SUBDIRS += ../nbui//projects/qt/nbui.pro
SUBDIRS += ../search/projects/qt/search.pro
#SUBDIRS += ../../systemtests/$$BRANCH/backup/projects/qt_qnx/systemtests.pro
#SUBDIRS += ../../thirdparty/$$BRANCH/projects/qt/ArgTable/argtable.pro
#SUBDIRS += ../../thirdparty/$$BRANCH/projects/qt/CUnit/CUnit.pro
#SUBDIRS += ../../thirdparty/$$BRANCH/projects/qt/glew/glew.pro
SUBDIRS += ../thirdparty/projects/qt/libpng/png.pro
SUBDIRS += ../thirdparty/projects/qt/libxml2/libxml2.pro
SUBDIRS += ../thirdparty/projects/qt/tinyxml/tinyxml.pro
#SUBDIRS += ../../thirdparty/$$BRANCH/projects/qt/zlib/zlib.pro

unix {
#    SUBDIRS += ../../thirdparty/$$BRANCH/projects/qt/flite/flite.pro
    SUBDIRS += ../thirdparty/projects/qt/sqlite/sqlite.pro
}
