import os
import glob
import sys
import shutil
import stat

sys.path.append("./util")
import update_version
import rename_tpslib
import libcom
import tpslib_to_char_array

#----------------------------------------------------------------------
def delFile(filePath):
    """check and delete specified file"""
    if os.path.isfile(filePath):
        os.chmod(filePath, stat.S_IWRITE)
        os.remove(filePath)

if __name__=='__main__':

    curPath = os.path.abspath(".")
    branchName = os.path.split(curPath)[1]

    #mkdir output and tpslib
    if not os.path.isdir(os.path.join(curPath,"output")):
        os.mkdir(os.path.join(curPath,"output"))
    if not os.path.isdir(os.path.join(curPath,"tpslib")):
        os.mkdir(os.path.join(curPath,"tpslib"))

    #mv ./CoreSDK/tempaltes* ./CoreSDK/tempaltes.txt
    file_names = glob.glob("./CoreSDK/templates_*.txt")
    if len(file_names) > 0:
        delFile("./CoreSDK/templates.txt")
        os.rename( file_names[0], "./CoreSDK/templates.txt")

    #cat tps_capabilities.txt CoreSDK/tempaltes.txt > tpslib.txt
    capabilitiesFile = open("tps_capabilities.txt")
    templatesFile = open("./CoreSDK/templates.txt")
    tpslibFile = open('tpslib.txt', "w+")

    shutil.copyfileobj(capabilitiesFile, tpslibFile)
    shutil.copyfileobj(templatesFile, tpslibFile)

    capabilitiesFile.close()
    templatesFile.close()
    tpslibFile.close()

    #${PYTHON} ./util/update_version.py
    #${PYTHON} ./util/libcom.py -d . -o tesla.tpl tpslib_tmp.txt
    update_version.main()
    libcom.generateTpl(".", "tesla.tpl", "tpslib_tmp.txt")

    #rm tpslib.txt
    delFile("tpslib.txt")

    #${PYTHON} ./util/rename_tpslib.py tesla.tpl
    args = ["rename_tpslib.py","tesla.tpl"]
    rename_tpslib.main(args)

    #${PYTHON} ./util/tpslib_to_char_array.py tesla.tpl tpslib.h
    args = ["tesla.tpl", "tpslib.h"]
    tpslib_to_char_array.main(args)

    #rm tesla.tpl
    delFile("tesla.tpl")

    #mv -f tpslib.h ../../common/${BRANCH}/include/private/
    commonRoot = os.path.abspath("../../")
    delFile(os.path.join(commonRoot, "common" ,branchName, "include", "private", "tpslib.h"))
    shutil.move("tpslib.h", os.path.join(commonRoot, "common" ,branchName, "include", "private"))

    #mv -f tpslib_tmp.txt ./tpslib/tpslib.txt
    delFile(os.path.join(curPath, "tpslib", "tpslib.txt"))
    shutil.move("tpslib_tmp.txt", os.path.join(curPath, "tpslib", "tpslib.txt"))

    #completed
    print "tpslib files has been generated, they are " + os.path.join(commonRoot, "common" ,branchName, "include", "private", "tpslib.h") + " and " + os.path.join(curPath, "output", "tpslib", "tpslib.txt")