import os
import sys
import shutil

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def copy_headers(items, targetPath):
    """"""
    if os.path.isdir(os.path.join(targetPath,"include")):
        build_common.deletePath(os.path.join(targetPath,"include"))
    os.mkdir(os.path.join(targetPath, "include"))

    for item in items:
        os.mkdir( os.path.join(targetPath, "include", item) )
        itemInclude = os.path.join(build_common.rootPath, item, build_common.branchName, "include")
        build_common.copyPath(itemInclude, os.path.join(targetPath, "include", item), ["private", "protected"])

    return True

#----------------------------------------------------------------------
def copy_libraries(items, targetPath, buildType):
    """"""
    if os.path.isdir(os.path.join(targetPath,"lib")):
        build_common.deletePath(os.path.join(targetPath,"lib"))

    os.mkdir(os.path.join(targetPath, "lib"))
    build_common.copyPath(os.path.join(build_common.outputRoot, buildType, "lib"), os.path.join(targetPath, "lib"))

    return True

#----------------------------------------------------------------------
def copy_ccc_libaries(targetPath, buildType):
    """"""
    libPath = os.path.join(build_common.curPath,"CoreSDK", "lib", "qt", buildType)
    if not os.path.isdir(os.path.join(targetPath,"lib")):
        os.mkdir(os.path.join(targetPath,"lib"))
    build_common.copyPath(libPath, os.path.join(targetPath, "lib"))

#----------------------------------------------------------------------
def change_and_copy_project(targetPath):
    """"""
    projectPath = os.path.join(build_common.curPath, "locationtoolkit.pro")
    if os.path.isfile(projectPath):
	fileObj = open(projectPath, 'r')
        lines = fileObj.readlines()
        fileObj.close()
        if len(sys.argv) > 1 and sys.argv[1].lower() == "hybrid":
            lines[2] = lines[2].strip('\n') + ' -lncdb\n'
        fileObj = open(os.path.join(targetPath, "locationtoolkit.pro"), 'w')
        fileObj.writelines(lines)
        fileObj.close

#----------------------------------------------------------------------
def package(targetPath, buildType = 'release'):
    """"""
    copy_headers(build_common.ltkItems, targetPath)
    copy_libraries(build_common.ltkItems, targetPath, buildType)
    copy_ccc_libaries(targetPath, buildType)
    change_and_copy_project(targetPath)
    return True

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_140715_ltk_v1_5" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    build_common.samleItems = ["mapkit3d_sample", "navkit_sample", "navigationuikit_sample"]

    package("../LTK")
