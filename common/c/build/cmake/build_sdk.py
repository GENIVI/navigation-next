#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import array
import glob
import os
import platform
import shutil
import stat
import subprocess
import sys

GENERATER_MARKER = "THIS IS GENERATED CODE. DO NOT MODIFY"
TOP              = os.path.abspath('')
BRANCH           = os.path.basename(os.path.dirname(TOP))
CCC_ROOT_DIR     = os.path.sep.join(TOP.split(os.path.sep)[:-3])
BUILD_DIR        = os.path.sep.join(TOP.split(os.path.sep)[:-2])
OPTS             = None
CCC_MODE         = "onboard"

desc="""
This will be a cross-platform build script for CoreSDK!
"""
sys.path.append('platform')

def RemoveGeneratedFiles(arg, dirname, filenames):
    """
    Remove auto generated files.
    Arguments:
    - `arg`: Argument passed to os.path.walk.
    - `dirname`: current directory name.
    - `filenames`: file names under dirname.
    """
    for name in filenames:
        path = os.path.join(dirname, name)
        if os.path.isfile(path) and open(path).read().find(GENERATER_MARKER) != -1:
            os.remove(path)


def GenerateCode():
    generator_dir = os.path.join(CCC_ROOT_DIR, 'build', BRANCH,
                                 'code_generator')
    nbservices_dir = os.path.join(CCC_ROOT_DIR, 'nbservices', BRANCH)

    #generator_list contain dates of modified code generator files
    generator_list = []
    for root,dirs,files in os.walk(generator_dir):
        for one_file in files:
            generator_list.append(os.stat(os.path.join(root, one_file)).st_mtime)

    #If code generator files are newer than generated files we should generate new files
    fakeTarget = os.path.join(nbservices_dir, 'src',
                              'protocol', 'NBProtocolMetadata.cpp')

    if ((not os.path.exists(fakeTarget)) or
        os.stat(fakeTarget).st_mtime < max(generator_list)):

        # Remove all generated files first. When metadata changed, some previously
        # generated files may be not needed anymore, remove all of them.
        print 'Cleaning local files ...'
        header_path = os.path.join(nbservices_dir, 'include', 'generated')
        source_path = os.path.join(nbservices_dir, 'src', 'protocol')

        os.path.walk(os.path.dirname(fakeTarget), RemoveGeneratedFiles, None)
        os.path.walk(header_path, RemoveGeneratedFiles, None)

        print 'Generating new files ...'
        savedDir = os.getcwd()
        os.chdir('..')
        retVal = subprocess.call(['java', '-jar',
                                  os.path.join(generator_dir, 'tps.jar'),
                                  os.path.join(generator_dir, 'res',
                                               'CppConfiguration.xml')])
        os.chdir(savedDir)
        if retVal:
            print('Error occurred while generating files ...\n')
            sys.exit(retVal)
        else:
            print('Succeeded in generating files ...\n')
    else:
        print('No need to generate codes ...\n')

def ReadContents(path):
    """
    Get contents of file specified by path.
    Arguments:
    - `path`: file to be read.
    """
    try:
        contents = open(path, 'r').readlines()
    except :
        contents = []
    return contents;

def MergeTps(outDirectory):
    """Merge generated tps file with template

    Arguments:
    - `outDirectory` : outDirectory of this SDK.
    """
    try:
        templates = ReadContents(os.path.join(BUILD_DIR, BRANCH, 'tpslib', 'template.txt'))
    except :
        print sys.exc_info()
        templates = []

    try:
        generated = ReadContents(os.path.join(BUILD_DIR, BRANCH, 'code_generator', 'gen', 'tpslib.txt'))
    except:
        print sys.exc_info()
        generated = []

    templates.extend(generated)
    if not templates:
        print("Template is empty!\n")
        sys.exit(1)

    if not os.access(outDirectory, os.F_OK):
        os.makedirs(outDirectory)

    outFile = os.path.join(outDirectory, "templates.txt")
    try:
        open(outFile, 'w').write("".join(templates))
    except:
        print("Failed to save generated template, %s\n"%sys.exc_info()[1])

    # Copy admin template and capabilities template
    if(os.path.exists(os.path.join(outDirectory, 'admintemplates.txt'))):
        os.remove(os.path.join(outDirectory, 'admintemplates.txt'))

    shutil.copy(os.path.join(BUILD_DIR, BRANCH, 'tpslib', 'admintemplate.txt'),
                os.path.join(outDirectory, 'admintemplates.txt'))

def makeVersionInfo(component):
    cmd = ["python", os.path.join(component, "build", "version.py"),
           component, OPTS.platform]
    print "creating VERSIONINFO.H for " + component
    rc = subprocess.call(cmd)
    print "create versioninfo = %d" % rc

    if rc != 0:
        sys.exit()

def makeVersionInfoNbgm():
    nbgmmainPath = os.path.join('nbgm', 'nbgmmain')
    cmd = ["python", os.path.join(nbgmmainPath, "build", "version.py"),
           nbgmmainPath, OPTS.platform]
    print "creating VERSIONINFO.H for nbgmmain"
    rc = subprocess.call(cmd)
    print "create versioninfo = %d" % rc

    if rc != 0:
        sys.exit()

    pass

def CreateSymbolicLink(src, dst):
    import ctypes
    flags = 1 if src is not None and os.path.isdir(src) else 0
    if not ctypes.windll.kernel32.CreateSymbolicLinkA(dst, src, flags):
        raise OSError

if platform.system() == 'Windows':
    os.symlink = CreateSymbolicLink

def CreateLink(src, dst):
    """
    Create link between source and destination
    """
    if os.access(dst, os.F_OK):
        try:
            os.rmdir(dst)
        except:
            os.unlink(dst)
    os.symlink(src, dst)

class CoreSDKArgParser(argparse.ArgumentParser):
    def parse_args(self, args=None, namespace=None):
        #find the platform argument value
        self.platform=""
        foundPlatform = False
        for arg in args:
            if (arg == "-p"):
                foundPlatform = True
                continue
            if foundPlatform and not arg[0]=='-':
                self.platform = arg

        return argparse.ArgumentParser.parse_args(self, args, namespace)

    def format_help(self):
        print argparse.ArgumentParser.format_help(self)

        #try to print platform special information
        if self.platform:
            try:
                texts = __import__(self.platform, globals(), locals(), ['*']).get_helpInformation()
                self.print_FormattedText(self.platform, texts)
            except:
                None
        else:
            platformFiles = glob.glob('platform/*')
            platforms = [os.path.basename(os.path.splitext(p)[0]) for p in platformFiles if os.path.splitext(p)[1] == ".py"]
            platforms.remove("CoreSDKCompiler")
            for p in platforms:
                try:
                    texts = __import__(p, globals(), locals(), ['*']).get_helpInformation()
                    self.print_FormattedText(p, texts)
                except:
                    None

    def print_FormattedText(self, platform, texts):
        if len(texts) > 0:
            print "      " + platform + ":"
            for text in texts:
                print "          " + text
            print

if __name__ == '__main__':
    #TODO:
    # 1. argument parser using argparse.
    # 2. Load different configure functions based on platform.
    # 3. Call configure function to configure
    # 4. Call cmake to generate project(VS, XCode or unix make file)
    # 5. Compile
    # 6. Package.
    parser = CoreSDKArgParser(description=desc,
                              formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('-p', '--platform',
                        help='specify platform')

    parser.add_argument('-t', '--build-type',
                        help='specify build type: debug, release, or default.'
                        ' The "default" type will be platform-dependent.')

    parser.add_argument('-a', "--architecture",
                        help='specify build architecture, for example x86, arm.'
                        ' These architectures values are platform related.')

    parser.add_argument('-g', '--generate-only',
                        action='store_true',
                        help='Generate project files only')

    parser.add_argument('-D', '--define', action='append',
                        help='Provide extra information by define Key-Value'
                        ' pair. A Key-Value pair looks like: "key=values".'
                        ' For example, you can define ANDROID_NDK which is'
                        ' used by android like this:'
                        '\t -DANDROID_NDK="/opt/android-ndk-r8d".'
                        ' You can define multiple k-v pairs here, but if'
                        ' there are white spaces in key or value, you need to'
                        ' wrapper it using "".'
                        ' Any details please refer the platform special information as below.')

    if len(sys.argv) == 1:
        print("Missing arguments, showing help..\n")
        parser.print_help()
        sys.exit(1)

    OPTS = parser.parse_args(sys.argv[1:])
    OPTS.kvp = {}
    if OPTS.define:
        for item in OPTS.define:
            # Hope there is only one '=' in this pair...
            tmp = item.split('=')
            if len(tmp) != 2:
                print("Invalid definition: %s\n"%(item))
                continue
            OPTS.kvp[tmp[0]] = tmp[1]

    # print("%s"%OPTS)

    if not OPTS.platform:
        # @todo: should we provide default platform?
        OPTS.platform = 'android_ndk'
        print("P: %s"%OPTS.platform)

    # @todo: move platform-dependent checking into separate files.
    if OPTS.platform == 'android_ndk':
        ndk_path = OPTS.kvp.get('ANDROID_NDK', None)
        if ndk_path: # use NDK specified by user.
            if os.access(ndk_path, os.F_OK):
                os.environ.update({'ANDROID_NDK':ndk_path})
            else:
                print("\nNDK path(%s) is not accessible...\n"%(ndk_path))
                sys.exit(1)
        else:
            ndk_path = os.environ.get("ANDROID_NDK")
            if (not ndk_path) or (not os.access(ndk_path, os.F_OK)):
                print("\nCan't find proper ANDROID_NDK.\n"
                      "You can either set this to environment variable, "
                      "or pass it in using -D option.\n")
                sys.exit(1)
        print "Android NDK: %s"%os.environ.get('ANDROID_NDK')

    # Link component folder into current folder.
    components = glob.glob(os.path.join(CCC_ROOT_DIR, '*'))
    components.remove(os.path.join(CCC_ROOT_DIR, 'build'))

    for component in components:
        CreateLink(os.path.join(component, BRANCH),
                           os.path.basename(component))

    CreateLink(os.path.join('..', 'code_generator'), 'code_generator');

    # Call code generator to generate codes..
    GenerateCode()

    #-----------------------------------------------------------------------------------------------------------------------------------------#
    # Set CCC mode - hybrid or offboard
    #-----------------------------------------------------------------------------------------------------------------------------------------#
    use_onboard_file = open(os.path.join(CCC_ROOT_DIR, "nbservices", BRANCH, "include", "private", "useonboard.h"),"w")
    if CCC_MODE == "onboard":
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n//#define OFFBOARD_SERVICE_ONLY\n#endif')
    else:
        use_onboard_file.write('#ifndef USE_ONBOARD_H\n#define USE_ONBOARD_H\n#define OFFBOARD_SERVICE_ONLY\n#endif')
    use_onboard_file.close()

    # Prepare version files for components
    makeVersionInfo("nbpal")
    makeVersionInfo("abpal")
    makeVersionInfo("abservices")
    makeVersionInfo("coreservices")
    makeVersionInfo("nbservices")
    makeVersionInfo("nbui")
    makeVersionInfo("common")
    makeVersionInfoNbgm()

    try:
        compiler = __import__(OPTS.platform, globals(), locals(), ['*']).get_compiler(OPTS)
    except:
        print("Using the default compiler, because failed to load platform utilities..\n")
        compiler = __import__("CoreSDKCompiler", globals(), locals(), ['*']).get_compiler(OPTS)

    ret = compiler.configure()

    if not ret:
        print("Failed to configure project for platform: %s\n"%
              (OPTS.platform))
        sys.exit(1)

    if OPTS.generate_only:
        sys.exit(0)

    ret = compiler.compile()
    if not ret:
        print("Failed to compile project for platform: %s\n"%
              (OPTS.platform))
        sys.exit(1)

    ret = compiler.package()
    if not ret:
        print("Failed to package libraries for platform: %s\n"%
              (OPTS.platform))
        sys.exit(1)

    version_file = "version_" + OPTS.platform + ".txt"
    version_path = os.path.join(BUILD_DIR, BRANCH, version_file)
    ccc_version = __import__("CoreSDKCompiler", globals(), locals(),
                             ['*']).SimpleFileRead(version_path).strip()

    if OPTS.architecture:
        outDirection = os.path.join(BUILD_DIR, BRANCH,
                                    ("CoreSDK_" + ccc_version + "_" + OPTS.platform
                                    + "_" + OPTS.architecture
                                    + "_internal"))
    else:
        outDirection = os.path.join(BUILD_DIR, BRANCH,
                                    ("CoreSDK_" + ccc_version + "_" + OPTS.platform
                                    + "_internal"))
    MergeTps(outDirection)

    # This comments could not be removed,
    # because parabuild need it to match the result.
    print "Build Complete"
