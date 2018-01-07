import os
import sys
import shutil
import zipfile
import platform

sys.path.append("./util")
import build_common

#----------------------------------------------------------------------
def generate_installer(zipFileName, targetPath):
    tmpfolder = os.path.join(targetPath, "temp")
    if os.path.isdir(tmpfolder):
        build_common.deletePath(tmpfolder)
    os.mkdir(tmpfolder)
    #shutil.copyfile(os.path.join(targetPath, zipFileName), os.path.join(tmpfolder, "ltk.zip"))
    if os.environ.get("BOARD") == "jetson":
        shutil.copyfile(os.path.join(build_common.curPath, "install_armhf.sh"), os.path.join(tmpfolder, "install.sh"))
    else:#linux x86
        shutil.copyfile(os.path.join(build_common.curPath, "install_x86.sh"), os.path.join(tmpfolder, "install.sh"))
    os.chdir(targetPath)
    os.system("chmod +x temp/install.sh")
    os.system("makeself temp setup.bin LTKInstaller ./install.sh")
    os.chdir(build_common.curPath)
    build_common.deletePath(tmpfolder)
    return True

#----------------------------------------------------------------------
def generate_document(versionPath):
    """"""
    os.chdir(build_common.curPath)
    if os.path.isdir(os.path.join(build_common.curPath, "output/docs")):
        os.rmdir(os.path.join(build_common.curPath, "output/docs"))
    os.mkdir(os.path.join(build_common.curPath, "output/docs"))
    os.system("doxygen mapkit3dConfiguration")
    os.system("doxygen navkitConfiguration")
    os.system("doxygen navigationuikitConfiguration")
    os.system("doxygen commonConfiguration")
    os.system("doxygen locationkitConfiguration")
    shutil.move(os.path.join(build_common.outputRoot,"docs"), versionPath)

    docName = "AutoReference Qt API Guide R1.0.pdf"
    shutil.copyfile(os.path.join(build_common.rootPath,"doc", "qt", docName), os.path.join(versionPath, "docs", docName))
    docName = "AutoReference Getting Started Guide R1.0.pdf"
    shutil.copyfile(os.path.join(build_common.rootPath,"doc", "qt", docName), os.path.join(versionPath, "docs", docName))
     
    return True

#----------------------------------------------------------------------
def zip_version_folder(versionPath):
    """"""
    folder = os.path.split(versionPath)[1]
    zipFileName = folder + ".zip"
    print "package--creating zip file: " + folder + ".zip"
    shutil.make_archive(folder, "zip", ".", folder)

    return zipFileName

#----------------------------------------------------------------------
def package(targetPath, versionPath):
    """"""
    generate_document(versionPath)
    zipFileName = zip_version_folder(versionPath)

    if os.path.isdir(os.path.join(build_common.curPath,targetPath)):
        build_common.deletePath(os.path.join(build_common.curPath,targetPath))
    os.mkdir(targetPath)

    folder = os.path.split(versionPath)[1]
    shutil.move(zipFileName,targetPath)
    #shutil.move(folder, targetPath)
    build_common.deletePath(folder)
    if os.path.isdir(os.path.join(build_common.curPath, "tpslib")):
        shutil.move(os.path.join(build_common.curPath, "tpslib"), targetPath)
    if os.path.isdir(os.path.join(build_common.curPath, "testapp")):
        shutil.move(os.path.join(build_common.curPath, "testapp"), os.path.join(targetPath, folder))

    if platform.system() == "Linux":
        generate_installer(zipFileName, targetPath)
    return True

if __name__=='__main__':
    build_common.curPath = os.path.abspath("../")
    build_common.branchName = "feature_140715_ltk_v1_5" #os.path.split(build_common.curPath)[1]
    build_common.rootPath = os.path.abspath("../../../")
    build_common.outputRoot = os.path.join(build_common.rootPath, "build", build_common.branchName, "output")
    build_common.ltkItems = ["common", "locationkit", "mapkit3d", "navigationkit", "navigationuikit"]
    build_common.sampleItems = ["mapkit3d_sample", "navkit_sample", "navigationuikit_sample"]

    targetPath = os.path.join(build_common.curPath, "1.0.0.63")
    versionPath = os.path.join(build_common.curPath, "ltk_1.0.0.63")
    package(targetPath, versionPath)
