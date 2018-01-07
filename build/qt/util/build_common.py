import os
import shutil
import stat

global curPath
global branchName
global rootPath
global outputRoot
global ltkItems
global sampleItems
global thirdpartyItems
global platform

#----------------------------------------------------------------------
def copyPath(srcPath, destPath,avoid=[]):
    """copy all the files in srcPath to destPath"""

    if not os.path.exists(srcPath):
        return

    if not os.path.isdir(destPath):
        os.mkdir(destPath)
        
    for source in os.listdir(srcPath):
        source = os.path.join(srcPath, source )
        try:
            if os.path.isfile(source):
                    shutil.copy(source, destPath)
            elif os.path.isdir(source):
                dirName = os.path.split(source)[1]
                if avoid.count(dirName) == 0:
                    des = os.path.join(destPath, dirName)
                    #shutil.copytree(source, des)
                    if not os.path.isdir(des):
                        os.mkdir(des)
                    copyPath(source, des, avoid)
        except:
            print 'copy %s failed' % source

#----------------------------------------------------------------------
def compileQtProject(pathName, buildType):
    """compile certain Qt project in specified path"""
    os.chdir(pathName)
    os.system("qmake")
    if "DOCLEAN" in os.environ:
    	if os.environ["DOCLEAN"] == "yes":
        	os.system("make clean")
    if buildType == "debug":
        if os.system("make -j 4 debug") != 0:
            return False
    elif buildType == "release":
        if os.system("make -j 4 release") != 0:
            return False
    else:
        if os.system("make -j 4 all") != 0:
            return False
    return True

#----------------------------------------------------------------------

def deletePath(path):
    """delete the sepcified path, it can be a folder or a file"""
    if os.path.exists(path):
        if os.path.isdir(path):  # if dir, remove the read only properties of all files
            for fileList in os.walk(path):
                curSubPath = fileList[0]
                curFiles = fileList[2]
                for item in curFiles:
                    fullPath = os.path.join(curSubPath, item)
                    if os.path.isfile(fullPath):
                        os.chmod( os.path.join(curSubPath, item), stat.S_IWRITE )
            shutil.rmtree(path, True)
        elif os.path.isfile(path):
            os.chmod(path, stat.S_IWRITE)
            os.remove(path)
