import os
import sys
import subprocess
import array
import shutil
import glob
import zlib
import struct
import sha
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

#-----------------------------------------------------------------------------------------------------------------------------------------#
#SETUP DIR STRUCTURE
#-----------------------------------------------------------------------------------------------------------------------------------------#
# Get the root directory of the project. 
# Assumes this py file is one directory below root directory

TOOLS_DIR = os.path.abspath(sys.argv[1])
INC_DIR = os.path.abspath(sys.argv[2])
TITLE = sys.argv[3]
INTERFACE = sys.argv[4]
PLATFORM = sys.argv[5]

SOURCE_DIRS=INC_DIR
TMP_DIR = "\\tmp"
OUTPUT_DIR = "..\\output\\doxygen\\"+INTERFACE
REC = " "
INTERNAL_DOC = ""
DOXYGEN_TOOL = TOOLS_DIR + '\\gendoxygen'
SOURCE_DIRS += " " + INC_DIR + "\\" + PLATFORM + " "

if INTERFACE.lower() == "protected" or INTERFACE.lower() == "private":
  SOURCE_DIRS += INC_DIR + "\\protected "
  INTERNAL_DOC = " INTERNAL"

if INTERFACE == "private":
  SOURCE_DIRS += INC_DIR + "\\private "
  SOURCE_DIRS += "..\\src\\util "
  SOURCE_DIRS += "..\\src\\" + PLATFORM + " "
  REC += "rec"

TITLE += INTERFACE

SOURCE_DIRS = '"' + SOURCE_DIRS + '"'

makedir(TMP_DIR)

cmdStr = "@call " + TOOLS_DIR + "\\gendoxygen.cmd " + '\"' + TITLE + '\"' + " " + SOURCE_DIRS + " " + TMP_DIR + " " + OUTPUT_DIR + REC + INTERNAL_DOC

print cmdStr

retVal = subprocess.call(cmdStr,shell=True)

sys.exit(1)
