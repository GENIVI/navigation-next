import os
import sys
import stat

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def prepare(items, rootPath):

    # copy source code to root path
    if os.path.isdir(os.path.join(rootPath,"samples")):
        build_common.deletePath(os.path.join(rootPath,"samples"))
    os.mkdir(os.path.join(rootPath, "samples"))

    sampleCodeRoot = os.path.join(build_common.rootPath, "sampleapp", build_common.branchName)
    build_common.copyPath(os.path.join(sampleCodeRoot, "util"), os.path.join(rootPath, "samples", "util"))
    for item in items:
        os.mkdir(os.path.join(rootPath, "samples", item))
        build_common.copyPath(os.path.join(sampleCodeRoot, item), os.path.join(rootPath, "samples", item), ["startscript"])

    # copy library folder to ../../sampleapp
    tempLibraryFolder = os.path.join(build_common.curPath, "../../sampleapp/library")
    if os.path.isdir(tempLibraryFolder):
        build_common.deletePath(tempLibraryFolder)
    os.mkdir(tempLibraryFolder)

    if not os.path.isdir(os.path.join(rootPath, "library")):
        return False

    build_common.copyPath(os.path.join(rootPath, "library"), tempLibraryFolder)

    # get current server token from server_config.txt
    serverConfsFile = open(os.path.join(build_common.curPath, "server_configurations.txt"), "r")
    configs = serverConfsFile.readlines()
    serverConfsFile.close()

    token = ""
    if len(configs) > 0:
        token = configs[0].split(",")[1]
        token = token.strip().strip("\"")

    # generate servertoken.h
    tokenHeaderPath = os.path.join(sampleCodeRoot, "util", "include", "servertoken.h")
    if os.path.isfile(tokenHeaderPath):
        os.chmod(tokenHeaderPath, stat.S_IWRITE|stat.S_IWGRP|stat.S_IWOTH|stat.S_IREAD|stat.S_IRGRP|stat.S_IROTH)
        fileObj = open(tokenHeaderPath, 'r')
        content = fileObj.read()
        fileObj.close()
        content = content.replace("$YOUR_TOKEN", token)
        fileObj = open(tokenHeaderPath,'w')
        fileObj.write(content)
        fileObj.close()

    return True

#----------------------------------------------------------------------
def build_items(items, buildType):
    """"""
    sampleCodeRoot = os.path.join(build_common.rootPath, "sampleapp", build_common.branchName)

    for item in items:
        projectPath = os.path.join(sampleCodeRoot, item, "project")
        outputPath = os.path.join(projectPath, buildType)
        if os.path.isdir(os.path.join(outputPath)):
            build_common.deletePath(os.path.join(outputPath))

        if build_common.compileQtProject(projectPath, buildType) == False:
            print("!!!!!!!!!!Build LTK [" + item + "] failed.")
            return False;

    return True

#----------------------------------------------------------------------
def build(rootPath, buildType = 'release'):
    """"""
    if not prepare(build_common.sampleItems, rootPath):
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
