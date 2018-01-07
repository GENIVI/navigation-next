import os
import sys
import subprocess
import array
import re
from subprocess import *

def run_cmd(cmd):     #run os command and return result as string
    p = Popen(cmd, shell=True, stdout=PIPE)
    output = p.communicate()[0]
    return output

def replaceInFile(file, old, new):
  fin = open(file, "r")
  origStr = fin.read()
  fin.close()
  rc = re.compile("(?<=[/\\\\])" + old + "(?![\\.])")
  resStr = rc.sub(new, origStr)
  fout = open(file, "w")
  fout.write(resStr)
  fout.close()

OLD_BRANCH = "invalid"
NEW_BRANCH = "invalid"
EXT_LIST = [ "pbxproj", "txt", "vsprops", "vcproj", "bat", "mk", "pro", "project", "cproject" ]
DIR_LIST = [ "abpal", "abservices", "build", "coreservices", "nbgm", "nbpal", "nbservices", "systemtests", "thirdparty" ]
CORE_DIR = "../../../"
FileList = []

if len(sys.argv) > 1:
  OLD_BRANCH = sys.argv[1]
if len(sys.argv) > 2:
  NEW_BRANCH = sys.argv[2]

if(OLD_BRANCH == "invalid") | (NEW_BRANCH == "invalid"):
  print "python update_branch.py [old_branch_name] [new_branch_name]"
  sys.exit()

#search all appropriate files
print "Looking for files:\n"
for dir in DIR_LIST:
  CURRENT_DIR = CORE_DIR + dir + "/" + NEW_BRANCH
  print CURRENT_DIR
  for root, dirs, files in os.walk(CURRENT_DIR):
    for ext in EXT_LIST:
      FileList += [os.path.join(root, name) for name in files if name[-len(ext):] == ext]

print "Files found, creating PCL...\n"


P4CLIENT_NAME = run_cmd("p4 set P4CLIENT")
P4CLIENT_NAME = P4CLIENT_NAME[9:]
P4USER = run_cmd("p4 set P4USER")
P4USER = P4USER[7:]

clfilename = "cldescr.111"
fcl = open(clfilename, "w")
fcl.write("Change:\tnew\n")
fcl.write("Client:\t" + P4CLIENT_NAME + "\n")
fcl.write("User:\t" + P4USER + "\n")
fcl.write("Status:\tnew\n")
fcl.write("Description:\tCreated by update_branch.py script\n")
fcl.close()
CLNUM = run_cmd("cat " + clfilename + "| p4 change -i")
CLNUM = CLNUM[7:]
CLNUM = CLNUM[:-11]
os.remove(clfilename)
print "PCL created, checking out files...\n"

#process each file
for file in FileList:
  f = open(file, "r")
  data = f.read()
  f.close
  if OLD_BRANCH in data:  #if file contain old branch name - we should update it
    cmd = "p4 edit -c " + CLNUM + " " + '"' + file + '"'
    os.system(cmd)
    replaceInFile(file, OLD_BRANCH, NEW_BRANCH)

cmd = "p4 revert -a -c " + CLNUM
os.system(cmd)
print "Done!"
