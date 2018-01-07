#!/bin/sh

export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
export EGL_DRIVER=egl_dri2
aoss ./navkit_sample
