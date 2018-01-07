#!/bin/bash

cd `dirname $0`
file_path=`pwd`
cd -
python2.4 "${file_path}/build_navkit_package.py" "${file_path}/output"