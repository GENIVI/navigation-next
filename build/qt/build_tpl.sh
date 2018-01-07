#!/bin/sh
#Compile the tpslib.txt file into tesla.tps
BRANCH=$(basename $(pwd))
PYTHON=python
mkdir output
mkdir output/tpslib

##########################################################################################
#
# templates.txt
#
##########################################################################################


mv ./CoreSDK/templates* ./CoreSDK/templates.txt
cat tps_capabilities.txt CoreSDK/templates.txt > tpslib.txt

${PYTHON} ./util/update_version.py
${PYTHON} ./util/libcom.py -d . -o tesla.tpl tpslib_tmp.txt
rm tpslib.txt

#rem Create a version of the file with the hex id name
${PYTHON} ./util/rename_tpslib.py tesla.tpl

#rem Convert binary file to c header file
${PYTHON} ./util/tpslib_to_char_array.py tesla.tpl tpslib.h
rm tesla.tpl

#copy header file to ltk_common
mv -f tpslib.h ../../common/${BRANCH}/include/private/

#copy tpslib.txt to output
mv -f tpslib_tmp.txt ./output/tpslib/tpslib.txt


##########################################################################################
#
# admintemplates.txt
#
##########################################################################################

cat tps_qa_capabilities.txt ./CoreSDK/admintemplates.txt > qatpslib_tmp.txt

# compile qa templates
${PYTHON} ./util/libcom.py -d . -o admin.tpl qatpslib_tmp.txt

#rem Create a version of the file with the hex id name, writing to tpslib directory
${PYTHON} ./util/rename_tpslib.py admin.tpl


#rem Convert binary file to c header file
${PYTHON} ./util/tpslib_to_char_array.py admin.tpl qatpslib.h qatpslib
rm admin.tpl

#copy header file to ltk_common
mv -f qatpslib.h ../../common/${BRANCH}/include/private/
