import os
import sys
import shutil
import platform

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def copy_items(items, targetPath, buildType):
    """"""
    sourceRootPath = os.path.join(build_common.rootPath, "thirdparty", build_common.branchName, "bin")

    Items = ["am", "automotivedemo", "Qt5.8.0", "neptuneui", "qtapplicationmanager"]

    for Item in Items: 
        destPath = os.path.join(targetPath, "bin", "thirdparty", Item)
        if not os.path.isdir(destPath):
            os.makedirs(destPath)
        if Item == "qtapplicationmanager":
            appmanPath = os.path.join(build_common.rootPath, "thirdparty", build_common.branchName, Item)
            os.chdir(appmanPath)
            os.system("qmake")
            os.system("make")
            appmanBinPath = os.path.join(appmanPath,"bin")
            ItemDestBinPath = os.path.join(destPath, "bin")
            if not os.path.isdir(ItemDestBinPath):
                os.makedirs(ItemDestBinPath)
            build_common.copyPath(appmanBinPath,ItemDestBinPath)
        else:
            build_common.copyPath(os.path.join(sourceRootPath, Item), destPath)

    locationStudioPath = os.path.join(targetPath, "bin", "thirdparty", "locationstudio")
    if not os.path.isdir(locationStudioPath):
	os.makedirs(locationStudioPath)
  
    ncdbPath = os.path.join(locationStudioPath, "libs", "ncdb")
    if not os.path.isdir(ncdbPath):
	os.makedirs(ncdbPath)

    pngPath = os.path.join(locationStudioPath, "libs", "png")
    if not os.path.isdir(pngPath):
	os.makedirs(pngPath)

    # copy ncdb

    # if linux on hybrid, copy libncdb.so
    if len(sys.argv) > 2 and sys.argv[2].lower() == "hybrid" and platform.system() == "Linux":
    	ncdbLibPath = os.path.join(build_common.rootPath, "build", build_common.branchName, "CoreSDK", "lib", "qt", buildType, "libncdb.so")
        if os.path.isfile(ncdbLibPath):
            print "ncdb source path "+ ncdbLibPath
            print "ncdb des path "+ ncdbPath
            shutil.copyfile(ncdbLibPath, os.path.join(ncdbPath, "libncdb.so"))
    
    # copy png

    pngSrcPath = os.path.join("/lib", "x86_64-linux-gnu", "libpng12.so.0.54.0")
    if os.path.isfile(pngSrcPath):
        shutil.copyfile(pngSrcPath, os.path.join(pngPath, "libpng12.so.0.54.0"))

    os.symlink("libpng12.so.0.54.0", os.path.join(pngPath, "libpng12.so.0"))

    resConsolePath = os.path.join(locationStudioPath, "res", "console", "resource")
    if not os.path.isdir(resConsolePath):
	os.makedirs(resConsolePath)

    resClusterPath = os.path.join(locationStudioPath, "res", "cluster", "resource")
    if not os.path.isdir(resClusterPath):
	os.makedirs(resClusterPath)

    resMapviewPluginPath = os.path.join(locationStudioPath, "res", "mapviewplugin", "resource")
    if not os.path.isdir(resMapviewPluginPath):
   	 os.makedirs(resMapviewPluginPath)

    # copy console and cluster resources
 
    build_common.copyPath(os.path.join(targetPath, "bin", "qtnavigator_console_plugin", "resource"), resConsolePath)
    build_common.copyPath(os.path.join(targetPath, "bin", "qtnavigator_cluster_plugin", "resource"), resClusterPath)
    build_common.copyPath(os.path.join(targetPath, "bin", "mapviewplugin", "resource"), resMapviewPluginPath)

    #copy install script
    shutil.copyfile(os.path.join(build_common.rootPath, "thirdparty", build_common.branchName, "automotivedemo", "install_automotive.sh"), os.path.join(targetPath, "install_automotive.sh"))
    
    return True

#----------------------------------------------------------------------
def package(targetPath, buildType = 'release'):
    """"""
    return copy_items(build_common.thirdpartyItems, targetPath, buildType)

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_141125_ltk_v1_6" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    #build_common.sampleItems = ["mapkit3d_sample", "navkit_sample", "navigationuikit_sample"]
    build_common.sampleItems = ["navigationuikit_sample"]

    package("../LTK")
