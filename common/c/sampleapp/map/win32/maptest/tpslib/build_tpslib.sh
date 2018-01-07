#!/bin/sh
#Compile the tpslib.txt file into tesla.tps
PYTHON=python2.4
${PYTHON} ../tools/libcom.py -d . -o tesla.tpl tpslib.txt
#rem Create a version of the file with the hex id name
${PYTHON} ./rename_tpslib.py tesla.tpl
