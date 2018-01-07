import os
import sys
import shutil
import platform

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def copy_executable(items , targetPath, buildType):
    """"""
    if not os.path.isdir(os.path.join(targetPath, "bin")):
        os.mkdir(os.path.join(targetPath, "bin"))

    for item in items:
        sourceBinPath = os.path.join( build_common.rootPath, "sampleapp", build_common.branchName, item, "project", buildType )
        destPath = os.path.join(targetPath, "bin", item)
        if not os.path.isdir(destPath):
            os.mkdir(destPath)

        for file in os.listdir(sourceBinPath):
            file = os.path.join(sourceBinPath, file)
            if os.path.isfile(file):
                if file.find( ".cpp" ) < 0 and file.find( ".o" ) < 0 and file.find( ".h" ) < 0:
                    shutil.copy(file, destPath)
                    #return True

    return True

#----------------------------------------------------------------------
def copy_resources(items, targetPath):
    """"""
    sampleCodeRoot = os.path.join(build_common.rootPath, "sampleapp", build_common.branchName)

    for sampleName in items:
        resourcePath = os.path.join(sampleCodeRoot, sampleName, "resource")

        if not os.path.isdir(os.path.join(targetPath, "bin", sampleName)):
            os.mkdir(os.path.join(targetPath, "bin", sampleName))
        if not os.path.isdir(os.path.join(targetPath, "bin", sampleName, "resource")):
            os.mkdir(os.path.join(targetPath, "bin", sampleName, "resource"))

        build_common.copyPath(resourcePath, os.path.join(targetPath, "bin", sampleName, "resource"))

        if sampleName == "mapkit3d_sample":
            resourcePath = os.path.join(build_common.rootPath, "mapkit3d", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, "bin", sampleName, "resource"))
        if sampleName == "navkit_sample":
            resourcePath = os.path.join(build_common.rootPath, "navigationkit", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, "bin", sampleName, ""))
        if sampleName == "navigationuikit_sample":
            resourcePath = os.path.join(build_common.rootPath, "navigationuikit", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, "bin", sampleName, "resource"))
            resourcePath = os.path.join(build_common.rootPath, "navigationkit", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, "bin", sampleName, "resource"))
            resourcePath = os.path.join(build_common.rootPath, "mapkit3d", build_common.branchName, "resource")
            build_common.copyPath(resourcePath, os.path.join(targetPath, "bin", sampleName, "resource"))

#----------------------------------------------------------------------
def copy_dependence(items, targetPath, buildType):
    """"""
    for itemName in items:

        binaryPath = os.path.join(targetPath, "bin", itemName)

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
                shutil.copyfile(scriptPath, os.path.join(binaryPath, "startup.sh"))

	# if linux on hybrid, copy libncdb.so
        if len(sys.argv) > 1 and sys.argv[1].lower() == "hybrid" and platform.system() == "Linux":
            ncdbLibPath = os.path.join(build_common.rootPath, "build", build_common.branchName, "CoreSDK", "lib", "qt", buildType, "libncdb.so")
            if os.path.isfile(ncdbLibPath):
                shutil.copyfile(ncdbLibPath, os.path.join(binaryPath, "libncdb.so"))

#---------------need to remove later--------------------------------
def build_copy_wayland_sample(targetPath):
    """"""
    itemName = "waylandCompositorSample"
    waylandSamplePath = os.path.join(build_common.rootPath,"sampleapp", build_common.branchName, itemName)
    os.chdir(os.path.join(waylandSamplePath, "demo"))
    os.system("qmake")
    os.system("make clean")
    os.system("make")
    binaryPath = os.path.join(targetPath, "bin", itemName)
    os.mkdir(binaryPath)
    build_common.copyPath(waylandSamplePath,binaryPath)
    samplePath = os.path.join(targetPath, "samples", itemName)
    os.mkdir(samplePath)
    build_common.copyPath(waylandSamplePath,samplePath)
#----------------------------------------------------------------------
def copy_items(items, targetPath, buildType):
    """"""
    copy_executable(items, targetPath, buildType)
    copy_resources(items, targetPath)
    copy_dependence(items, targetPath, buildType)
#--------need to remove later-------------------------------------------
    build_copy_wayland_sample(targetPath)
    return True

#----------------------------------------------------------------------
def package(targetPath, buildType = 'release'):
    """"""
    return copy_items(build_common.sampleItems, targetPath, buildType)

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_141125_ltk_v1_6" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    #build_common.sampleItems = ["mapkit3d_sample", "navkit_sample", "navigationuikit_sample"]
    build_common.sampleItems = ["navigationuikit_sample"]

    package("../LTK")
