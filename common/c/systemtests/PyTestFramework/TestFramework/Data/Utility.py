#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os

import  traceback
def PDEBUG(fmt, *args):
    """
    Utility to show debug logs.
    """
    stack = traceback.extract_stack(None, 2)[0]
    try:
        msg = fmt%args
    except:
        msg = "Failed to format string.."
    finally:
        print("DEBUG - (%s:%d -- %s): %s"%(stack[0], stack[1], stack[2], msg))

def filename_sans_extention(path):
    """
    """
    path = os.path.basename(path)
    p = path.rfind('.')
    if p != -1:
        path = path[:p]
    return path
