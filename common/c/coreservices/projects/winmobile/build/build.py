import os
import sys
import subprocess
import array
import shutil
import glob
import getopt

import build_utils; build_utils.TABDEPTH = 0

#-----------------------------------------------------------------------------------------------------------------------------------------#
#SETUP DIR STRUCTURE
#-----------------------------------------------------------------------------------------------------------------------------------------#
#Get the root directory of the project. 
#Assumes this py file is one directory below root directory
SOLUTIONDIR = os.path.abspath(sys.argv[0]) 
if SOLUTIONDIR.count('\\') == 0:
  SCRIPTNAME = SOLUTIONDIR
  SOLUTIONDIR = os.getcwd()   
else:
  SCRIPTNAME = SOLUTIONDIR[SOLUTIONDIR.rindex('\\')+1:]
  SOLUTIONDIR = SOLUTIONDIR[0:SOLUTIONDIR.rindex('\\')]
  SOLUTIONDIR = SOLUTIONDIR[0:SOLUTIONDIR.rindex('\\')]
  
PROJECTDIR = os.path.abspath(sys.argv[0]) 
if PROJECTDIR.count('\\') == 0:
  SCRIPTNAME = PROJECTDIR
  PROJECTDIR = os.getcwd()   
else:
  SCRIPTNAME = PROJECTDIR[PROJECTDIR.rindex('\\')+1:]
  PROJECTDIR = PROJECTDIR[0:PROJECTDIR.rindex('\\')]
  PROJECTDIR = PROJECTDIR[0:PROJECTDIR.rindex('\\')]

SOLUTIONDIR = SOLUTIONDIR.replace('\\','/')
PROJECTDIR = PROJECTDIR.replace('\\','/')
MAIN_BUILDDIR  = PROJECTDIR + "/build/"
BUILD_DIRFILE = MAIN_BUILDDIR+"bd.txt"
PROFILE_DIRFILE = MAIN_BUILDDIR+"prof.txt"

if not os.path.isfile("../../NIMCoreServices.sln"):
  print "Unable to find project directory!"
  sys.exit()

# set import path to include all our local python scripts
sys.path.append(SOLUTIONDIR + "/build/")

CORETECH_BUILDDIR = ""

#-----------------------------------------------------------------------------------------------------------------------------------------#
#SETUP COMMANDLINE ARGS and GLOBAL ARGS
#----------------------------------------------------------------------------------------------------------------------------------------#
PROFILE = "Debug"
BUILD_SAMPLE = False
BUILD_NIMCORE = True
BUILD_DOC = False
COPY_TO_BUILD = True
RUN_SETUP = False

for arg in sys.argv:
  arg = arg.upper()  
  if arg == "-X":
    RUN_SETUP = True
  if arg == "-D":
    PROFILE = "Debug"
  if arg == "-R":
    PROFILE = "Release"
  if arg == "-S":
    BUILD_SAMPLE = True
  if arg == "NIMCORE" or arg == "ALL":
    BUILD_NIMCORE = True
  if arg == "DOC":
    BUILD_DOC = True
  if arg.find("BUILDDIR:") != -1:
    bd = arg.lstrip("BUILDDIR:")    
    if bd != "":    
      bd = bd.lower()
      build_utils.SimpleFileWrite(BUILD_DIRFILE, bd)
  if arg.find("DOXY:") != -1:
    DOXY = arg.lstrip("DOXY:")
    
OUTDIR = PROJECTDIR + '/bin/' + PROFILE + '/'

MSPATH="C:/WINDOWS/Microsoft.NET/Framework/v2.0.50727/"
if not os.path.isfile(MSPATH + 'msbuild.exe'):
  MSPATH = ""
   
#-----------------------------------------------------------------------------------------------------------------------------------------#
#SHOW BUILDDIR MESSAGE
#-----------------------------------------------------------------------------------------------------------------------------------------#
if not os.path.isfile(BUILD_DIRFILE):
  response = raw_input('Use P4 root directory as default build directory? (Y/N): ')
  if response.upper() == 'Y':
    P4ROOT = build_utils.GetP4Root()
    if P4ROOT == '': 
      print "Cannot get P4 root"
      P4ROOT = build_utils.PromptForDirectory("Please specify P4ROOT: ")      
    else:
      P4ROOT = build_utils.SanitizeDir(P4ROOT) 
    CORETECH_BUILDDIR = P4ROOT + "coretech/" + "build/"    
  else:
    dir = build_utils.PromptForDirectory("Set default build directory: '")    
    CORETECH_BUILDDIR = dir
  build_utils.SimpleFileWrite(BUILD_DIRFILE, CORETECH_BUILDDIR)
else:
  dir = build_utils.SimpleFileRead(BUILD_DIRFILE)
  dir = build_utils.SanitizeDir(dir)    
  CORETECH_BUILDDIR = dir  

if CORETECH_BUILDDIR == "":
  print "No build directory specified, exiting"
  sys.exit(1)

CORETECH_BUILDDIR = CORETECH_BUILDDIR + PROFILE + '/'
  
#-----------------------------------------------------------------------------------------------------------------------------------------#
#SHOW OPTIONS
#-----------------------------------------------------------------------------------------------------------------------------------------#  
options = "PROFILE: " + PROFILE + "\n"
options += "PROJECT: " + "NIM CORE" + "\n"
options += "OUTPUT DIR: " + CORETECH_BUILDDIR
  
build_utils.PrintHeader("BUILD PARAMETERS")
print options

#-----------------------------------------------------------------------------------------------------------------------------------------#
#BUILD AND PACKAGE NIMCORE
#-----------------------------------------------------------------------------------------------------------------------------------------#
if BUILD_NIMCORE == True:
  build_utils.PrintHeader("BUILDING NIM CORE SERVICES")
  #build project    
  build_utils.buildProj(MSPATH,"../../CoreGPS.vcproj",PROFILE,"CoreGPS")
  build_utils.buildProj(MSPATH,"../../CoreNetwork.vcproj",PROFILE,"CoreNetwork")
  build_utils.buildProj(MSPATH,"../../CoreTPS.vcproj",PROFILE,"CoreTPS")
  build_utils.buildProj(MSPATH,"../../CoreUtil.vcproj",PROFILE,"CoreUtil")
  build_utils.buildProj(MSPATH,"../../NIMCoreLib.vcproj",PROFILE,"NIMCoreLib")
  
  #create SDK dirs
  PROFILE_DIR = PROJECTDIR + '/bin/' + PROFILE + '/'
  SDKDIR = PROFILE_DIR + 'NimCoreSDK/'
  build_utils.makedirclean(SDKDIR) 

  SDKDIRHEADERS = SDKDIR + 'headers/'
  build_utils.makedirclean(SDKDIRHEADERS)
  
  SDKDIRCORE = SDKDIR + '/headers/core/'
  build_utils.makedirclean(SDKDIRCORE)
    
  SDKDIRLIB = SDKDIR + 'lib/'
  build_utils.makedirclean(SDKDIRLIB)
  
  SDKDIRBIN = SDKDIR + 'bin/'
  build_utils.makedirclean(SDKDIRLIB)

  #copy files    
  build_utils.copyfiles(MAIN_BUILDDIR+"setup_nimcore.bat", SDKDIR)  
  build_utils.copyfiles(MAIN_BUILDDIR+"setenv.exe", SDKDIR)
  build_utils.copyfiles(OUTDIR+"NIMCoreLib.lib", SDKDIRLIB)  
  build_utils.copyfiles(OUTDIR+"NIMCoreLib.bin", SDKDIRBIN)  
  build_utils.copydir_ext("../../src", SDKDIRCORE, ".h")
  

  #copy files to build  
  if COPY_TO_BUILD == True:
    build_utils.PrintHeader("COPYING FILES TO BUILD DIR")
    if not os.path.isdir(CORETECH_BUILDDIR):
      build_utils.makedirclean(CORETECH_BUILDDIR)
      build_utils.copydir(SDKDIR, CORETECH_BUILDDIR)
    else:
      if not os.path.isdir(CORETECH_BUILDDIR + "bin"):
        build_utils.makedirclean(CORETECH_BUILDDIR + "bin")
      build_utils.copyfiles(SDKDIRBIN+"/*.*", CORETECH_BUILDDIR + "bin")
      build_utils.copyfiles(SDKDIRLIB + "/*.*", CORETECH_BUILDDIR + "lib")        
      build_utils.copydir(SDKDIRCORE, CORETECH_BUILDDIR + "headers/core")
      build_utils.copyfiles(SDKDIR+"/"+"setup_nimcore.bat", CORETECH_BUILDDIR)
      build_utils.copyfiles(SDKDIR+"/"+"setenv.exe", CORETECH_BUILDDIR)
    print "Done copying files \n\n"
  if BUILD_DOC == True:
    build_utils.PrintHeader("BUILDING NIMPAL DOCUMENTATION")
    if not os.path.isdir(DOXY):
      print "Cannot find doxygen directory.  please install doxygen and set %DOXYGEN% environment variable to doxygen directory"
      print "\n\n"
    else:
      build_utils.buildDoxygen(DOXY + "/bin/", "nimcore", CORETECH_BUILDDIR + "/headers/core", CORETECH_BUILDDIR + "/doc/nimcore")
      print "Done with doxygen \n\n"
  build_utils.SimpleFileWrite(PROFILE_DIRFILE, CORETECH_BUILDDIR)
  
else:
  build_utils.PrintHeader("BUILDING NOTHING")
