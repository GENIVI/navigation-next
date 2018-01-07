#!/usr/bin/env python
# -*- coding: utf-8 -*-
##############################################################################
##  @file        iphone.py
##  @defgroup    Makefiles
##
##  Copyright (C) 2014 TeleCommunication Systems, Inc.
##
##  The information contained herein is confidential, proprietary to
##  TeleCommunication Systems, Inc., and considered a trade secret as defined
##  in section 499C of the penal code of the State of California. Use of this
##  information by anyone other than authorized employees of TeleCommunication
##  Systems is granted only under a written non-disclosure agreement, expressly
##  prescribing the scope and manner of such use.
##############################################################################

import os
import sys
import subprocess
import glob
import shutil

from CoreSDKCompiler import *


class iPhoneCompiler(CoreSDKCompiler):
    def __init__(self, opts):
        """
        """
        CoreSDKCompiler.__init__(self, opts)
        output_path = os.path.join(os.path.dirname(self._top_dir),
                                   ("CoreSDK_" + self._ccc_version + "_"
                                    + self._opts.platform + "_internal"))

        self.package_dir = os.path.join(output_path, "lib")
        self.lib_debug_dir = os.path.join(output_path, "lib-debug")

    def __configure__(self):
        """
        """
        if(self._opts.build_type == "debug"):
            self.configure_debug()
        elif(self._opts.build_type == "release"):
            self.configure_release()
        else:
            self.configure_debug()
            self.configure_release()

        return True

    def __compile__(self):
        """
        """
        if(os.path.exists("Debug")):
            os.chdir('Debug')
            print("Compiliing iphone simulator")
            subprocess.call("cmake --build . ", shell=True)
            os.chdir('..')
        if(os.path.exists("Release")):
            os.chdir('Release')
            print("Compiliing iphoneos")
            subprocess.call("cmake --build . ", shell=True)
        return True

    def __package__(self):
        """
        """
        if(not os.path.exists(self.package_dir)):
            os.makedirs(self.package_dir)

        if(not os.path.exists(self.lib_debug_dir)):
            os.makedirs(self.lib_debug_dir)

        print "Copy libraries ..."
        if(not self._opts.build_type):
            debug_libraries = self.get_libraries("debug")
            release_libraries = self.get_libraries("release")
            for dlib in debug_libraries:
                for rlib in release_libraries:
                    if(os.path.basename(rlib) == os.path.basename(dlib)):
                        self.package_library(os.path.basename(rlib), dlib,
                                             rlib)
                        self.strip_library(rlib)
                        self.copy_library(os.path.basename(rlib), rlib,
                                          "release")

        elif(self._opts.build_type == "debug"):
            debug_libraries = self.get_libraries("debug")
            for item in debug_libraries:
                self.copy_library(os.path.basename(item), item,
                                  self._opts.build_type)
            pass
        elif(self._opts.build_type == "release"):
            release_libraries = self.get_libraries("release")
            for item in release_libraries:
                self.strip_library(item)
                self.copy_library(os.path.basename(item), item,
                                  self._opts.build_type)
            pass
        else:
            return False

        #TODO: why not install static libraries?
        print "Copy headers ..."
        if(self._opts.build_type == "debug"):
            os.chdir('Debug')
        else:
            os.chdir('Release')
        cmd = "cmake --build . --target install"
        subprocess.call(cmd, shell=True)

        if(not self._opts.build_type):
            self.copy_NBM()
            self.copy_NCDB()
        return True

    def get_libraries(self, build_type):
        """
        """
        if(build_type == "debug"):
            os.chdir("Debug")
            outputdir = "Debug-iphonesimulator"
        elif(build_type == "release"):
            os.chdir("Release")
            outputdir = "MinSizeRel-iphoneos"
        else:
            return
        temp = glob.glob(os.path.join(os.getcwd(), "*"))
        commonts = []
        libraries = []
        for item in temp:
            if(os.path.isdir(item) and
               not item.find("CoreSDK") != -1 and
               not item.find("CMake") != -1 and
               not item.find("jni") != -1):
                commonts.append(item)

        for item in commonts:
            if(item.find("nbpal") != -1):
                libs = glob.glob(os.path.join(item, "iphone",
                                 outputdir, "*"))
            else:
                libs = glob.glob(os.path.join(item,
                                 outputdir, "*"))
            for lib in libs:
                libraries.append(lib)
        os.chdir("..")
        return libraries

    def copy_NBM(self):
        """
        """
        #print "path=%s" % os.path.join(self.package_dir, "libNBM.a")
        shutil.copy(os.path.join(self.package_dir, "libNBM.a"),
                    self.lib_debug_dir)

    def copy_NCDB(self):
        """
        """
        shutil.copy(os.path.join(self.package_dir, "libncdb.a"),
                    self.lib_debug_dir)

    def package_library(self, name, debugPath, releasePath):
        """
        pack library of debug version and release version.
        """
        cmd = "lipo -create %s %s -output %s/%s" % (debugPath, releasePath,
                                                    self.lib_debug_dir, name)
        print "package library cmd=%s" % cmd
        subprocess.call(cmd, shell=True)

    def strip_library(self, path):
        """
        Library of release version will be stripped.
        """
        cmd = "strip -S -x " + path + " > /dev/null 2>&1"
        subprocess.call(cmd, shell=True)

    def copy_library(self, name, path, build_type):
        """
        """
        if(build_type == "release"):
            cmd = "lipo -create %s -output %s/%s" % (path, self.package_dir,
                                                     name)
        else:
            cmd = "lipo -create %s -output %s/%s" % (path, self.lib_debug_dir,
                                                     name)
        subprocess.call(cmd, shell=True)

    def configure_debug(self):
        """
        """
        if(not os.path.exists("Debug")):
            os.mkdir('Debug')
        os.chdir('Debug')
        cmd = 'cmake -G "Xcode" -DPLATFORM=iphone -DSTATIC_LIB=ON \
               -DBUILD_PLATFORM_IPHONE=OFF \
               -DBUILD_PLATFORM_IPHONE_SIMULATOR=ON \
               -DCMAKE_CONFIGURATION_TYPES="Debug" -DDEBUG_CMAKE=ON ../../'
        #print("using command: %s\n" % (cmd))
        subprocess.call(cmd, shell=True)
        os.chdir('..')

    def configure_release(self):
        """
        """
        if(not os.path.exists("Release")):
            os.mkdir('Release')
        os.chdir('Release')
        cmd = 'cmake -G "Xcode" -DPLATFORM=iphone -DSTATIC_LIB=ON \
               -DBUILD_PLATFORM_IPHONE=ON \
               -DBUILD_PLATFORM_IPHONE_SIMULATOR=OFF \
               -DCMAKE_CONFIGURATION_TYPES="MinSizeRel" \
               -DDEBUG_CMAKE=ON ../../'
        #print("using command: %s\n" % (cmd))
        subprocess.call(cmd, shell=True)
        os.chdir('..')


def get_compiler(opts):
    """

    Arguments:
    - `opts`:
    """
    return iPhoneCompiler(opts)


def get_helpInformation():
    return None
