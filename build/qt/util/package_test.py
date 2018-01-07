import os
import sys
import shutil
import platform

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def copy_executable(items , targetPath, buildType):
    """"""
    if not os.path.isdir(targetPath):
        os.mkdir(targetPath)

    for item in items:
        sourceBinPath = os.path.join( build_common.rootPath, "testapp", build_common.branchName, item, "project", buildType )
        destPath = os.path.join(targetPath, item)
        if not os.path.isdir(destPath):
            os.mkdir(destPath)

        for file in os.listdir(sourceBinPath):
            file = os.path.join(sourceBinPath, file)
            if os.path.isfile(file):
                if file.find( ".cpp" ) < 0 and file.find( ".o" ) < 0:
                    shutil.copy(file, destPath)
    return True

#----------------------------------------------------------------------
def copy_resources(items, targetPath):
    """"""
    testCodeRoot = os.path.join(build_common.rootPath, "testapp", build_common.branchName)

    for sampleName in items:
        resourcePath = os.path.join(testCodeRoot, sampleName, "resource")

        if not os.path.isdir(os.path.join(targetPath, sampleName)):
            os.mkdir(os.path.join(targetPath, sampleName))
        if not os.path.isdir(os.path.join(targetPath, sampleName, "resource")):
            os.mkdir(os.path.join(targetPath, sampleName, "resource"))

        build_common.copyPath(resourcePath, os.path.join(targetPath, sampleName, "resource"))

        if sampleName == "mapkit3d_sample":
            resourcePath = os.path.join(build_common.rootPath, "mapkit3d", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, sampleName, "resource"))
        if sampleName == "navkit_sample":
            resourcePath = os.path.join(build_common.rootPath, "navigationkit", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, sampleName, ""))
        if sampleName == "navigationuikit_sample":
            resourcePath = os.path.join(build_common.rootPath, "navigationuikit", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, sampleName, "resource"))
            resourcePath = os.path.join(build_common.rootPath, "navigationkit", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, sampleName, "resource"))

#----------------------------------------------------------------------
def copy_dependence(items, targetPath, buildType):
    """"""
    for itemName in items:

        binaryPath = os.path.join(targetPath, itemName)

        # if windows, copy dll files
        if platform.system() == "Windows" or platform.system() == "Microsoft":
            dllPath = os.path.join(build_common.rootPath, "build", build_common.branchName, "bin")
            build_common.copyPath(dllPath, binaryPath, ["debug","release"])
            build_common.copyPath(os.path.join(dllPath,buildType), binaryPath)

            dllPath = os.path.join(build_common.rootPath, "build", build_common.branchName, "CoreSDK", "lib", "qt", buildType, "nbpal.dll")
            shutil.copy(dllPath, binaryPath)

        # if linux, copy start up script
        if platform.system() == "Linux":
            if os.environ.get("BOARD") == "jetson":
                scriptPath = os.path.join(build_common.rootPath,"sampleapp", build_common.branchName, itemName, "startscript", "startup_jetson.sh")
            else:
                scriptPath = os.path.join(build_common.rootPath,"sampleapp", build_common.branchName, itemName, "startscript", "startup.sh")
            if os.path.isfile(scriptPath):
                shutil.copy(scriptPath, binaryPath)

#----------------------------------------------------------------------
def copy_items(items, targetPath, buildType):
    """"""
    copy_executable(items, targetPath, buildType)
    copy_resources(items, targetPath)
    copy_dependence(items, targetPath, buildType)
    return True

#----------------------------------------------------------------------
def package(targetPath, buildType = 'release'):
    """"""
    return copy_items(build_common.sampleItems, targetPath, buildType)

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_140715_ltk_v1_5" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    build_common.sampleItems = ["mapkit3d_sample"]

    package("../testapp")
