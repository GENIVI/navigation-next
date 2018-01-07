import os
import sys
import stat

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def prepare(items):

    # get current server configs from server_config.txt
    serverConfsFile = open(os.path.join(build_common.curPath, "server_configurations.txt"), "r")
    configLines = serverConfsFile.readlines()
    serverConfsFile.close()

    configs = ""
    for line in configLines:
        items = line.split(",")
        configs = configs + items[0] + ", "
        configs = configs + items[1] + ",\n\t"

    # generate serverconfig.h
    testCodeRoot = os.path.join(build_common.rootPath, "testapp", build_common.branchName)
    configHeaderPath = os.path.join(testCodeRoot, "util", "include", "serverconfig.h")
    if os.path.isfile(configHeaderPath):
        os.chmod(configHeaderPath, stat.S_IWRITE|stat.S_IWGRP|stat.S_IWOTH|stat.S_IREAD|stat.S_IRGRP|stat.S_IROTH)
        fileObj = open(configHeaderPath, 'r')
        content = fileObj.read()
        fileObj.close()
        content = content.replace("$SERVER_CONFIG", configs)
        fileObj = open(configHeaderPath,'w')
        fileObj.write(content)
        fileObj.close()
    else:
        return False

    return True

#----------------------------------------------------------------------
def build_items(items, buildType):
    """"""
    testCodeRoot = os.path.join(build_common.rootPath, "testapp", build_common.branchName)

    for item in items:
        projectPath = os.path.join(testCodeRoot, item, "project")
        outputPath = os.path.join(projectPath, buildType)
        if os.path.isdir(os.path.join(outputPath)):
            build_common.deletePath(os.path.join(outputPath))

        if build_common.compileQtProject(projectPath, buildType) == False:
            print("!!!!!!!!!!Build LTK [" + item + "] failed.")
            return False;

    return True

#----------------------------------------------------------------------
def build(buildType = 'release'):
    """"""
    if not prepare(build_common.sampleItems):
        return False

    return build_items(build_common.sampleItems, buildType)

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_140715_ltk_v1_5" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    build_common.sampleItems = ["mapkit3d_sample"]

    build(build_common.rootPath)
