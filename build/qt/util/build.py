import os
import sys
import shutil
import platform

sys.path.append("./util")
import build_common
import build_library
import package_library
import build_sample
import build_thirdparty
import build_test
import package_sample
import package_thirdparty
import package_assets
import package_release
import package_test

versionLtk = ''
versionFolder = ''
versionPath = ''

#----------------------------------------------------------------------
def setup_environment():
    """"""
    global versionPath
    global versionFolder
    global versionLtk

    build_common.platform = "qnx"
    if len(sys.argv) > 1:
    	build_common.platform = sys.argv[1]
    build_common.curPath = os.path.abspath(".")
    build_common.branchName = os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")

    if build_common.platform == "linux":
        build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "searchkit"]
        build_common.sampleItems = ["qtnavigator_console_plugin", "qtnavigator_cluster_plugin", "qtnavigator_sampleApp","mapviewplugin"]
        build_common.thirdpartyItems = ["automotivedemo"]
    elif build_common.platform == "qnx":
        build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "searchkit"]
        build_common.sampleItems = ["mapkit3d_sample", "navkit_sample"]
        build_common.thirdpartyItems = []
    else :
        build_common.ltkItems = []
        build_common.sampleItems = []
        build_common.thirdpartyItems = []
    # read toolchain and architecture
    toolchain = "ubuntu"
    architecture = "x86_64"
    if len(sys.argv) > 3:
	toolchain = sys.argv[3].lower()
    if len(sys.argv) > 4:
	architecture = sys.argv[4].lower()

    print ("here is .........."+toolchain+"_"+architecture)
    # read version.txt, create package folder
    versionLtk = open( "version.txt", 'r').read()
    versionFolder = "ltk_" + versionLtk + "_" + toolchain + "_" + architecture
    if len(sys.argv) > 2 and sys.argv[2].lower() == "hybrid":
        versionFolder = versionFolder + "_hybrid"
    versionPath = os.path.join(build_common.curPath, versionFolder)

    if os.path.isdir(versionPath):
        build_common.deletePath(versionPath)
    os.mkdir(versionPath)

    return True

def build_navkit_resource():
    navkitBuildPath = os.path.join(build_common.rootPath, "navigationkit", build_common.branchName, "Supportfiles","build")
    sys.path.append(navkitBuildPath)
    import build_navkit_package

    build_navkit_package.GenResource(navkitBuildPath, os.path.join(navkitBuildPath,"..","..","resource"))


if __name__=='__main__':

    if not setup_environment():
        print "setup environment failed"
        print "BUILD FAILED"
        exit()

    build_navkit_resource()

    if not build_library.build(versionLtk):
        print "build ltk libraries failed"
        print "BUILD FAILED"
        exit()

    os.mkdir(os.path.join(versionPath, "library"))
    if not package_library.package(os.path.join(versionPath, "library")):
        print "package libraries failed"
        print "BUILD FAILED"
        exit()

    if not build_sample.build(versionPath):
        print "build ltk samples failed"
        print "BUILD FAILED"
        exit()

    if not package_sample.package(versionPath):
        print "package samples failed"
        print "BUILD FAILED"
        exit()

    #if not build_thirdparty.build(versionPath):
    #    print "build Thirdparty failed"
    #    print "BUILD FAILED"
    #    exit()

    if not package_thirdparty.package(versionPath):
        print "package thirdparty failed"
        print "BUILD FAILED"
        exit()

    #if not build_test.build():
    #    print "build test app failed"
    #    print "BUILD FAILED"
    #    exit()

    #if not package_test.package(os.path.join(build_common.curPath, "testapp")):
    #    print "package test app failed"
    #    print "BUILD FAILED"
    #    exit()

    #if not package_assets.package(versionPath):
    #    print "package assets failed"
    #    print "BUILD FAILED"
    #    exit()

    if not package_release.package(os.path.join(build_common.curPath, versionLtk), versionPath):
        print "build final release package failed"
        print "BUILD FAILED"
        exit()

    print "BUILD SUCCESSFUL"
