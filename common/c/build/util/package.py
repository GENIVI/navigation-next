import os
import sys
import subprocess
import array
import shutil
import glob

import build_utils; build_utils.TABDEPTH = 0

def MakeRelease(plat, profile):
  NBPAL_VERSION=""
  ABPAL_VERSION=""
  CORESERVICES_VERSION=""
  NBSERVICES_VERSION=""
  ABSERVICES_VERSION=""

  line = os.path.abspath('')
  splitPath = line.split('\\')
  #CURRENT_DIRECTORY is a branch name
  CURRENT_DIRECTORY = splitPath[len(splitPath)-1]

  CFG_POSTFIX=""
  PROFILE=profile

  if plat=="w32":
    CFG_POSTFIX="win32"
  elif plat=="wm":
    CFG_POSTFIX="winmobile"
  elif plat=="brew_arm" or plat=="brew":
    CFG_POSTFIX="brew"
  elif plat == "qt_qnx":
    CFG_POSTFIX = "qt_qnx"
  else:
    CFG_POSTFIX=plat

  LIB_VERSION = build_utils.SimpleFileRead("../../build/"+CURRENT_DIRECTORY+"/version_"+CFG_POSTFIX+".txt")
  BASE_PACKAGE_DIR="CoreSDK_"+LIB_VERSION+"_"+CFG_POSTFIX+"_internal"
  BIN_PACKAGE_DIR="CoreSDK_"+LIB_VERSION+"_"+CFG_POSTFIX+"_internal"+"/bin"
  INC_DIRECTORY = "CoreSDK_"+LIB_VERSION+"_"+CFG_POSTFIX+"_internal"+"/include"
  PLATFORM_INC_DIRECTORY=INC_DIRECTORY + "/" + CFG_POSTFIX
  
  NBPAL_VERSION = build_utils.SimpleFileRead("../../nbpal/"+CURRENT_DIRECTORY+"/resources/"+CFG_POSTFIX+"/version.txt")
  ABPAL_VERSION = build_utils.SimpleFileRead("../../abpal/"+CURRENT_DIRECTORY+"/resources/"+CFG_POSTFIX+"/version.txt")
  CORESERVICES_VERSION = build_utils.SimpleFileRead("../../coreservices/"+CURRENT_DIRECTORY+"/resources/version.txt")
  NBSERVICES_VERSION = build_utils.SimpleFileRead("../../nbservices/"+CURRENT_DIRECTORY+"/resources/version.txt")
  ABSERVICES_VERSION = build_utils.SimpleFileRead("../../abservices/"+CURRENT_DIRECTORY+"/resources/version.txt")
  NBGM_VERSION = build_utils.SimpleFileRead("../../nbgm/"+CURRENT_DIRECTORY+"/resources/"+CFG_POSTFIX+"/version.txt")
  
  NBPAL_OUTPUT_DIR = "../../nbpal/"+CURRENT_DIRECTORY+"/output"
  build_utils.makedir(NBPAL_OUTPUT_DIR)
  
  ABPAL_OUTPUT_DIR = "../../abpal/"+CURRENT_DIRECTORY+"/output"
  build_utils.makedir(ABPAL_OUTPUT_DIR)
  
  CORESERVICES_OUTPUT_DIR = "../../coreservices/"+CURRENT_DIRECTORY+"/output"
  build_utils.makedir(CORESERVICES_OUTPUT_DIR)
  
  NBSERVICES_OUTPUT_DIR = "../../nbservices/"+CURRENT_DIRECTORY+"/output"
  build_utils.makedir(NBSERVICES_OUTPUT_DIR)
  
  ABSERVICES_OUTPUT_DIR = "../../abservices/"+CURRENT_DIRECTORY+"/output"
  build_utils.makedir(ABSERVICES_OUTPUT_DIR)
  
  NBGM_OUTPUT_DIR = "../../nbgm/"+CURRENT_DIRECTORY+"/output"
  build_utils.makedir(NBGM_OUTPUT_DIR)

  build_utils.makedir(BIN_PACKAGE_DIR)
  build_utils.makedir(INC_DIRECTORY)
  build_utils.makedir(PLATFORM_INC_DIRECTORY)
  
  build_utils.SimpleFileWrite(BASE_PACKAGE_DIR+"/manifest.txt", "name: CoreSDK\n")
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "version: %s\n" % LIB_VERSION)
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "platform: "+CFG_POSTFIX+"\n")
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "profile: %s\n" % PROFILE)
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "abpal: %s\n" % ABPAL_VERSION)
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "nbservices: %s\n" % NBSERVICES_VERSION)
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "abservices: %s\n" % ABSERVICES_VERSION)
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "nbpal: %s\n" % NBPAL_VERSION)
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "coreservices: %s\n" % CORESERVICES_VERSION)
  build_utils.appendfile(BASE_PACKAGE_DIR+"/manifest.txt", "nbgm: %s\n" % NBGM_VERSION)

  #Copy libs into release folder

  # We currently don't use version numbers anymore for the lib files. If we want to use version numbers again,
  # then we can comment out this code.
#  version = LIB_VERSION.split('.')
#  LIB_MAJOR_MINOR = version[0] + version[1]
  LIB_MAJOR_MINOR = ""
  
  ext = ".lib"
  prefix = ""
  if plat == "qt_qnx":
    ext = ".a"
    prefix = "lib"

  if plat != "brew_arm":
    build_utils.copy("../../nbpal/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"nbpal"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/nbpal"+LIB_MAJOR_MINOR+ext)
    build_utils.copy("../../abpal/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"abpal"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/abpal"+LIB_MAJOR_MINOR+ext)
    build_utils.copy("../../coreservices/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"coreservices"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/coreservices"+LIB_MAJOR_MINOR+ext)
    build_utils.copy("../../nbservices/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"nbservices"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/nbservices"+LIB_MAJOR_MINOR+ext)
    build_utils.copy("../../abservices/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"abservices"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/abservices"+LIB_MAJOR_MINOR+ext)
    build_utils.copy("../../nbgm/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"nbgm"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/nbgm"+LIB_MAJOR_MINOR+ext)
    build_utils.copy("../../nbui/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"nbui"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/nbui"+LIB_MAJOR_MINOR+ext)
    build_utils.copy("../../nav/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/"+prefix+"nav"+LIB_MAJOR_MINOR+ext,BIN_PACKAGE_DIR+"/nav"+LIB_MAJOR_MINOR+ext)

  #Copy dll into release folder
  if CFG_POSTFIX.startswith("win") and not profile.lower().endswith("static"):
    build_utils.copy("../../nbpal/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/nbpal"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/nbpal"+LIB_MAJOR_MINOR+".dll")
    build_utils.copy("../../abpal/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/abpal"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/abpal"+LIB_MAJOR_MINOR+".dll")
    build_utils.copy("../../coreservices/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/coreservices"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/coreservices"+LIB_MAJOR_MINOR+".dll")
    build_utils.copy("../../nbservices/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/nbservices"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/nbservices"+LIB_MAJOR_MINOR+".dll")
    build_utils.copy("../../abservices/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/abservices"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/abservices"+LIB_MAJOR_MINOR+".dll")
    build_utils.copy("../../nbgm/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/nbgm"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/nbgm"+LIB_MAJOR_MINOR+".dll")
    build_utils.copy("../../nbui/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/nbui"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/nbui"+LIB_MAJOR_MINOR+".dll")
    build_utils.copy("../../nav/"+CURRENT_DIRECTORY+"/output/"+CFG_POSTFIX+"/"+PROFILE+"/nav"+LIB_MAJOR_MINOR+".dll",BIN_PACKAGE_DIR+"/nav"+LIB_MAJOR_MINOR+".dll")

  #Copy headers into release folder
  build_utils.copyfiles("../../nbpal/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../nbpal/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../abpal/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../abpal/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../coreservices/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../coreservices/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../nbservices/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../nbservices/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../abservices/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../abservices/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../nbgm/"+CURRENT_DIRECTORY+"/nbgmmain/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../nbgm/"+CURRENT_DIRECTORY+"/nbgmmain/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../nbui/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../nbui/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../nav/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../nav/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)

  build_utils.copyfiles("../../thirdparty/"+CURRENT_DIRECTORY+"/include/*.*", INC_DIRECTORY)
  build_utils.copyfiles("../../thirdparty/"+CURRENT_DIRECTORY+"/include/"+CFG_POSTFIX+"/*.*", PLATFORM_INC_DIRECTORY)
  build_utils.copyfiles("../../thirdparty/"+CURRENT_DIRECTORY+"/sqlite/include/*.*", PLATFORM_INC_DIRECTORY)
  build_utils.copyfiles("../../thirdparty/"+CURRENT_DIRECTORY+"/lp150b15/*.h", PLATFORM_INC_DIRECTORY)

  if CFG_POSTFIX.startswith("win"):
    build_utils.copyfiles("../../thirdparty/"+CURRENT_DIRECTORY+"/projects/win32/libpng/visualc71/zlib/*.h", PLATFORM_INC_DIRECTORY)

  # TPSLIB_TEMPLATES_FILE = "tpslib_templates.txt"
  # #Copy tpslib templates into SDK release folder
  # build_utils.copy(TPSLIB_TEMPLATES_FILE, BASE_PACKAGE_DIR+"/"+TPSLIB_TEMPLATES_FILE)

  # #Build tpslib.txt for systemtests
  # TPS_CAPABILITIES_FILE = "tpslib_capabilities.txt"
  # TPS_SYSTEMTESTS_PATH = "../../systemtests/"+CURRENT_DIRECTORY+"/tpslib/"

  # TPSLIB_FILE = "tpslib.txt"

  # build_utils.SimpleFileWrite(TPS_SYSTEMTESTS_PATH+TPSLIB_FILE, build_utils.SimpleFileRead(TPS_SYSTEMTESTS_PATH+TPS_CAPABILITIES_FILE))
  # build_utils.appendfile(TPS_SYSTEMTESTS_PATH+TPSLIB_FILE, build_utils.SimpleFileRead(TPSLIB_TEMPLATES_FILE))
