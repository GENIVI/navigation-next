#!/usr/bin/env python
# -*- coding: utf-8 -*-
######################################################################
##   @file        codegenerator.py
##   @defgroup    build
##
##   Description: Script to generate codes using code generator.
##   (C) Copyright 2012 by TeleCommunications Systems, Inc.
##
##   The information contained herein is confidential, proprietary to
##   TeleCommunication Systems, Inc., and considered a trade secret as defined
##   in section 499C of the penal code of the State of California. Use of this
##   information by anyone other than authorized employees of TeleCommunication
##   Systems is granted only under a written non-disclosure agreement, expressly
##   prescribing the scope and manner of such use.
##
##
#####################################################################

import os
import stat
import sys
import subprocess
import array
import shutil
import glob

GENERATER_MARKER = "THIS IS GENERATED CODE. DO NOT MODIFY"

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

def MoveGeneratedIncludeFiles(arg, dirname, filenames):
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
            os.rename(path, os.path.join('../nbservices/include/generated/',name))

def MoveGeneratedSourceFiles(arg, dirname, filenames):
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
            os.rename(path, os.path.join('../nbservices/src/protocol/',name))

def GenerateCode():

    #generator_list contain dates of modified code generator files
    generator_list = []
    for root,dirs,files in os.walk('code_generator'):
      for one_file in files:
            generator_list.append(os.stat(os.path.join(root, one_file)).st_mtime)

    #If code generator files are newer than generated files we should generate new files
    fakeTarget = os.path.join('../nbservices/',
                              'src/protocol/NBProtocolMetadata.cpp')
    if ((not os.path.exists(fakeTarget)) or
        os.stat(fakeTarget).st_mtime < max(generator_list)):

        # Remove all generated files first. When metadata changed, some previously
        # generated files may be not needed anymore, remove all of them.
        print 'Cleaning local files ...'
        header_path = os.path.join('../nbservices/' 'include/generated/')
        source_path = os.path.join('../nbservices/' 'src/protocol/')

        os.path.walk(os.path.dirname(fakeTarget), RemoveGeneratedFiles, None)
        os.path.walk(header_path, RemoveGeneratedFiles, None)

        print 'Generating new files ...'
        retVal = subprocess.call(['java', '-jar', 'code_generator/tps.jar',
                                  'code_generator/res/CppConfiguration.xml'])
        if retVal:
            print('Error occurred while generating files ...\n')
            sys.exit(retVal)
        else:
            print('Succeeded in generating files ...\n')
        
        print('Moving new files ...')

        os.path.walk('../../nbservices/build/include/generated', MoveGeneratedIncludeFiles, None)
        os.path.walk('../../nbservices/build/src/protocol', MoveGeneratedSourceFiles, None)
        
        shutil.rmtree('../../nbservices')
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
        templates = ReadContents(os.path.join('tpslib', 'template.txt'))
    except :
        print sys.exc_info()
        templates = []

    try:
        generated = ReadContents(os.path.join('code_generator', 'gen', 'tpslib.txt'))
    except :
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
    except :
        print("Failed to save generated template, %s\n"%sys.exc_info()[1])

    # Copy admin template and capabilities template
    shutil.copy(os.path.join('tpslib',  'admintemplate.txt'),
                os.path.join(outDirectory, 'admintemplates.txt'))

if __name__ == '__main__':
    if len(sys.argv) != 3:
        msg="""Usage: %s version platform.

        In order to copy tps template, CodeGenerator needs to know CCC version
        and platform. For example:

        %s 4.2.1.36 iphone

        """
        print(msg%(sys.argv[0], sys.argv[0]))
        exit(1)

    GenerateCode()
    MergeTps("%s_%s_%s_internal"%("coreSDK", sys.argv[1], sys.argv[2]))
