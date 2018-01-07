import os, stat
import sys
import glob
import subprocess
import time
import shutil
lib_path = os.path.abspath('../../tools')
sys.path.append(lib_path)
import libcom
from tesla import tps
from tesla.tps import lib
from tesla.tps import io
from os import listdir
from os.path import isfile, join

BUILD_DIRECTORY = os.path.join(os.getcwd(), '..', '..', 'supportfiles', 'navfileset')
OUTPUT_DIRECTORY = sys.argv[1]

def chmod(filename):
    if os.path.isfile(filename):
        os.chmod(filename,0777)

def checkPath(p):
    if not os.path.exists(p):
        raise IOError,"Not found:%s" % p
    return p

#get the filesets_branch directory
def getFilesetsBranch():
    path = checkPath(os.path.join(BUILD_DIRECTORY, 'filesets_branch.txt'))
    txtFile = open( path, 'r' )
    branch = ""
    for line in txtFile:
        line = line.rstrip( '\n' )
        if ( line == "" ):
            continue
        else:
            branch = line
    txtFile.close()
    return branch

#get current language
def getCurrentLanguage():
    path = checkPath(os.path.join(BUILD_DIRECTORY, 'current_language.txt'))
    txtFile = open( path, 'r' )
    lang = ""
    for line in txtFile:
        line = line.rstrip( '\n' )
        if ( line == "" ):
            continue
        else:
            lang = line
    txtFile.close()
    return lang

#get current voice style
def getCurrentVoiceStyle():
    path = checkPath(os.path.join(BUILD_DIRECTORY, 'current_voice.txt'))
    txtFile = open( path, 'r' )
    voiceStyle = ""
    for line in txtFile:
        line = line.rstrip( '\n' )
        if ( line == "" ):
            continue
        else:
            voiceStyle = line
    txtFile.close()
    return voiceStyle

def getP4SourceDir():
    """Only parse current path of script to get your P4 root."""

    s = os.getcwd()
    pos = s.find('client')
    if (pos != -1):
        return s[:pos - 1]
    else:
        print 'error finding p4 root dir'
        sys.exit()

#path is here: //depot/client/resources/%BRANCH_NAME%/navigation/...
def getResourceDir():
    p4root = getP4SourceDir()
    filesets_branch = getFilesetsBranch()
    return os.path.join(p4root, 'client', 'resources', filesets_branch, 'navigation')

#generate compiled tpslib file
def generateTplFile(tpslib_path, srcfile, destfile):
    print "Generating tpl file .....\n"
    libcom.generateTpl(tpslib_path, destfile, os.path.join(tpslib_path, srcfile))

    print "Complete tplfile .....\n"

def allFiles(src_dir, extname):
    if (extname == '*'):
        a = os.path.join(src_dir, extname)
        onlyfiles = [ f for f in listdir(src_dir) if isfile(join(src_dir, f)) ]
    else:
        onlyfiles = [ f for f in listdir(src_dir) if (isfile(join(src_dir, f)) and f.endswith("." + extname)) ]

    return onlyfiles

def compileSexp2Tps(sexpfn, outfn, tpslib):
    tl=tps.lib.TemplateLibrary(tpslib)

    try:
        elt=tps.tpselt.fromsexp(open(sexpfn, 'rU').read())
        try:
            open(outfn, 'wb').write(tps.io.pack(tl, elt))
        finally:
            pass
    finally:
        pass

    if not os.path.isfile(outfn):
        sys.exit("Error: creating tps file from sexp file: "+sexpfn+"--> "+outfn)
    return 0

def sexp2tpsFS(sexpfn, tpsfn, tplfn):
    if not os.path.isfile(sexpfn):
        sys.exit("Error: missing sexp file: "+sexpfn)
    newDir=os.path.join(os.path.split(tpsfn)[0])
    if  len(newDir) and not os.path.isdir(newDir) :
        os.makedirs(newDir)
    compileSexp2Tps(sexpfn, tpsfn, tplfn)

# NS_Respack folder structure
# NK_<language_code>
#  * voices.tps
#  * TTS_basicaudio.tps
#  * directions.tps
#  * tpslib
# NK_<language_code>_audio
#  * <voice name>
#  * tpslib
#  * basicaudio.tps
#  * base audio files
# NK_common
#  * tpslib
#  * images.tps
#  * hovmapping.tps
#  * ttfmapping.tps
#  * routing_icons

def setupNkCommonFolder():
    nkCommonPath = os.path.join(OUTPUT_DIRECTORY, 'NK_common')
    if  len(nkCommonPath) and not os.path.isdir(nkCommonPath):
        os.makedirs(nkCommonPath)
    tplDict = os.path.join(nkCommonPath, 'tpslib')
    resourcePath = getResourceDir()
    dirConfigPath = os.path.join(resourcePath, 'guidanceconfig')
    generateTplFile(dirConfigPath, 'tpslib.txt', tplDict)
    files = ["images", "hovmapping", "ttfmapping"]
    for sexpFile in files:
        sexpFilePath = os.path.join(dirConfigPath, sexpFile + '.sexp')
        dstTpsFile = sexpFile + '.tps'
        dstTpsFilePath = os.path.join(nkCommonPath, dstTpsFile)
        print dstTpsFilePath + " is compiling"
        sexp2tpsFS(sexpFilePath, dstTpsFilePath, tplDict)
    #copy routing_icons once
    currentLanguage = getCurrentLanguage()
    voiceStyle = getCurrentVoiceStyle()
    nkRoutingIconsPath = os.path.join(nkCommonPath, 'routing_icons')
    if  len(nkRoutingIconsPath) and not os.path.isdir(nkRoutingIconsPath):
        os.makedirs(nkRoutingIconsPath)
        resourcePath = getResourceDir()
        iconsPath = os.path.join(resourcePath, 'routing_icons')

        print "routing icons are copying"
        copyfiles(iconsPath + os.sep + '*.png', nkRoutingIconsPath)

def setupNkLanguageCodeAudioFolder():
    #copy voices once
    currentLanguage = getCurrentLanguage()
    voiceStyle = getCurrentVoiceStyle()
    nkAudioPath = os.path.join(OUTPUT_DIRECTORY, 'NK_' + currentLanguage + '_audio', voiceStyle)
    if  len(nkAudioPath) and not os.path.isdir(nkAudioPath):
        os.makedirs(nkAudioPath)
        #remove '-'
        currentLanguage = currentLanguage.replace("-", "")
        voicesDirName = currentLanguage + '-' + voiceStyle
        resourcePath = getResourceDir()
        voicesPath = os.path.join(resourcePath, 'voices', 'aac', voicesDirName)
        tonesPath  = os.path.join(resourcePath, 'voices', 'aac', 'tones')

        tplDict = os.path.join(nkAudioPath, 'tpslib')
        dirConfigPath = os.path.join(resourcePath, 'voices')
        generateTplFile(dirConfigPath, 'tpslib.txt', tplDict)

        sexpFilePath = os.path.join(voicesPath, 'basicaudio.sexp')
        dstTpsFile = 'basicaudio.tps'
        dstTpsFilePath = os.path.join(nkAudioPath, dstTpsFile)
        print dstTpsFilePath + " is compiling"
        sexp2tpsFS(sexpFilePath, dstTpsFilePath, tplDict)

        print "base voice files are copying"
        copyfiles(voicesPath + os.sep + '*.aac', nkAudioPath)
        copyfiles(tonesPath + os.sep + '*.aac', nkAudioPath)

def setupNkLanguageCodeFolder():
    currentLanguage = getCurrentLanguage()
    voiceStyle = getCurrentVoiceStyle()
    nkAudioPath = os.path.join(OUTPUT_DIRECTORY, 'NK_' + currentLanguage)
    if  len(nkAudioPath) and not os.path.isdir(nkAudioPath):
        os.makedirs(nkAudioPath)
    tplDict = os.path.join(nkAudioPath, 'tpslib')
    resourcePath = getResourceDir()
    dirConfigPath = os.path.join(resourcePath, 'guidanceconfig')
    generateTplFile(dirConfigPath, 'tpslib.txt', tplDict)
    sexpFilePath = os.path.join(dirConfigPath, 'voices-' + currentLanguage + '.sexp')
    dstTpsFile = 'voices.tps'
    dstTpsFilePath = os.path.join(nkAudioPath, dstTpsFile)
    print dstTpsFilePath + " is compiling"
    sexp2tpsFS(sexpFilePath, dstTpsFilePath, tplDict)

    nkCommonPath = os.path.join(OUTPUT_DIRECTORY, 'NK_common')
    directionsSexpFileName = 'directions-' + currentLanguage + '.sexp'
    directionsSexpFilePath = os.path.join(resourcePath, 'directions', directionsSexpFileName)
    directionsDstTpsFilePath = os.path.join(nkAudioPath, "directions.tps")
    print dstTpsFilePath + " is compiling"
    commonTplDict = os.path.join(nkCommonPath, 'tpslib')
    sexp2tpsFS(directionsSexpFilePath, directionsDstTpsFilePath, commonTplDict)

    #remove '-'
    currentLanguage = currentLanguage.replace("-", "")
    voicesDirName = currentLanguage + '-' + voiceStyle
    voicesPath = os.path.join(resourcePath, 'voices', 'aac', voicesDirName)

    basicAudioTplDict = os.path.join(nkAudioPath, 'basicAudioTpslib')
    dirConfigPath = os.path.join(resourcePath, 'voices')
    generateTplFile(dirConfigPath, 'tpslib.txt', basicAudioTplDict)

    sexpFilePath = os.path.join(voicesPath, 'basicaudio.sexp')
    dstTpsFile = 'TTS_basicaudio.tps'
    dstTpsFilePath = os.path.join(nkAudioPath, dstTpsFile)
    print dstTpsFilePath + " is compiling"
    sexp2tpsFS(sexpFilePath, dstTpsFilePath, basicAudioTplDict)

def copy(srcFile,dstFile):
    try:
        shutil.copyfile(srcFile,dstFile)
        os.chmod(dstFile,stat.S_IWRITE | stat.S_IREAD)
    except OSError, (errno,strerror):
        print """Error copying %(path)s, %(error)s """ % {'path' : srcFile, 'error': strerror }

def copyfiles(fspec,dstDir):
    try:
        files = glob.glob(fspec)
        print files
        for fileName in files:
            print fileName + " is copying"
            fname = fileName[fileName.rindex(os.sep)+1:]
            chmod(dstDir + os.sep + fname)
            copy(fileName,dstDir + os.sep + fname)
    except OSError, (errno,strerror):
        print """Error copying files %(path)s, %(error)s """ % {'path' : fspec, 'error': strerror }


def run():
    setupNkCommonFolder()
    setupNkLanguageCodeAudioFolder()
    setupNkLanguageCodeFolder()
    print "COMPLETE"

run()