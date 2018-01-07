#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os
import subprocess
import tempfile
import select

# @fixme: for windows platform, we may need to add ".exe"
def which(program):
    def is_exe(fpath):
        return os.access(fpath, os.X_OK) and os.path.isfile(fpath)

    fpath, fname = os.path.split(program)
    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ['PATH'].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)
            if is_exe(exe_file):
                return exe_file
        return None


class Executor(object):
    def __init__(self, *args, **kargs):
        """Init this compiler.
        """
        self._logger      = kargs.get('Logger')
        self._process     = None
        self._root_dir = os.path.abspath('.')
        self._gen_dir     = os.path.join(self._root_dir, 'TestFramework','Generated')
        self._case_dir    = os.path.join(self._root_dir, 'TestFramework','TestCases')
        self._src_path    = os.path.join(self._gen_dir, 'main.c')
        self._header_path = os.path.join(self._gen_dir, 'Generated.h')
        self._cmake_path  = os.path.join(self._gen_dir, 'CMakeLists.txt')
        self._target_name = 'SystemTest'
        self._target_path = os.path.join(self._gen_dir, 'SystemTest')
        self._cunit_dir   = os.path.join(self._root_dir, '..', '..', '..',
                                         'thirdparty',
                                         'feature_140523_hybrid_phase2',
                                         'CUnit')
        self._template_dir = os.path.join(self._root_dir, 'TestFramework','Templates')
        pass

    def stop(self):
        """
        """
        if self._process:
            self._process.kill()

        pass

    def generate_codes(self, cases):
        """
        We will generate two files automatically:
        1. Main.c/cpp:
           Head and tail of this file may be platform specific, but the main
           logic should be the same.
        2. CMakeLists which will be used to compile and run test app.
        """
        self._cases = cases
        # @todo: remove hard-coded path!
        main_content  = self.__get_main_head();
        cmake_content = self.__get_cmake_head()
        header_content = []

        for case in self._cases:
            name = case.GetName()

            main_content.append(
                '    CreateTestSuite("default", &%s_SuiteSetup, '
                '&%s_SuiteCleanup, %s_AddTests);'%(
                    name,name,name))

            header_content.append('#include "%s.h"'%name)

            cmake_content.append('file(GLOB CSRC "%s/%s*.c" "%s/%s.cpp")'%(
                self._case_dir, name, self._case_dir, name))
            cmake_content.append('list(APPEND SRC ${CSRC})')

        cmake_content.append('add_executable(SystemTest ${SRC})')
        cmake_content.extend(self.__get_cmake_tail())
        main_content.extend(self.__get_main_tail())

        open(self._src_path, 'w').write('\n'.join(main_content))
        open(self._cmake_path, 'w').write('\n'.join(cmake_content))
        open(self._header_path, 'w').write('\n'.join(header_content))

        return 0

    def compile(self):
        """Compile into libraries.
        """
        self._logger.WriteLog('Compiling...')
        pwd = os.getcwd()
        os.chdir(self._gen_dir)
        r = self._run_cmd_internal(self.__get_configure_cmd())
        if r != 0:
            self._logger.WriteLog("Failed to configure project!")
            return r

        r = self._run_cmd_internal(self.__get_compile_cmd())
        if r != 0:
            self._logger.WriteLog("Failed to compile project!")
            return r

        os.chdir(pwd)
        self._process = None
        return r

    def run(self):
        """
        """
        pwd = os.getcwd()
        os.chdir(self._gen_dir)
        self._run_cmd_internal(self.__get_exe_cmd())
        os.chdir(pwd)
        try:
            os.remove(self._src_path)
            os.remove(self._target_path)
        except :
            pass
        return True
        pass

    def __get_configure_cmd(self):
        self._cmake = which('cmake')
        if self._cmake is None:
            # raise exception ? or return None?
            pass
        return [self._cmake, 'CMakeLists.txt']

    def __get_compile_cmd(self):
        return [self._cmake, '--build', '.']

    def __get_exe_cmd(self):
        """
        """
        return ['./SystemTest']
        return True

    def _run_cmd_internal(self, cmd):
        """
        """

        try:
            self._process = subprocess.Popen(cmd,
                                             stdout=subprocess.PIPE,
                                             stderr=subprocess.STDOUT)
        except Exception as e:
            self._logger.WriteLog('Faild to execute command: %s'%''.join(cmd))
            return -1

        while True:
            reads = [self._process.stdout.fileno()]
            ret = select.select(reads, [], [])

            for fd in ret[0]:
                if fd == self._process.stdout.fileno():
                    self._logger.WriteLog(''.join(self._process.stdout.readlines()))
            if self._process.poll() != None:
                print "Exit..."
                break
        r = self._process.returncode
        self._process = None
        return r

    def __get_main_head(self):
        """Return prefix/header for main.c, children may need to overwrite
        this function to do platform-specified initialization.

        Please make sure 'generated.h' is included..
        It returns a list of strings.
        """
        try:
            contents = open(os.path.join(self._template_dir, 'main_head.txt'), 'r').readlines()
        except Exception as e:
            contents = []
        return contents

    def __get_main_tail(self):
        """Return suffix/tail for main.c, children may need to overwrite
        this function to do platform-specified clean up.

        It returns a list of strings.
        """
        try:
            contents = open(os.path.join(self._template_dir, 'main_tail.txt'), 'r').readlines()
        except Exception as e:
            print e
            contents = []

        return contents


    def __get_cmake_head(self):
        """Return prefix/header for main.c, children may need to overwrite
        this function to do platform-specified initialization.

        Please make sure 'generated.h' is included..
        It returns a list of strings.
        """
        contents = []
        contents.append('project (%s)'%self._target_name)
        contents.append('cmake_minimum_required(VERSION 2.8)')
        contents.append('file(GLOB SRC "*.c")')
        contents.append('include_directories(%s)'%self._case_dir)

        # Cunit codes..
        contents.append('include_directories(%s)'%
                        os.path.join(self._cunit_dir, 'include'))
        contents.append('file(GLOB CU_FRAMEWORK "%s/Sources/Framework/*")'%
                        self._cunit_dir)
        contents.append('file(GLOB CU_BASIC "%s/Sources/Basic/*")'%
                        self._cunit_dir)
        contents.append('file(GLOB CU_AUTOMATED "%s/Sources/Automated/*")'%
                        self._cunit_dir)
        contents.append('list(APPEND SRC ${CU_FRAMEWORK} ${CU_BASIC} ${CU_AUTOMATED})')

        return contents

    def __get_cmake_tail(self):
        """Return suffix/tail for main.c, children may need to overwrite
        this function to do platform-specified clean up.

        It returns a list of strings.
        """
        return []



    def clean(self):
        """Do clean ups here.
        """
        pass

def get_executor(*args, **kargs):
    return Executor(Logger=kargs.get('Logger'))
