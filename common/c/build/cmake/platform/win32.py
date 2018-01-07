#!/usr/bin/env python

import os
import platform
import sys
import subprocess
import multiprocessing
from shutil import copy

from CoreSDKCompiler import *

class Win32_Compiler(CoreSDKCompiler):
    """
    """

    def __init__(self, opts):
        CoreSDKCompiler.__init__(self, opts);
        pass

    def __configure__(self):
        build_type = 'Debug'

        if self._opts.build_type is not None:
            print("BB: %s\n"%(self._opts.build_type))
            if self._opts.build_type.lower() == 'release':
                build_type = 'Release'

        CMAKE_COMMAND = 'cmake -G"Visual Studio 11 2012" \
        -DDEBUG_CMAKE=ON \
        -DPLATFORM=win32 \
        -DSTATIC_LIB=ON \
        -DCMAKE_BUILD_TYPE=%s ..'%(
            build_type)

        print("using command: %s\n"%( CMAKE_COMMAND))
        subprocess.call(CMAKE_COMMAND, shell=True)

        return True

    def __compile__(self):
        """
        """
        subprocess.call('cmake --build . ', shell=True)
        return True

    def __package__(self):
        """
        """
        #copying libraries to package directory
        cmd = 'cmake --build . --target install'
        print("calling %s\n"%cmd)
        subprocess.call(cmd, shell=True)

        return True


def get_compiler(opts):
    """

    Arguments:
    - `opts`:
    """
    return Win32_Compiler(opts)

def get_helpInformation():
    return None