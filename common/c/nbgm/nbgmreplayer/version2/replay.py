'''
main module of nbgm replay system  version: 1.0
Please use -h as a command line argument for more help
'''

import os, sys, logging, getopt, env

# initialize the enviroment
if __name__ == '__main__':
    # get the root path, and changethe current work path to root_path/bin/
    if len(env.CURREN_APP_PATH) == 0:
        NBGM_ROOT_PATH = os.getcwd()
        CURREN_WORK_SPACE = os.path.join(NBGM_ROOT_PATH, 'bin%s'%os.sep)
        os.chdir(CURREN_WORK_SPACE)
    else:
        NBGM_ROOT_PATH = env.CURREN_APP_PATH
        CURREN_WORK_SPACE = os.path.join(NBGM_ROOT_PATH, 'bin%s'%os.sep)

    # check if the libary folder existed or not
    library_path = os.path.join(NBGM_ROOT_PATH, 'library%s'%os.sep)
    if not os.path.isdir(library_path):
        print('App exited...Cannot find the library folder %s!' % library_path)
        quit()     
    sys.path.append(library_path)
    
    # check if the resource folder existed or not
    NBGM_RES_PATH = os.path.join(NBGM_ROOT_PATH, 'resource%s'%os.sep)
    if not os.path.isdir(NBGM_RES_PATH):
        print ('App exited...Cannot find the resouce folder %s!' % NBGM_RES_PATH)
        quit()
    os.environ['nbgm_resource'] = NBGM_RES_PATH

    # check if the tiles folder existed or not
    NBGM_TILE_PATH = os.path.join(NBGM_ROOT_PATH, 'tiles%s'%os.sep)
    if not os.path.isdir(NBGM_TILE_PATH):
        print ('App exited...Cannot find the tile folder %s!' % NBGM_TILE_PATH)
        quit()
    os.environ['nbgm_tiles'] = NBGM_TILE_PATH

    # check if the screenshots folder existed or not  
    NBGM_SCREENSHOTS_PATH = os.path.join(NBGM_ROOT_PATH, 'screenshots%s'%os.sep)
    if not os.path.isdir(NBGM_SCREENSHOTS_PATH):
        os.mkdir(NBGM_SCREENSHOTS_PATH)
    os.environ['nbgm_screenshots'] = NBGM_SCREENSHOTS_PATH
    
    NBGM_USER_FILE = os.path.join(NBGM_ROOT_PATH, 'user%smain.py'%os.sep)
    NBGM_DLL_FILE = os.path.join(CURREN_WORK_SPACE, env.NBGM_LIBRARY_DEFAULT_NAME)
    NBGM_XML_FILE = os.path.join(CURREN_WORK_SPACE, 'nbgm.xml')
    NBGM_USE_OPENGL = True
    NBMG_SCRIPT_MODE = True
    NBGM_WIN_WIDTH = env.DEFAULT_WIDHT
    NBGM_WIN_HEIGHT = env.DEFAULT_HEIGHT
    NBGM_SCREEN_DPI = env.DEFAULT_DPI
    VERSION_INFO = 'NBGM Library Version = %s \nAPP Version = %s \n'%(env.NBGM_LIBRARY_VERSION, env.APP_VERSION)


from taskapp import *
from nbgmtask import *
from nbgm import *
from nbgmscripttask import *


NBMG_HELP_INFO = '''
==================================== NBGM Replay Help ======================================

>> Command Line

   * --load:   specify nbgm log/script file path, default file is './nbgm.log'
   * --dll:    specify nbgm dll file path, default file is './bin/nbgm14.dll'
   * --xml:    specify nbgm xml file path, default file is './bin/nbgm.xml'
   * --res:    specify nbgm resource folder, defualt path is './resource/'
   * --tiles:  specify nbgm tile folder, defualt path is './tiles/'
   * --output: sepcify replay log filename
   * --width:  sepcify the width of the window, default is 800
   * --height: sepcify the width of the window, default is 600
   * --d3d     enbale Direct3D to render, default render system is OpenGL
   * --info,-i show verbose information of task execution
   * --help,-h help
   * --version,-v show version


>> Repaly Control

   * p  pause/resume the replay

=============================================================================================
'''


def PrintSummary(reader, runner):
    print("""
=================================== NBGM Replay Summary =====================================

>> Path

  * nbgm target file = %s
  * nbgm dll file = %s
  * nbgm xml file = %s
  * resource path = %s
  * tiles path = %s


>> Result

  * nbgm target file size = %.2f KB
  * total number of parsed commands = %d
  * total number of executed commands = %d
  * simulation execution time = %.3f s


==============================================================================================
"""
          % (NBGM_USER_FILE, NBGM_DLL_FILE, NBGM_XML_FILE, NBGM_RES_PATH, NBGM_TILE_PATH,\
             reader.source_size/1024., reader.total_tasks, runner.total_tasks, runner.total_time))




#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# Command Line Check
#-----------------------------------------------------------------------------------------------------------------------------------------#
#
def ProcessCommandLine():
    global NBGM_ROOT_PATH, NBGM_USER_FILE, NBGM_DLL_FILE, NBGM_XML_FILE, NBGM_RES_PATH, NBGM_TILE_PATH, NBGM_TASK_INFO_FLAG, NBGM_USE_OPENGL,NBMG_SCRIPT_MODE,NBGM_WIN_HEIGHT,NBGM_WIN_WIDTH
    opts, args = getopt.getopt(sys.argv[1:],'hiv', ['load=', 'dll=', 'xml=', 'res=', 'tiles=', 'output=', 'width=', 'height=','info', 'd3d', 'help', 'version'])
    for opt, val in opts:
        if opt == '--load':
            if os.path.isabs(val):
                NBGM_USER_FILE = val
            else:
                NBGM_USER_FILE = os.path.join(NBGM_ROOT_PATH, val)
            ext = os.path.splitext(NBGM_USER_FILE)[1][1:]
            if ext == 'py':
                NBMG_SCRIPT_MODE = True
            elif ext == 'log':
                NBMG_SCRIPT_MODE = False
            else:
                print ('App exited... Cannot load the target file %s !'%NBGM_USER_FILE)
                return False

        elif opt == '--dll':
            if os.path.isabs(val): 
                NBGM_DLL_FILE = val
            else:
                NBGM_DLL_FILE = os.path.join(CURREN_WORK_SPACE, val)
        elif opt == '--xml':
            if os.path.isabs(val):
                NBGM_XML_FILE = val
            else:
                NBGM_XML_FILE = os.path.join(CURREN_WORK_SPACE, val)
        elif opt == '--res':
            if val[len(val)-1] != '\\':
                val += '\\'
            NBGM_RES_PATH = val
        elif opt == '--tiles':
            if val[len(val)-1] != '\\':
                val += '\\'
            NBGM_TILE_PATH = val
        elif opt in('-i','--info'):
            logging.getLogger().setLevel(logging.DEBUG)
        elif opt == '--width':
            NBGM_WIN_WIDTH = int(val)
        elif opt == '--height':
            NBGM_WIN_HEIGHT = int(val)        
        elif opt == '--d3d':
            NBGM_USE_OPENGL = False
        elif opt == '--output':
            outputLogFile = ''
            if os.path.isabs(val):
                outputLogFile = val
            else:
                outputLogFile = os.path.join(NBGM_ROOT_PATH, val)            
            logging.getLogger().addHandler(logging.FileHandler(outputLogFile))
        elif opt in ('-h','--help'):
            print (NBMG_HELP_INFO)
            return False
        elif opt in ('-v','--version'):
            print (VERSION_INFO)
            return False

    if not os.path.isfile(NBGM_XML_FILE):
        logging.critical('Cannot find NBGM xml file "%s" !', NBGM_XML_FILE)
        return False

    if not os.path.isfile(NBGM_USER_FILE):
        logging.critical('Cannot find the target file "%s" !', NBGM_USER_FILE)
        return False

    if not os.path.isfile(NBGM_DLL_FILE):
        logging.critical('Cannot find NBGM dll file "%s" !', NBGM_DLL_FILE)
        return False

    if not os.path.isdir(NBGM_RES_PATH):
        logging.critical('Cannot find NBGM work path "%s" !', NBGM_RES_PATH)
        return False

    if not os.path.isdir(NBGM_TILE_PATH):
        logging.critical('Cannot find NBGM tile path "%s" !', NBGM_TILE_PATH)
        return False

    os.environ['nbgm_height'] = str(NBGM_WIN_HEIGHT)
    os.environ['nbgm_width'] = str(NBGM_WIN_WIDTH)
    os.environ['nbgm_tiles'] = NBGM_TILE_PATH
    os.environ['nbgm_resource'] = NBGM_RES_PATH
    return True

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# Entry Point
#-----------------------------------------------------------------------------------------------------------------------------------------#
#
def main():
    # command line
    if not ProcessCommandLine():
        return

    # create a nbgm device through a native libray
    nbgm = NBGMDevice(NBGM_DLL_FILE, NBGM_RES_PATH, NBGM_TILE_PATH, NBGM_USE_OPENGL, NBGM_WIN_WIDTH, NBGM_WIN_HEIGHT, NBGM_SCREEN_DPI)

    # create a nbgm task reader
    Reader = NBMG_SCRIPT_MODE and NBGMActionScriptReader_v1 or NBGMTaskReader_v1
    nbgm_reader = Reader(NBGM_USER_FILE, NBGM_DLL_FILE, NBGM_XML_FILE)

    # create a nbgm task runner
    nbgm_runner = NBGMTaskScheduler(nbgm)

    # create an app instance
    replayer = TaskApp(nbgm, nbgm_reader, nbgm_runner)

    # run it
    replayer.run()

    # show summary
    PrintSummary(nbgm_reader, nbgm_runner)


if __name__ == '__main__':
    # init log format
    logging.basicConfig(format = '%(asctime)s.%(msecs)03d - %(levelname)-5s - %(threadName)-12s - %(message)s' ,\
                        datefmt = '%H:%M:%S', level = logging.INFO)
    main()