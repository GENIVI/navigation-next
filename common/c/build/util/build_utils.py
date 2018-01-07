import os
import glob
import zlib
import struct
import sha
import subprocess
import sys
import shutil
import time
import exceptions

TABDEPTH = 0
STRING_NOT_FOUND = -1

#--------...--------...--------...--------...--------...--------...--------...--------

def msg(str,tab):
    global TABDEPTH
    l = len(str)
    if l > 0:
        print str.rjust(l+(TABDEPTH*2),' ')
    TABDEPTH += tab


#--------...--------...--------...--------...--------...--------...--------...--------
def chmod(filename):
    if os.path.isfile(filename):
        os.chmod(filename,0777)
#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------
def suckfile(filename):
    f = open(filename,'r')
    str = f.read()
    f.close()
    return str

def appendfile(filename, text):
        f = open ( filename, 'a' )
        f.write ( text )
        f.close()

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------
# file and dir removal

def removeitem(path,isFile):
    try:
        if isFile:
            if os.path.isfile(path):
                chmod(path)
                os.remove(path)
        else:
            if os.path.isdir(path):
                os.rmdir(path)
    except OSError, (errno,strerror):
        print """Error removing %(path)s, %(error)s """ % {'path' : path, 'error': strerror }

def removeitems(path):
    try:
        files = glob.glob(path)
        for fileName in files:
            if os.path.isfile(fileName):
                chmod(fileName)
                os.remove(fileName)
    except OSError, (errno,strerror):
        print """Error removing %(path)s, %(error)s """ % {'path' : path, 'error': strerror }

def removedir(path):
    if os.path.isdir(path):
        files = os.listdir(path)
        for entry in files:
            fullpath = os.path.join(path,entry)
            if os.path.isfile(fullpath):
                removeitem(fullpath,True)
            elif os.path.isdir(fullpath):
                removedir(fullpath)
                removeitem(fullpath,False)
        removeitem(path,False)

def prunedir(path):
    for curr, dirs, files in os.walk(path, topdown=False):
        if os.path.isdir(curr):
            removeitems(curr + '\*.*')
            removeitem(curr,False)

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

def copy(srcFile,dstFile):
    try:
        shutil.copy(srcFile,dstFile)
        chmod(dstFile)
    except OSError, (errno,strerror):
        print """Error copying %(path)s, %(error)s """ % {'path' : srcFile, 'error': strerror }

def copyfiles(fspec,dstDir):
    try:
        files = glob.glob(fspec)
        for fileName in files:
            fname = os.path.basename(fileName)
            dstFile = os.path.join(dstDir,fname)
            chmod(dstFile)
            copy(fileName, dstFile)
    except OSError, (errno,strerror):
        print """Error copying files %(path)s, %(error)s """ % {'path' : fspec, 'error': strerror }

def copyfilesprefix(fspec,dstDir,append):
    try:
        files = glob.glob(fspec)
        for fileName in files:
            fname = os.path.basename(fileName)
            dstFile = os.path.join(dstDir,fname)
            chmod(dstFile)
            copy(fileName, dstFile)

    except OSError, (errno,strerror):
        print """Error copying files %(path)s, %(error)s """ % {'path' : fspec, 'error': strerror }

def makedirclean(DIR):
    if not os.path.isdir(DIR):
      makedir(DIR)
    else:
      removeitems(DIR + "/*.*")

def copydir(src, dst, symlinks=False, excl=None):
    names = os.listdir(src)
    makedirclean(dst)
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        if not excl == None:
            skip = False
            for e in excl:
                if name.find(e) > STRING_NOT_FOUND:
                    skip = True
                    break
            if skip == True:
                continue
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copydir(srcname, dstname, symlinks, excl)
            else:
                copy(srcname, dstname)
        except (IOError, os.error), why:
            errors.append((srcname, dstname, why))

def copydir_ext(src, dst, ext, symlinks=False):
    names = os.listdir(src)
    makedirclean(dst)
    errors = []
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        try:
            if symlinks and os.path.islink(srcname):
                linkto = os.readlink(srcname)
                os.symlink(linkto, dstname)
            elif os.path.isdir(srcname):
                copydir_ext(srcname, dstname, ext, symlinks)
            else:
                fext = os.path.splitext(srcname)
                if fext[-1] == ext:
                  copy(srcname, dstname)
        except (IOError, os.error), why:
            errors.append((srcname, dstname, why))

def file_exists(filepath):
  return os.path.isfile(filepath)

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def buildProj(MSPATH,PROJPROJ,PROFILE,PROJNAME):
    cmdStr = '''%smsbuild.exe /nologo %s \
                /p:configuration=%s \
                /p:Platforms="Windows Mobile 6 Standard SDK (ARMV4I)''' % (MSPATH,PROJPROJ,PROFILE)
    retVal = subprocess.call(cmdStr,shell=True)
    if retVal != 0:
        sys.exit("There were error(s) in building " + PROJNAME)

def buildSln(MSPATH,PROJPROJ,PROFILE,PROJNAME,BUILD_PLATFORM = None):
    setSlnVar='set MSBuildEmitSolution=1'
    cmdStr = '''%smsbuild.exe /nologo %s \
                /p:configuration="%s" \
                /t:rebuild''' % (MSPATH,PROJPROJ,PROFILE)
    if BUILD_PLATFORM:
        cmdStr += ''' /p:Platform="%s"''' % (BUILD_PLATFORM)

    print 'PROFILE: ' + PROFILE
    retVal = subprocess.call(setSlnVar,shell=True)
    retVal = subprocess.call(cmdStr,shell=True)
    if retVal != 0:
        sys.exit("There were error(s) in building " + PROJNAME)

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def buildDoxygen(doxygenPath, doxyProjectFile, srcDocPath, destDocPath):
        docWorkingDir = os.path.join(os.path.dirname(os.path.abspath(doxyProjectFile)), 'html')
        makedirclean(docWorkingDir)
        makedirclean(destDocPath)
        cmdStr = '""%sdoxygen.exe"" %s %s' % (doxygenPath, srcDocPath, doxyProjectFile)
        retVal = subprocess.call(cmdStr,shell=True)
        if retVal == 0:
                copydir(docWorkingDir, destDocPath)
        else:
                sys.exit("There were error(s) building documentation file " + doxyProjectFile)

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def GetP4Root():
  P4ROOT='p4root'
  if os.path.isfile(P4ROOT):
    removeitem(P4ROOT,True)
  retVal = subprocess.call('p4 client -o | findstr /b Root > P4ROOT',shell=True)
  p4root_file = open("p4root","r")
  rootdir = p4root_file.readline()
  rootdir=rootdir.strip("Root:")
  rootdir=rootdir.strip()
  #rootdir=rootdir.replace("\","//")
  p4root_file.close()
  removeitem(P4ROOT,True)
  return rootdir

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def PrintHeader(txt):
  print "\n"
  print "----------------...--------...----------------"
  print txt
  print "----------------...--------...----------------"

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def SanitizeDir(dir):
  ret = dir
  if ret[len(ret)-1] == "\\" or ret[len(ret)-1] == '/':
     return ret
  else:
    ret+="/"
  return ret

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def Run(cmdStr):
  retVal = subprocess.call(cmdStr,shell=True)
  if retVal != 0:
    sys.exit("There was an error running...")

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def PromptForDirectory(prompt):
  response = raw_input(prompt)
  dir = SanitizeDir(response)
  return dir

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def SimpleFileWrite(filepath, data):
  f = open(filepath, 'w')
  f.write(data)
  f.close()

#--------...--------...--------...--------...--------...--------...--------...--------

#--------...--------...--------...--------...--------...--------...--------...--------

def SimpleFileRead(filepath):
    ret = ""
    if os.path.isfile(filepath):
      f = open(filepath, 'r')
      ret = f.read()
      f.close()
    return ret

def SimpleFileReadLine(filepath):
    f = open(filepath, 'r')
    ret = f.readlines()
    f.close()
    return ret

def ValidateVersion(version):
    sect = version.split('.')
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
    version = ret_version.split('.')
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

def GetVersionFromFile(version_in):
    ret_version = SimpleFileRead(version_in)
    if ValidateVersion(ret_version) == False:
      return False
    return ret_version

def GetActiveParams(line, project, cfg):
    active = ""
    if line.find(project) != STRING_NOT_FOUND:
      active = line
      if active.find("BUILD:") != STRING_NOT_FOUND:
        cfg.build = True
      else:
        cfg.build = False
      if cfg.build == True:
        if active.find("RELEASE") != STRING_NOT_FOUND:
          cfg.profile = "ReleaseStatic"
        if active.find("DEBUG") != STRING_NOT_FOUND:
          cfg.profile = "DebugStatic"
        if active.find("PACKAGE") != STRING_NOT_FOUND:
          cfg.package = True
        if active.find("DOX") != STRING_NOT_FOUND:
          cfg.dox = True

def GetPathParams(line, project, cfg, deploy=False):
    active = ""
    if line.find(project) != -1:
      active = line
      path = active.strip(project)
      path = path.strip()
      path = path.strip(' ')
      path = path.strip('=')
      path = path.strip('\n')
      path = path.replace('\\', '/')
      if deploy==False:
        cfg.path = path
      else:
        cfg.deploy_path = path

def GetCfgParams(cfgfile, project, cfg):
    cfgfile = SimpleFileReadLine(cfgfile)
    ACTIVE_PARAM = project + "_ACTIVE"
    SOURCE_PARAM = project + "_SOURCE"
    SDK_PARAM = project + "_SDK"
    DEPLOY_PARAM = project + "_DEPLOY"
    for line in cfgfile:
      if line.find(ACTIVE_PARAM) != STRING_NOT_FOUND:
        GetActiveParams(line, ACTIVE_PARAM, cfg)
      if cfg.build == True:
        GetPathParams(line, SOURCE_PARAM, cfg)
      else:
        GetPathParams(line, SDK_PARAM, cfg)
      if cfg.package == True:
        GetPathParams(line, SDK_PARAM, cfg, True)

def GetPathFromInc(path):
  return path.strip("/include")
