import os
import stat
import sys
import subprocess
import array
import shutil
import glob

try:
    BUILD_MACHINE = sys.argv[1]
except IndexError:
    BUILD_MACHINE = 'build'

if len(sys.argv) > 2:
    SRC_ROOT = sys.argv[2]
    print('SRC_ROOT is specified')
    NBM_VERSION_FILE = SRC_ROOT + "/nbm_android_version.txt"
else:
    SRC_ROOT = ""
    print('SRC_ROOT is not specified')
    NBM_VERSION_FILE = "nbm_android_version.txt"

#Code generator
br = os.getcwd().split('\\')
br = br[len(br) - 1]
retVal = 0

#generator_list contain dates of modified code generator files
generator_list = []
for root,dirs,files in os.walk('./code_generator/'):
    for one_file in files:
        generator_list.append(os.stat(root+'/'+one_file).st_mtime)

#If code generator files are newer than generated files we should generate new files
if((not os.path.exists('../../nbservices/' + br + '/src/protocol/NBProtocolAvoid.cpp')) or os.stat('../../nbservices/' + br + '/src/protocol/NBProtocolAvoid.cpp').st_mtime < max(generator_list)):
  retVal = subprocess.call(["java", "-jar", "code_generator/tps.jar", "code_generator/res/CppConfiguration.xml"])

if retVal:
    sys.exit(retVal)

f = open(NBM_VERSION_FILE, "r")
NBM_VER = f.read();
NBM_FOLDER = "nbm_" + NBM_VER + "_android"
HOME_DIR = os.path.expanduser('~')

if BUILD_MACHINE == "parabuild":
  NBM_PATH = "\$\(SRCROOT\)/../../../../thirdparty/\$\(BRANCH\)/nbm/" + NBM_FOLDER
else: #"local"
  NBM_PATH = "\$\(SRCROOT\)/../../../../../../../core/releases/nbm/android/" + NBM_FOLDER

print(NBM_PATH)
#os.environ['NBM_PATH'] = NBM_PATH
#cmdStr = SRC_ROOT + "util/setNBMpath.sh " + HOME_DIR + " " + NBM_PATH
#retVal = subprocess.call(cmdStr,shell=True)
