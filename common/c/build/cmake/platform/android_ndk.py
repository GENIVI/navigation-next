#!/usr/bin/env python

import os
import platform
import sys
import subprocess
import multiprocessing
import shutil
import glob

from CoreSDKCompiler import *

class AndroidNDK_Compiler(CoreSDKCompiler):
    """
    """

    def __init__(self, opts):
        CoreSDKCompiler.__init__(self, opts);

        if opts.architecture:
            self.package_dir = os.path.join(os.path.dirname(self._top_dir),
                                            ("CoreSDK_" + self._ccc_version + "_"
                                            + self._opts.platform + "_"
                                            + opts.architecture + "_internal"),
                                            "lib")
        else:
            self.package_dir = os.path.join(os.path.dirname(self._top_dir),
                                            ("CoreSDK_" + self._ccc_version + "_"
                                            + self._opts.platform + "_internal"),
                                            "lib")

    def __configure__(self):
        # Checking ndk path.
        ndk_path = self._opts.kvp.get('ANDROID_NDK', None)
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

        # Checking ccache path
        ccache_path = self._opts.kvp.get('NDK_CCACHE', None)
        if ccache_path and os.access(ccache_path, os.F_OK):
            os.environ.update({'NDK_CCACHE':ccache_path})

        # Begin real configuration...
        SYSTEM       = platform.system().lower()
        MACHINE_TYPE = platform.machine().lower()
        MAKE_CMD     = "make"
        NDK_PATH     = os.environ.get("ANDROID_NDK")

        if NDK_PATH is None:
            print("You need to set ANDROID_NDK in environmental variable!\n")
            return False

        if SYSTEM == 'windows':
            # Maybe we need to add machine type into MAKE_PATH..
            CMAKE_GENERATE = '-G "MinGW Makefiles"'
            MAKE_CMD = "make.exe"
        else:
            CMAKE_GENERATE = ''

        MAKE_PATH = os.path.join(NDK_PATH, os.path.sep.join(["prebuilt", SYSTEM,
                                                             "bin", MAKE_CMD]))
        if not os.access(MAKE_PATH, os.F_OK):
            MAKE_PATH = os.path.join(NDK_PATH, os.path.sep.join(
                ["prebuilt", SYSTEM + "-" + MACHINE_TYPE, "bin", MAKE_CMD]))
            if not os.access(MAKE_PATH, os.F_OK):
                print("Can't find command: make\n", MAKE_PATH)
                return False

        build_type = 'Debug'

        if self._opts.build_type is not None:
            print("BB: %s\n"%(self._opts.build_type))
            if self._opts.build_type.lower() == 'release':
                build_type = 'Release'

        CMAKE_COMMAND = 'cmake %s \
        -DANDROID_STL="stlport_shared" \
        -DANDROID_NATIVE_API_LEVEL=9 \
        -DDEBUG_CMAKE=ON \
        -DPLATFORM=android_ndk \
        -DCMAKE_TOOLCHAIN_FILE=%s \
        -DCMAKE_BUILD_TYPE=%s \
        -DCMAKE_MAKE_PROGRAM=%s ..'%(
            CMAKE_GENERATE,
            os.path.join(sys.path[0], "contrib", "android.toolchain.cmake"),
            build_type,
            MAKE_PATH)

        architecture = ""
        if self._opts.architecture:
            architecture = self._opts.architecture
            if self._opts.architecture == 'arm':
                architecture = 'armeabi-v7a'
            CMAKE_COMMAND = '%s -DANDROID_ABI=%s -DARCHITECTURE=%s'%(
                            CMAKE_COMMAND,
                            architecture,
                            self._opts.architecture)

        print("using command: %s\n"%( CMAKE_COMMAND))

        subprocess.call(CMAKE_COMMAND, shell=True)

        return True

    def __compile__(self):
        """
        """
        try:
            number = multiprocessing.cpu_count() + 1
        except :
            number = 2
        subprocess.call('cmake --build . -- "-j%d"'%(number), shell=True)
        return True

    def __package__(self):
        """
        """
        #striping all libraries
        machineDirName=""
        for _, dirnames, _ in os.walk(os.path.join("..", "libs")):
            for dirname in dirnames:
                machineDirName = dirname
                cmd = "strip -s -x " + os.path.join("..", "libs", dirname, "*")
                subprocess.call(cmd, shell=True)

        #copying libraries to package directory
        cmd = 'cmake --build . --target install'
        print("calling %s\n"%cmd)
        subprocess.call(cmd, shell=True)

        #copying stlport lib
        shutil.copy(os.path.join("..", "libs", machineDirName, "libstlport_shared.so"), self.package_dir)
        
        #moving all so to target/architecture path
        targetDir = os.path.join(self.package_dir, self._opts.build_type.lower(), machineDirName)
        if os.path.isdir(targetDir):
            shutil.rmtree(targetDir,True)
        os.makedirs(targetDir)
        libFiles = glob.glob(os.path.join(self.package_dir, "*.so"))
        for file in libFiles:
            shutil.move(file, targetDir)

        return True


def get_compiler(opts):
    """

    Arguments:
    - `opts`:
    """
    return AndroidNDK_Compiler(opts)

def get_helpInformation():
    return ["ANDROID_NDK ==> the path of android ndk",
             "NDK_CCACHE  ==> CCache support. To speed up large rebuilds, ",
             "                simply define the NDK_CCACHE environment variable to 'ccache'",
             "                (or the path to your ccache binary), as in: export NDK_CCACHE=ccache"]
