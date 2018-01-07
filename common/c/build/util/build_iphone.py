import os
import stat
import sys
import subprocess
import array
import shutil
import glob

from codegenerator import GenerateCode

GENERATER_MARKER = "THIS IS GENERATED CODE. DO NOT MODIFY"

if __name__ == '__main__':
    if len(sys.argv) > 2:
        SRC_ROOT = sys.argv[2]
        print('SRC_ROOT is specified')
        NBM_VERSION_FILE = SRC_ROOT + "/nbm_iphone_version.txt"
    else:
        SRC_ROOT = ""
        print('SRC_ROOT is not specified')
        NBM_VERSION_FILE = "nbm_iphone_version.txt"

    #Code generator
    GenerateCode()

    f = open(NBM_VERSION_FILE, "r")
    NBM_VER = f.read();
    NBM_FOLDER = "nbm_" + NBM_VER + "_iphone"
    HOME_DIR = os.path.expanduser('~')

    NBM_PATH = "\$\(PROJECT_DIR\)/../../../../../../../core/releases/nbm/iphone/" + NBM_FOLDER

    cmdStr = SRC_ROOT + "util/setNBMpath.sh " + HOME_DIR + " " + NBM_PATH
    retVal = subprocess.call(cmdStr,shell=True)
    sys.exit(retVal)
