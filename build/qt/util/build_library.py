import os
import sys
import shutil
import stat

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def prepare(version):
    """create version.h for ltkcommon"""
    versionHeaderPath = os.path.join(build_common.rootPath, "common", build_common.branchName, "include", "private", "version.h")
    if os.path.isfile(versionHeaderPath):
        os.chmod(versionHeaderPath, stat.S_IWRITE|stat.S_IWGRP|stat.S_IWOTH|stat.S_IREAD|stat.S_IRGRP|stat.S_IROTH)
        fileObj = open(versionHeaderPath, 'r')
        content = fileObj.read()
        fileObj.close()
        content = content.replace("$LTK_VERSION", version)
        fileObj = open(versionHeaderPath,'w')
        fileObj.write(content)
        fileObj.close()
    """create usehybrid.h for ltkcommon"""
    useHybridHeadPath = os.path.join(build_common.rootPath, "common", build_common.branchName, "include", "private", "usehybrid.h")
    if os.path.isfile(useHybridHeadPath):
        lines = ['#ifndef _USE_HYBRID_H_\n', '#define _USE_HYBRID_H_\n', '\n', '//#define ACTIVE_HYBRID_MODE\n', '\n', '#endif // _USE_HYBRID_H_']
        if len(sys.argv) > 1 and sys.argv[1].lower() == "hybrid":
            lines[3] = '#define ACTIVE_HYBRID_MODE\n'
        os.chmod(useHybridHeadPath, stat.S_IWRITE|stat.S_IWGRP|stat.S_IWOTH|stat.S_IREAD|stat.S_IRGRP|stat.S_IROTH)
        fileObj = open(useHybridHeadPath, 'w')
        fileObj.writelines(lines)
        fileObj.close

#----------------------------------------------------------------------
def build_items(items, buildType):
    """"""
    for item in items:
        proPath = os.path.join(build_common.rootPath, item, build_common.branchName, "project")
        if build_common.compileQtProject(proPath, buildType) == False:
            print("!!!!!!!!!!Build LTK [" + item + "] failed.")
            return False;
        os.chdir(build_common.rootPath)

    return True

#----------------------------------------------------------------------
def build(version, buildType = 'release'):
    """"""
    prepare(version)
    return build_items(build_common.ltkItems, buildType)

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_140715_ltk_v1_5" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    build_common.samleItems = ["mapkit3d_sample", "navkit_sample", "navigationuikit_sample"]

    build("1.0.0.172")
