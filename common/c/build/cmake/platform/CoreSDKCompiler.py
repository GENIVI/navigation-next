#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess

def SimpleFileRead(filepath):
    ret = ""
    if os.path.isfile(filepath):
        f = open(filepath, 'r')
        ret = f.read()
        f.close()
    return ret

class CoreSDKCompiler(object):
    """Default Compiler for CoreSDk.
    """

    def __init__(self, opts):
        """Init this compiler.
        """
        self._opts        = opts
        self._top_dir     = os.path.abspath('')
        self._build_dir   = 'outputs_%s'%opts.platform
        self._branch      = os.path.basename(os.path.dirname(self._top_dir))
        self._ccc_root    = os.path.sep.join(self._top_dir.split(os.path.sep)[:-3])
        version_file      = "version_" + self._opts.platform + ".txt"
        version_path      = os.path.join(self._ccc_root, "build", self._branch, version_file)
        self._ccc_version = SimpleFileRead(version_path).strip()

        if not os.path.exists(self._build_dir):
            os.mkdir(self._build_dir)

        pass

    def configure(self):
        """Configure this project.
        """
        print("\nConfiguring...\n")

        os.chdir(self._build_dir)
        ret = self.__configure__()
        os.chdir(self._top_dir)
        return ret

    def compile(self):
        """Compile into libraries.
        """
        print("\nCompiling...\n")
        os.chdir(self._build_dir)
        ret = self.__compile__()
        os.chdir(self._top_dir)
        return ret

    def package(self):
        """Package for publish.
        """
        print("\nPackaging libraries..\n")
        os.chdir(self._build_dir)
        ret = self.__package__()
        os.chdir(self._top_dir)
        return ret

    def __configure__(self):
        """Overloaded by children to do configuration.
        """
        build_type = 'Debug'

        if self._opts.build_type is not None:
            print("BB: %s\n"%(self._opts.build_type))
            if self._opts.build_type.lower() == 'release':
                build_type = 'Release'

        CMAKE_COMMAND = 'cmake  \
        -DDEBUG_CMAKE=ON \
        -DPLATFORM= %s\
        -DSTATIC_LIB=ON \
        -DCMAKE_BUILD_TYPE=%s ..'%(self._opts.platform,
            build_type)

        print("using command: %s\n"%( CMAKE_COMMAND))
        subprocess.call(CMAKE_COMMAND, shell=True)

        return True


    def __compile__(self):
        """Overloaded by children to start compile.
        """
        subprocess.call('cmake --build . ', shell=True)
        return True


    def __package__(self):
        """Overloaded by children to do package.
        """
        return True

def get_compiler(opts):
    """
    """
    return CoreSDKCompiler(opts)
