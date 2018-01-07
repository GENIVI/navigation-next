import os
import sys
import shutil
import stat

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def build_items(items, buildType):
    """"""
    for item in items:
        proPath = os.path.join(build_common.rootPath, "thirdparty", build_common.branchName, item)

    	os.chdir(proPath)
    	os.system("qmake")
    	os.system("make clean")
    
        if os.system("make") != 0:
            print("!!!!!!!!!!Build Thirdparty [" + item + "] failed.")
	    return False

        os.system("sudo make install")
        os.chdir(build_common.rootPath)

    return True

#----------------------------------------------------------------------
def build(version, buildType = 'release'):
    """"""
    return build_items(build_common.thirdpartyItems, buildType)

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_140715_ltk_v1_5" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    build_common.samleItems = ["mapkit3d_sample", "navkit_sample", "navigationuikit_sample"]

    build("1.0.0.172")
