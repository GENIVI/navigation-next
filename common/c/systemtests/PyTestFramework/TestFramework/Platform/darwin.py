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

from Executor import *


class OSXExecutor(Executor):
    def __init__(self, *args, **kargs):
        """
        """
        super(OSXExecutor, self).__init__(self, args, kargs)



def get_executor(*args, **kargs):
    """

    Arguments:
    - `opts`:
    """
    return OSXExecutor(args, kargs)


def get_helpInformation():
    return None
