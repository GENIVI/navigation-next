import os
import sys
import subprocess
import array
import shutil
import glob
import zlib
import struct
import time
import exceptions

def makedir(newdir):
    if os.path.isdir(newdir):
        pass
    elif os.path.isfile(newdir):
        raise OSError("a file with the same name as the desired " \
                      "dir, '%s', already exists." % newdir)
    else:
        head, tail = os.path.split(newdir)
        if head and not os.path.isdir(head):
            makedir(head)
        if tail:
            os.mkdir(newdir)  

def SimpleFileWrite(filepath, data):  
  f = open(filepath, 'w')
  f.write(data)
  f.close()
  
def SimpleFileRead(filepath):  
    f = open(filepath, 'r')
    ret = f.read()
    f.close()
    return ret  
  
def ValidateVersion(version):
    sect = version.strip().split('.')
    if len(sect) != 4:
      return False
    for num in sect:
      if not num.isdigit():
        return False
    return True
    
def ConstructVersion(version_in, version_gen, version_out, version_arg=False):
    #read version.txt    
    if version_arg == False:
      ret_version = SimpleFileRead(version_in)      
    else:
      ret_version = version_arg
    if ValidateVersion(ret_version) == False:
      return False
    version = ret_version.strip().split('.')
    major = version[0] 
    minor = version[1] 
    sub = version[2] 
    build = version[3]
    
    #read version_gen.h file
    versionfile = SimpleFileRead(version_gen)    
    versionfile = versionfile.replace("%MAJOR%", major)
    versionfile = versionfile.replace("%MINOR%", minor)
    versionfile = versionfile.replace("%SUB%", sub)
    versionfile = versionfile.replace("%BUILD%", build)
    
    #write version file
    SimpleFileWrite(version_out, versionfile)
    
    return ret_version

#-----------------------------------------------------------------------------------------------------------------------------------------#
#SETUP DIR STRUCTURE
#-----------------------------------------------------------------------------------------------------------------------------------------#
# Get the root directory of the project. 
# Assumes this py file is one directory below root directory
SOLUTION_DIR = os.path.abspath(sys.argv[1]) 

RESOURCE_DIR = SOLUTION_DIR + "/resources"
INCLUDE_DIR = SOLUTION_DIR + "/include"

version = ConstructVersion(RESOURCE_DIR + "/version.txt", RESOURCE_DIR + "/version.gen.txt", INCLUDE_DIR + "/cslversioninfo.h")
if version == False:
  print "ERROR: Version not correct x.x.x.x"
  sys.exit(1)
