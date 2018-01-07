import os
import sys
import shutil
import zipfile

sys.path.append("./util")
import build_common
import build_LTK
import build_copy_ccc
import build_samples
import build_sample_source_code
import build_resource
import build_assets
import build_runable_necessary

if __name__=='__main__':

    build_common.curPath = os.path.abspath("./")
    build_common.branchName = os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")

    # read version.txt, create package folder
    versionLtk = open( "version.txt", 'r').read()
    versionFolder = "ltk_" + versionLtk
    versionPath = os.path.join(build_common.curPath, versionFolder)

    if os.path.isdir(versionPath):
        build_common.deletePath(versionPath)
    os.mkdir(versionPath)

    # buld ltk sdk
    print "package--creating LTK SDK"
    ltkItems = ["common", "locationkit", "mapkit3d"]
    targetPath = os.path.join(versionPath, "library")
    if os.path.isdir(targetPath):
        build_common.deletePath(targetPath)
    os.mkdir(targetPath)

    if not build_LTK.compileLTKs(ltkItems, targetPath):
        print "build ltk libraries failed"
        print "BUILD FAILED"
        exit()

    build_LTK.copyHeaders(ltkItems, targetPath)
    build_LTK.copyTemplatePro(targetPath)
    build_copy_ccc.copyCCCLibs(targetPath)

    # copy the library to the specified locatoin
    print "package--copying LTK SDK to sampleapp/library"
    tempLibraryFolder = os.path.join(build_common.curPath, "../../sampleapp/library")
    if os.path.isdir(tempLibraryFolder):
        build_common.deletePath(tempLibraryFolder)
    os.mkdir(tempLibraryFolder)
    build_common.copyPath(targetPath, tempLibraryFolder)


    sampleItems = ["mapkit3d_sample"]
    # copy source code to versionPath
    build_sample_source_code.copySourceCode(sampleItems, versionPath)

    # build sample app
    print "package--compiling sample app"
    targetPath = os.path.join(versionPath, "bin")
    if os.path.isdir(targetPath):
        build_common.deletePath(targetPath)
    os.mkdir(targetPath)

    for item in sampleItems:
        if not build_samples.buildSample(item, os.path.join(targetPath, item)):
            print "build sample apps failed"
            print "BUILD FAILED"
            exit()

    # copy resource files
    print "package--copying resource files"
    for item in sampleItems:
        build_resource.copyResource(item, os.path.join(targetPath, item, "resource"))

    # copy other necessary files
    print "package--copying other necessary files"
    for item in sampleItems:
        build_runable_necessary.copyRunableNecessary(item, os.path.join(targetPath, item))

    # copy assets
    print "package--copying assets"
    targetPath = os.path.join(versionPath, "assets")
    if os.path.isdir(targetPath):
        build_common.deletePath(targetPath)
    os.mkdir(targetPath)

    build_assets.copyAssets("mapkit3d", os.path.join(targetPath, "mapkit3d_sample"))

    # copy doc
    print "package--copying documents"
    outputPath = os.path.join(build_common.curPath,"output")

    os.chdir(build_common.curPath)
    os.system("doxygen doxygenConfiguration")
    shutil.move(os.path.join(outputPath,"docs"), versionPath)

    if os.path.isfile(os.path.join(build_common.rootPath,"doc",build_common.branchName,"LTK SDK GettingStarted_Guide.doc")):
        shutil.copy(os.path.join(build_common.rootPath,"doc",build_common.branchName,"LTK SDK GettingStarted_Guide.doc"), versionPath)

    # zip the final package
    zipFileName = versionFolder + ".zip"
    print "package--creating zip file: " + versionFolder + ".zip"
    shutil.make_archive(versionFolder, "zip", ".", versionFolder)

    # copy to the version folder
    if os.path.isdir(os.path.join(build_common.curPath,versionLtk)):
        build_common.deletePath(os.path.join(build_common.curPath,versionLtk))
    os.mkdir(versionLtk)
    shutil.move(zipFileName,versionLtk)
    shutil.move(versionFolder, versionLtk)
    if os.path.isdir(os.path.join(build_common.curPath, "tpslib")):
        shutil.move(os.path.join(build_common.curPath, "tpslib"), versionLtk)

    print "BUILD SUCCESSFUL"