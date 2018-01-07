import os
import sys
import shutil

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def package(targetPath):
    """"""
    if os.path.isdir( os.path.join(targetPath,"assets") ):
        build_common.deletePath(os.path.join(targetPath,"assets"))
    os.mkdir(os.path.join(targetPath,"assets"))

    resourcePath = os.path.join(build_common.rootPath, "mapkit3d", build_common.branchName, "resource")
    build_common.copyPath(resourcePath, os.path.join(targetPath, "assets") )

    resourcePath = os.path.join(build_common.rootPath, "navigationkit", build_common.branchName, "resource")
    build_common.copyPath(resourcePath, os.path.join(targetPath, "assets"))

    return True

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_141125_ltk_v1_6" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit"]
    build_common.sampleItems = ["mapkit3d_sample", "navkit_sample"]

    package("../LTK")
