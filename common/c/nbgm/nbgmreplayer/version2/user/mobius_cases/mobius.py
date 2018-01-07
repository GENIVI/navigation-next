from userscript import *
from nbgminireader import *
from loaderwin32 import *
import os,logging,shutil

CURRENT_WORK_PATH    = os.getcwd()
CURRENT_PATH         = os.path.split(os.path.realpath(__file__))[0]
NBGM_TEST_INI_FILE   = os.path.join(CURRENT_PATH, 'nbgmtest.ini')
NBM_TILE_EXE_FILE    = os.path.join(CURRENT_PATH, 'TestNBMTile.exe')
OUTPUT_PATH1         = os.path.join(CURRENT_WORK_PATH, 'output')
OUTPUT_PATH2         = os.path.join(CURRENT_WORK_PATH, 'temp')
NBGM_COMMON_MATERIAL = os.path.join(CURRENT_PATH, 'common_material_day.nbm')
BUG_TARCK_FILE       = os.path.join(CURRENT_PATH, 'bug_track_chunk.txt')
NCDB_CONFIG          = 'C:\\ncdb_config'

# wait for the exectuion of TestNBMTile.exe
WAIT_TIME_SCEONDS_FOR_NBM_TILE_EXE = 20


def FindAllFilesInFolder(folder, extend):
    file_list = []
    for root, dis, files in os.walk(folder):
        for file in files:
            if os.path.splitext(file)[1] == extend:
                file_list.append(os.path.join(root, file))
    return file_list

def DeleteFilesInFolder(folder, extend, key = ''):
    for root, dis, files in os.walk(folder):
        for file in files:
            absPath = os.path.join(root, file)
            if os.path.splitext(file)[1] == extend:
                if key:
                    if file.find(key) != -1:
                        os.remove(absPath)
                else:
                    os.remove(absPath)

def DeleteFilesInFolder2(folder, extend):
    for root, dis, files in os.walk(folder):
        for file in files:
            oldPath = os.path.join(root, file)
            if oldPath.find(extend) != -1:
                os.remove(oldPath)
            else:
                if oldPath.find('15AREA.nbm') != -1:
                    os.rename(oldPath, oldPath.replace('15AREA.nbm', 'AREA.nbm'))
                if oldPath.find('15ROAD.nbm') != -1:
                    os.rename(oldPath, oldPath.replace('15ROAD.nbm', 'ROAD.nbm'))

def CopyAllNbmFilesFromFolder(srcFolder, desFolder):
    for root, dis, files in os.walk(srcFolder):
        for file in files:
            if os.path.splitext(file)[1] == '.nbm':
                shutil.copy(os.path.join(root, file), desFolder)

def GetLableLevel(rasLevel):
    if rasLevel <= 5:
        return 3
    elif rasLevel <= 10:
        return 7
    elif rasLevel <= 13:
        return 12
    elif rasLevel <= 15:
        return 14
    else:
        return -1

def CreateTrackFile(lat, lon):
    try:
        chunck_file = open(BUG_TARCK_FILE, 'w')
        chunck_file.write('%f %f' % (lat, lon))
    except Exception, e:
        print e
        logging.error('CreateTrackFile %s failed!' % BUG_TARCK_FILE)
        chunck_file.close()
        return False
    else:
        chunck_file.close()
        return True

def CreateNbmTile(lon, lat, level, nbmType, mapPath):
    commandline = ''
    if nbmType == DVR_DVA:
        commandline = '%s -m %s -d -c -o -ALL -p 1 -i  %s -ZL 15' %\
            (NBM_TILE_EXE_FILE, mapPath, BUG_TARCK_FILE)
    else:
        if nbmType == RASTER:
            commandline = '%s -m %s -d -c -o -RAST -p 1 -i  %s' %\
                (NBM_TILE_EXE_FILE, mapPath, BUG_TARCK_FILE)
        elif nbmType == LABEL:
            commandline = '%s -m %s -d -c -o -LBLT -p 1 -i  %s' %\
                (NBM_TILE_EXE_FILE, mapPath, BUG_TARCK_FILE)
        commandline += ' -ZL %d' % level

    loader = LoaderWin32()
    if loader.load(commandline):
        nbas_quit_if_aborted()
        loader.wait(WAIT_TIME_SCEONDS_FOR_NBM_TILE_EXE)
        if loader.waitResult == WAIT_TIMEOUT:
            logging.error('NBM tile creation time out %d !!!' % WAIT_TIME_SCEONDS_FOR_NBM_TILE_EXE)
            return False
    else:
        logging.error('Cannot execute the command line %s' % commandline)
        return False
    DeleteFilesInFolder(CURRENT_WORK_PATH, '.LOG', 'NCDB_')
    return True

def InputCheck():
    if not os.path.isfile(NBGM_TEST_INI_FILE):
        logging.critical('Cannot find the ini file %s !' % NBGM_TEST_INI_FILE)
        return False
    if not os.path.isfile(NBM_TILE_EXE_FILE):
        logging.critical('Cannot find the exe file %s !' % NBM_TILE_EXE_FILE)
        return False
    if not os.path.isdir(NCDB_CONFIG):
        logging.critical('Cannot find the ncdb folder %s !' % NCDB_CONFIG)
        return False
    if not os.path.isfile(NBGM_COMMON_MATERIAL):
        logging.critical('Cannot find nbgm common material %s !' % NBGM_COMMON_MATERIAL)
        return False
    if not os.path.isdir(OUTPUT_PATH1):
        os.mkdir(OUTPUT_PATH1)
    if not os.path.isdir(OUTPUT_PATH2):
        os.mkdir(OUTPUT_PATH2)
    return True

# the entry point
def Run():
    if not InputCheck():
        return

    nbgm_ini_file = NBGMINIReader()
    if not nbgm_ini_file.read(NBGM_TEST_INI_FILE):
        logging.critical('Read %s failed !' % iniFilePath)
        return

    # load & use common materail
    nbas_load_common_material('day', NBGM_COMMON_MATERIAL)
    nbas_set_current_common_material('day')
    # set the sky wall far away
    nbas_set_horizon_distance(EXTREAM_FAR_DISTANCE)
    for region in nbgm_ini_file.regionList:
        if not region.isValid():
            continue
        input_path = os.path.abspath(region.inputPath)
        if not os.path.isdir(input_path):
            logging.error('Cannot find the input path %s !' % input_path)
            continue
        test_files = FindAllFilesInFolder(input_path, '.txt')
        for test_file_path in test_files:
            nbas_quit_if_aborted()
            path, name = os.path.split(test_file_path)
            title = os.path.splitext(name)[0]
            test_folder = os.path.join(path, title)
            if not os.path.isdir(test_folder):
                os.mkdir(test_folder)
            levels = nbgm_ini_file.GetLevles(title)
            if not levels:
                continue
            test_file = open(test_file_path, 'r')
            lineNum = 0
            for line in test_file:
                items = line.split(',')
                if len(items) < 2:
                    lineNum += 1
                    continue
                lat = float(items[0])
                lon = float(items[1])
                # set view port
                nbas_camera_set_viewport(lat, lon)
                if not CreateTrackFile(lat, lon):
                    break
                for i in xrange(2, len(levels)):
                    nbas_quit_if_aborted()
                    prop = levels[i]
                    if not prop.isValid():
                        continue
                    if i <= 15:
                        DeleteFilesInFolder(OUTPUT_PATH1,'.nbm')
                        if prop.hasBR():
                            if not CreateNbmTile(lon,lat,i,RASTER,region.mapPath):
                                break
                            DeleteFilesInFolder(OUTPUT_PATH2,'.nbm')
                            CopyAllNbmFilesFromFolder(OUTPUT_PATH1, OUTPUT_PATH2)
                        if prop.hasLBL():
                            if not CreateNbmTile(lon, lat, GetLableLevel(i), LABEL, region.mapPath):
                                break
                            if prop.hasBR():
                                CopyAllNbmFilesFromFolder(OUTPUT_PATH2, OUTPUT_PATH1)
                    elif i == 16:
                        DeleteFilesInFolder(OUTPUT_PATH1,'.nbm')
                        if not CreateNbmTile(lon, lat, 15, DVR_DVA, region.mapPath):
                            break
                        DeleteFilesInFolder2(OUTPUT_PATH1, 'LBLT.nbm')
                        DeleteFilesInFolder2(OUTPUT_PATH1, 'RAST.nbm')
                    if i <= 16:
                        nbas_unload_all_tiles()
                        nbas_wait_for_seconds(1)
                        nbmFileFolder = os.path.join(test_folder, 'Line%d_ZL%d' %(lineNum, i))
                        if not os.path.isdir(nbmFileFolder):
                            os.mkdir(nbmFileFolder)
                        CopyAllNbmFilesFromFolder(OUTPUT_PATH1, nbmFileFolder)

                    # load tiles
                    nbas_load_tile_folder(nbmFileFolder)
                    # set camera's state
                    nbas_camera_set_attitude(nbas_calc_view_distance_by_zoomlevel(i),0.0, 0.0)
                    nbas_wait_for_seconds(3)
                    imageFileName = os.path.join(test_folder, 'Line%d_ZL%d.bmp'% (lineNum, i))
                    info = 'lat=%f lon=%f ZoomLevel=%d'%(lat,lon, i)
                    # snap a screnen shot
                    nbas_snap_screenshot(imageFileName, info)
                    nbas_wait_for_seconds(1)

                lineNum += 1
            test_file.close()

    os.remove(BUG_TARCK_FILE)