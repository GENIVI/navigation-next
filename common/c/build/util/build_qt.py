import os
import os.path
import sys
import stat
import subprocess
import shutil

from codegenerator import *

class BuildConfig:
    def __init__(self):
        self.target = "linux_armhf"
        self.buildType = "debug"
        self.CCC_MODE = "onboard"
        self.version_ccc = ""
        self.sdkpath = ""
        self.outputPath = ""
        self.platform = "linux"
        self.buildItems = ["nbpal", "abpal", "abservices", "coreservices", "nbservices", "nbgm", "nbui", "map", "common", "nav", "navui", "search", "navui"]
        self.thirdpartyItems = []
    def check(self):
        return True

# get the branch name from current path
curPath = os.path.dirname(os.path.dirname((os.path.realpath(__file__))))
rootPath = os.path.dirname(curPath)
#branchName = os.path.split(curPath)[1]
buildConfig = BuildConfig()

def removeDir(rootdir):
    if os.path.exists(rootdir):
        filelist = os.listdir(rootdir)
        for f in filelist:
            filepath = os.path.join( rootdir, f )
            if os.path.isfile(filepath):
                os.chmod(filepath, stat.S_IWRITE)
                os.remove(filepath)
            elif os.path.isdir(filepath):
                removeDir(filepath)
        os.rmdir(rootdir)

def copyDir(srcPath, destPath, ext, recursive=True):
    if not os.path.exists(srcPath):
        return

    if not os.path.exists(destPath):
        os.makedirs(destPath)
    for source in os.listdir(srcPath):
        s = os.path.join(srcPath, source)
        d = os.path.join(destPath, source)
        if os.path.isfile(s):
            if os.path.exists(d):
                os.chmod(d, stat.S_IWRITE)
            if ext == "":
                shutil.copy(s, d)
            else:
                if os.path.splitext(source)[1] == ext:
                    shutil.copy(s, d)
        elif os.path.isdir(s) and recursive:
            copyDir(s, d, ext)

def copyHeaders(component):
    if component == "nbgm":
        copyDir(os.path.join(rootPath, component, "nbgmmain/include"), os.path.join(buildConfig.sdkpath, "include", "nbgm"), "")
    else:
        copyDir(os.path.join(rootPath, component, "include"), os.path.join(buildConfig.sdkpath, "include", component), "")

def ConfigureComponent(component):
    if component == "nbgm":
        verbuildPath = os.path.join(rootPath, component, "build")
        os.chdir(verbuildPath)
        os.system("python version.py ../nbgmmain qt")
    elif component == "abpal" \
       or component == "abservices" \
       or component == "coreservices" \
       or component == "nbservices" \
       or component == "nbgm" \
       or component == "common":
        verbuildPath = os.path.join(rootPath, component, "build")
        os.chdir(verbuildPath)
        os.system("python version.py ../ qt")

def buildComponent(component):
    print "===begine to buld [" + component + "]"
    ConfigureComponent(component)
    proPath = os.path.join(rootPath, component, "projects", "qt")
    os.chdir(proPath)
    os.system("qmake")
    if "DOCLEAN" in os.environ:
        if os.environ["DOCLEAN"] == "yes":
            os.system("make clean")
    if buildConfig.buildType == "debug":
        if os.system("make -j 4 debug") != 0:
            return False
    elif buildConfig.buildType == "release":
        if os.system("make -j 4 release") != 0:
            return False
    else:
        if os.system("make -j 4 all"):
            return False
    return True

def buildThirdParty(component):
    if buildConfig.platform == "linux" and component == "flite":
        flitePath = os.path.join(rootPath, "thirdparty", "flite-1.4-release")
        os.chdir(flitePath)
        os.system("chmod +x configure")
        board = os.environ.get("BOARD")
        if board == "jetson":
            machineType = "arm-linux-gnueabihf"
            os.system("./configure --host=arm-linux-gnueabihf")
        else:
            import platform
            (arch, _) = platform.architecture()
            if arch == "64bit":
                machineType = "x86_64-linux-gnu"
            else:
                machineType = "i386-linux-gnu"
            os.system("./configure")
        os.system("make all")

        flitebuildPath = os.path.join(flitePath, "build", machineType, "lib")
        libs = os.listdir(flitebuildPath)
        for lib in libs:
            print os.path.join(flitebuildPath, lib)
            print os.path.join(buildConfig.outputPath, "lib", "qt", "release", lib)
            shutil.copyfile(os.path.join(flitebuildPath, lib), os.path.join(buildConfig.outputPath, "lib", "qt", "release", lib))
            shutil.copyfile(os.path.join(flitebuildPath, lib), os.path.join(buildConfig.outputPath, "lib", "qt", "debug", lib))
    else:
        proPath = os.path.join(rootPath, "thirdparty", "projects", "qt", component)
        os.chdir(proPath)
        os.system("qmake")      
        if "DOCLEAN" in os.environ:
            if os.environ["DOCLEAN"] == "yes":
                os.system("make clean")
        if buildConfig.buildType == "debug":
            if os.system("make -j 4 debug") != 0:
                return False
        elif buildConfig.buildType == "release":
            if os.system("make -j 4 release") != 0:
                return False
        else:
            if os.system("make -j 4 all") != 0:
                return False
    return True

def buildComponents():
    for item in buildConfig.buildItems:
        if not buildComponent(item):
            print("!!!!!!!!!!Build Component [" + item + "] failed.")
            return False

    for item in buildConfig.thirdpartyItems:
        if not buildThirdParty(item):
            print("!!!!!!!!!!Build thirdparty [" + item + "] failed.")
            return False
    return True


def makeCommonVersionInfo():
    savedir = os.getcwd()
    path = "../common/"
    print "version cd to %s" % path
    os.chdir(path)

    verSrc = os.path.join(savedir, "version_qt.txt")
    verDst = os.path.join(os.getcwd(), "resources/version.txt")
    print "verDst: " + verDst
    os.chmod(verDst, 0777)
    shutil.copyfile(verSrc, verDst)

    script = os.path.join(os.getcwd(), "build/version.py")
    params = ["python", script, os.getcwd(), "qnx"]
    print params
    print "creating VERSIONINFO.H for common"
    rc = subprocess.call(params)
    print "create versioninfo = %d" % rc
    os.chdir(savedir)


def Configure():
    buildConfig.version_ccc = open(os.path.join(curPath, 'version_qt.txt'), 'r').read()
    buildConfig.sdkpath = os.path.join(curPath, "CoreSDK_" + buildConfig.version_ccc + "_qt_" + buildConfig.target + "_internal")
    buildConfig.outputPath = os.path.join(curPath, "output")
    removeDir(buildConfig.sdkpath)
    makeCommonVersionInfo()
    GenerateCode()
    #-----------------------------------------------------------------------------------------------------------------------------------------#
    # Set CCC mode - hybrid or offboard
    #-----------------------------------------------------------------------------------------------------------------------------------------#
    use_onboard_file = open(os.path.join(rootPath, "nbservices","include", "private", "useonboard.h"),"w")
    if buildConfig.CCC_MODE == "onboard":
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n//#define OFFBOARD_SERVICE_ONLY\n#endif')
    else:
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n#define OFFBOARD_SERVICE_ONLY\n#endif')
    use_onboard_file.close()

    MergeTps("%s_%s_%s_internal"%("CoreSDK", buildConfig.version_ccc, "qt_" + buildConfig.target))

    #-----------------------------------------------------------------------------------------------------------------------------------------#
    # Setup thridparty components
    #-----------------------------------------------------------------------------------------------------------------------------------------#
    print buildConfig.platform
    if buildConfig.platform == "win32":
        buildConfig.thirdpartyItems = ["tinyxml", "libxml2", "flite", "zlib", "glew"]
    elif buildConfig.platform == "linux":
        buildConfig.thirdpartyItems = ["tinyxml", "libxml2", "flite", "libpng", "sqlite"]
    else:
        buildConfig.thirdpartyItems = ["tinyxml", "libxml2", "libpng"]

    return True

def Build():
    if buildComponents():
        print "==========Build complete=========="
        print "BUILD SUCCESSFUL"
        return True
    else:
        print "==========Build Failed=========="
        return False

def Package():
    print "==========Begin to make package=========="
    copyDir(buildConfig.outputPath, buildConfig.sdkpath, "")
    os.makedirs(os.path.join(buildConfig.sdkpath, "include"))
    for item in buildConfig.buildItems:
        copyHeaders(item)

    copyDir(os.path.join(rootPath, "thirdparty","flite-1.4-release", "include"), os.path.join(buildConfig.sdkpath, "include", "flite"), ".h")
    shutil.copyfile(os.path.join(rootPath, "thirdparty", "flite-1.4-release", "lang", "cmu_us_rms", "voxdefs.h"), os.path.join(buildConfig.sdkpath, "include", "flite", "voxdefs.h"))

    #shutil.copy(os.path.join(curPath, "nbm", "libs", "libNBM.a"), os.path.join(buildConfig.sdkpath, "lib", "qt", "release") )
    #shutil.copy(os.path.join(curPath, "nbm", "libs", "libNBM.a"), os.path.join(buildConfig.sdkpath, "lib", "qt", "debug") )

    if buildConfig.platform == "qnx":
        shutil.copy(os.path.join(curPath, "ncdb", "libs", "arm", "a-le-v7", "libncdb.so"), os.path.join(buildConfig.sdkpath, "lib", "qt", "release") )
        shutil.copy(os.path.join(curPath, "ncdb", "libs", "arm", "a-le-v7-g", "libncdb_g.so"), os.path.join(buildConfig.sdkpath, "lib", "qt", "debug") )
    else:
        shutil.copy(os.path.join(curPath, "ncdb", "libs", "libncdb.so"), os.path.join(buildConfig.sdkpath, "lib", "qt", "release") )
        shutil.copy(os.path.join(curPath, "ncdb", "libs", "libncdb.so"), os.path.join(buildConfig.sdkpath, "lib", "qt", "debug") )

    print "==========Package complete=========="

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print "==========Build Failed=========="
        print "The count of argument should be 4."
        exit()

    print sys.argv[1]
    buildConfig.CCC_MODE = sys.argv[1].lower()
    buildConfig.buildType = sys.argv[2].lower()
    buildConfig.target = sys.argv[3].lower()
    buildConfig.platform = sys.argv[4].lower()

    if not buildConfig.check():
        exit()
    if not Configure():
        exit()
    if not Build():
        exit()
    if not Package():
        exit()
