import os
import sys
import subprocess
import array
import shutil
import glob

import build_utils; build_utils.TABDEPTH = 0
import config
import package

#-----------------------------------------------------------------------------------------------------------------------------------------#
#Get Configuration from Config file
#-----------------------------------------------------------------------------------------------------------------------------------------#
MSPATH="C:/WINDOWS/Microsoft.NET/Framework/v2.0.50727/"
CFG_FILE = ""
BUILD_NIMNBIPAL = True
BUILD_CFG = config.ConfigParams("", False, "", "", False, "", False, "", "")
PAL_LIST = ["ABPAL"] 
SERVICES_LIST = [ "CORE","NB", "AB"] 
CUNIT_LIST = [ "THIRDPARTY" ]
TEST_LIST = [ "SYSTEMTESTS" ]
BUILD_PLATFORM="Windows Mobile 6 Standard SDK (ARMV4I)"
CFG_PLAT = "winmobile"

CFG_FILE = sys.argv[1]
if len(sys.argv) > 2:
  CFG_PLAT = sys.argv[2]

if CFG_PLAT == "winmobile":
  CFG_PLAT = "wm"
elif CFG_PLAT == "win32":
  CFG_PLAT = "w32"

#-----------------------------------------------------------------------------------------------------------------------------------------#
#Build projects
#-----------------------------------------------------------------------------------------------------------------------------------------#
for project in CUNIT_LIST:
  nimproject = "NIM" + project
  build_utils.GetCfgParams(CFG_FILE, nimproject, BUILD_CFG)
  if BUILD_CFG.build == True:
    lproject=project.lower()
    lprojectservices = project.lower()
    build_utils.PrintHeader("BUILDING " + project)  
    SOLUTION_PATH=BUILD_CFG.path+"/"+lprojectservices+"_" + CFG_PLAT + ".sln"
    build_utils.buildSln(MSPATH,SOLUTION_PATH,BUILD_CFG.profile,lprojectservices,BUILD_PLATFORM)

# temporary change to build nbpal
build_utils.GetCfgParams(CFG_FILE, "NIMNBIPAL", BUILD_CFG)
if BUILD_CFG.build == True:
  build_utils.PrintHeader("BUILDING " + project)
  SOLUTION_PATH=BUILD_CFG.path+"/nbpal_" + CFG_PLAT + ".sln"
  build_utils.buildSln(MSPATH,SOLUTION_PATH,BUILD_CFG.profile,"nimnbipal",BUILD_PLATFORM)
  if BUILD_CFG.package == True:
    package.PackageBuild("nbpal", BUILD_CFG)

for project in PAL_LIST:
  build_utils.GetCfgParams(CFG_FILE, "NIM" + project, BUILD_CFG)
  if BUILD_CFG.build == True:   
    lproject = project.lower()
    build_utils.PrintHeader("BUILDING " + project)  
    SOLUTION_PATH=BUILD_CFG.path+"/"+lproject+"_" + CFG_PLAT + ".sln"
    build_utils.buildSln(MSPATH,SOLUTION_PATH,BUILD_CFG.profile,lproject,BUILD_PLATFORM)
    if BUILD_CFG.package == True:
      package.PackageBuild(lproject, BUILD_CFG)
    
for project in SERVICES_LIST:
  build_utils.GetCfgParams(CFG_FILE, "NIM" + project, BUILD_CFG)
  if BUILD_CFG.build == True:
    lproject=project.lower()
    lprojectservices = project.lower()+"services"
    build_utils.PrintHeader("BUILDING " + project)  
    SOLUTION_PATH=BUILD_CFG.path+"/"+lprojectservices+"_" + CFG_PLAT + ".sln"
    build_utils.buildSln(MSPATH,SOLUTION_PATH,BUILD_CFG.profile,lprojectservices,BUILD_PLATFORM)
    if BUILD_CFG.package == True:
      package.PackageBuild(lprojectservices, BUILD_CFG, lproject[3:])

for project in TEST_LIST:
  BUILD_CFG.project=""
  BUILD_CFG.path=""
  build_utils.GetCfgParams(CFG_FILE, project, BUILD_CFG)  
  if BUILD_CFG.build == True and BUILD_CFG.path != "":
    lproject=project.lower()
    lprojectservices = project.lower()
    build_utils.PrintHeader("BUILDING " + project)  
    SOLUTION_PATH=BUILD_CFG.path+"/"+lprojectservices+"_" + CFG_PLAT + ".sln"
    build_utils.buildSln(MSPATH,SOLUTION_PATH,BUILD_CFG.profile,lprojectservices,BUILD_PLATFORM)
