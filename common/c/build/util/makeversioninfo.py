import os
import subprocess
import sys

def makeVersionInfo(component, branch, platform):
    savedir = os.getcwd()
    path = "../../" + component + "/" + branch
    print "version cd to %s" % path
    os.chdir(path)

    cmdStr = "build\\version.bat " + os.getcwd() + " " + platform
    print "creating VERSIONINFO.H for " + component
    rc = subprocess.call(cmdStr)
    print "create versioninfo = %d" % rc

    os.chdir(savedir)
    if rc != 0:
        sys.exit()

def makeVersionInfoNbgm(branch, platform):
    savedir = os.getcwd()
    nbgmmainPath = "../../nbgm/" + branch +"/nbgmmain"
    nbrePath = "../../nbgm/" + branch +"/nbre"

    print "version cd to %s" % nbgmmainPath
    os.chdir(nbgmmainPath)
    cmdStr = "build\\version.bat " + os.getcwd() + " " + platform
    print "creating VERSIONINFO.H for nbgmmain"
    rc = subprocess.call(cmdStr)
    print "create versioninfo = %d" % rc
    os.chdir(savedir)

    #print "version cd to %s" % nbrePath
    #os.chdir(nbrePath)
    #cmdStr = "build\\version.bat " + os.getcwd() + " " + platform
    #print "creating VERSIONINFO.H for nbre"
    #rc = subprocess.call(cmdStr)
    #print "create versioninfo = %d" % rc
    #os.chdir(savedir)

    if rc != 0:
        sys.exit()

PLATFORM = "android_ndk"

line = os.path.abspath('')
splitPath = line.split('\\')
BRANCH = splitPath[len(splitPath)-1]

if __name__ == '__main__':
    if len(sys.argv) > 1:
        COMPONENT = sys.argv[1]

    if  COMPONENT != "nbgm":
        makeVersionInfo(COMPONENT, BRANCH, PLATFORM)
    else:
        makeVersionInfoNbgm(BRANCH, PLATFORM)