import os
import sys
import subprocess

from package import MakeRelease
import build_utils; build_utils.TABDEPTH = 0
from codegenerator import GenerateCode

VARS_DICT = {}
SET_GLOBAL_VARS = True

def SetEnvironment(dictionary):
    if sys.platform.startswith("win") and SET_GLOBAL_VARS:
        import _winreg
        #Open the registry key where the environment variables are stored on Windows
        regkey = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment", 0, _winreg.KEY_WRITE)

        for key in VARS_DICT.keys():
            value = VARS_DICT[key]
            _winreg.SetValueEx(regkey, key, 0, _winreg.REG_SZ, value)
            print key, ":", value

        _winreg.CloseKey(regkey)
        subprocess.call(os.path.join("util", "setenv.exe"))

    os.environ.update(VARS_DICT)

def AddToVarList(project, inc, lib, bin = None):
    global VARS_DICT

    if project in ALIAS_DICT.keys():
        project = ALIAS_DICT[project]
    else:
        project = "NIM" + project.upper()

    VARS_DICT[project + "_INC"] = inc
    VARS_DICT[project + "_LIB"] = lib
    VARS_DICT[project + "_BIN"] = (bin != None and bin or lib)


MAIN_LIST = ["thirdparty", "nbpal", "abpal", "nbgm", "coreservices", "nbservices", "abservices", "nav", "search", "nbui", "systemtests"]
BUILD_LIST = MAIN_LIST
THIRDPARTY_LIST = ["cunit", "sqlite", "argtable", "speex"]
NBGM_LIST = [ "nbgmmain", "nbre", "rendersystem" ]
NBUI_LIST = ["nbui"]

# For compatibility with old project files
ALIAS_DICT = {
    "nbpal": "NIMNBIPAL",
    "coreservices": "NIMCORE",
    "nbservices": "NIMNB",
    "abservices": "NIMAB",
    }

TARGET = "win32"
PROFILE = "release"
PARABUILD = "no"
CCC_MODE = "offboard"

if len(sys.argv) > 1:
    TARGET = sys.argv[1].lower()
if len(sys.argv) > 2:
    PROFILE = sys.argv[2].lower()
if len(sys.argv) > 3:
    if sys.argv[3].lower() == "parabuild":
        PARABUILD = "yes"

if len(sys.argv) > 4:
    CCC_MODE = sys.argv[4].lower()

if TARGET == "winmobile":
    TARGET_SHORT = "wm"
elif TARGET == "win32":
    TARGET_SHORT = "w32"
elif TARGET == "winrt":
    TARGET_SHORT = "winrt"
elif TARGET == "brew":
    TARGET_SHORT = "brew"

BUILD_PATH, BRANCH_NAME = os.path.split(os.getcwd())
ROOT_PATH, _ = os.path.split(BUILD_PATH)

def configure():
    for project in MAIN_LIST:
        PROJECT_PATH = os.path.join(ROOT_PATH, project, BRANCH_NAME)
        AddToVarList(project, os.path.join(PROJECT_PATH, "include"), os.path.join(PROJECT_PATH, "output", TARGET, PROFILE))

    for project in THIRDPARTY_LIST:
        PROJECT_PATH = os.path.join(ROOT_PATH, "thirdparty", BRANCH_NAME, project)
        AddToVarList(project, os.path.join(PROJECT_PATH, "include"), os.path.join(ROOT_PATH, "thirdparty", BRANCH_NAME, "output", TARGET, PROFILE))

    for project in NBGM_LIST:
        PROJECT_PATH = os.path.join(ROOT_PATH, "nbgm", BRANCH_NAME, project)
        AddToVarList(project, os.path.join(PROJECT_PATH, "include"), os.path.join(ROOT_PATH, "nbgm", BRANCH_NAME, "output", TARGET, PROFILE))

    for project in NBUI_LIST:
        PROJECT_PATH = os.path.join(ROOT_PATH, project, BRANCH_NAME)
        AddToVarList(project, os.path.join(PROJECT_PATH, "include"), os.path.join(PROJECT_PATH, "output", TARGET, PROFILE))

    NBM_VERSION = build_utils.SimpleFileRead("core_nbm_lib_version.txt").strip()

    if PARABUILD == "no":
        NBM_PATH = "..\\..\\..\\..\\..\\..\\..\\core\\releases\\nbm\\" + TARGET + "\\" + "nbm_" + NBM_VERSION + "_" + TARGET
    else:
        NBM_PATH = "..\\..\\..\\..\\core\\releases\\nbm\\" + TARGET + "\\" + "nbm_" + NBM_VERSION + "_" + TARGET

    AddToVarList("NBM", os.path.join(NBM_PATH, "include"), os.path.join(NBM_PATH, "libs"))
    global VARS_DICT
    VARS_DICT["NIM_CCC_BRANCH"] = BRANCH_NAME


    NCDB_VERSION = build_utils.SimpleFileRead("core_ncdb_lib_version.txt")

    if PARABUILD == "no":
        NCDB_PATH = "..\\..\\..\\..\\..\\..\\..\\core\\releases\\ncdb\\" + TARGET + "\\" + "ncdb_" + NCDB_VERSION + "_" + TARGET
    else:
        NCDB_PATH = "..\\..\\..\\..\\core\\releases\\ncdb\\" + TARGET + "\\" + "ncdb_" + NCDB_VERSION + "_" + TARGET

    AddToVarList("NCDB", os.path.join(NCDB_PATH, "include"), NCDB_PATH)

    SetEnvironment(VARS_DICT)

    #-----------------------------------------------------------------------------------------------------------------------------------------#
    # Set CCC mode - hybrid or offboard
    #-----------------------------------------------------------------------------------------------------------------------------------------#
    br = os.getcwd().split('\\')
    br = br[len(br) - 1]
    use_onboard_file = open('../../nbservices/' + br + '/include/private/useonboard.h',"w")
    if CCC_MODE == "onboard":
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n//#define OFFBOARD_SERVICE_ONLY\n#endif')
    else:
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n#define OFFBOARD_SERVICE_ONLY\n#endif')
    use_onboard_file.close()


def build():
    try:
        if TARGET == "win32":
            MSPATH = "C:/WINDOWS/Microsoft.NET/Framework/v4.0.30319/" # VS11
        else:
            MSPATH = "C:/WINDOWS/Microsoft.NET/Framework/v2.0.50727/" # VS8

        if TARGET == "winmobile":
            BUILD_PLATFORM = "Windows Mobile 6 Standard SDK (ARMV4I)"
        else:
            BUILD_PLATFORM = "Win32"

        for project in BUILD_LIST:
            PROJECT_PATH = os.path.join(ROOT_PATH, project, BRANCH_NAME)
            SOLUTION_PATH = os.path.join(PROJECT_PATH, project + "_" + TARGET_SHORT + ".sln")
            build_utils.PrintHeader("BUILDING " + project)
            build_utils.buildSln(MSPATH, SOLUTION_PATH, PROFILE, project, BUILD_PLATFORM)

        MakeRelease(TARGET_SHORT, PROFILE)
        print "\n\nBUILD SUCCESSFUL\n"
    except Exception, err:
        sys.stderr.write('ERROR: %s\n' % str(err))
        traceback.print_exc()
        print "\n\nBUILD FAILED\n"

if __name__ == '__main__':
    # Don't change global environment during the build
    SET_GLOBAL_VARS = False
    GenerateCode()
    configure()
    build()
