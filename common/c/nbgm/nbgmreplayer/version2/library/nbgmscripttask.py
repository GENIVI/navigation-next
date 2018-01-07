import os, sys, shutil, imp
from task import *
from nbgmtask import *

NBGM_SCRIPT_CONTEX = None

# empty task, used to sleep for a while, avoiding dead lock
class NBGMEmptyTask(Task):
    def __init__(self, time_stamp):
        Task.__init__(self, tm = time_stamp)
    def __str__(self):
        return ''
    def __build__(self):
        return True
    def __run__(self):
        pass

# quit singal, used to abort user script
class NBGMQuitSignal(Exception):
    def __init__(self, discription = 'App is closing...'):
        Exception.__init__(self)
        self.__discription = discription
    def __str__(self):
        return '%s'%self.__discription


class NBGMScriptContext:
    def __init__(self, func_map = None, action_queue = None):
        self.func_map = func_map
        self.action_queue = action_queue
        self.loaded_tiles = set()
        self.viewpoint_distance = 1000.0
        self.rotate_angle = 0.0
        self.tilt_angle = 0.0
        self.__stop = False

    def addTask(self, func_name, data_list, time_stamp = 0.0):
        if self.__stop:
            raise NBGMQuitSignal()
        task = NBGMActionScriptTask(name = func_name,\
                                    func = self.func_map.get(func_name).func,\
                                    arg_list = data_list,\
                                    tm = time_stamp)
        if task.build():
            self.putTaskIntoQueue(task)
        else:
            logging.error('ActionScriptTask %s is invalid!', func_name)

    def WaitForSeconds(self, seconds):
        taskBegin = NBGMEmptyTask(0.0)
        self.putTaskIntoQueue(taskBegin)
        taskEnd = NBGMEmptyTask(seconds)
        self.putTaskIntoQueue(taskEnd)

    def putTaskIntoQueue(self, task):
        tryAgain = True
        while tryAgain and not self.__stop:
            try:
                self.action_queue.put(task, False, 0.1)
            except Exception:
                tryAgain = True
            else:
                tryAgain = False

    def addTile(self, tileId):
        if tileId:
            self.loaded_tiles.add(tileId)

    def removeTile(self, tileId):
        self.loaded_tiles.remove(tileId)

    def queryInfo(self, func_name):
        if func_name == "GetViewPointDistance":
            return self.viewpoint_distance
        elif func_name == "GetTiltAngle":
            return self.tilt_angle
        elif func_name == "GetRotateAngle":
            return self.rotate_angle
        else:
            assert(0)
            return None

    def stop(self):
        self.__stop = True


class UserScript(Thread):
    def __init__(self, scripy = None):
        self.scripy = scripy
        Thread.__init__(self, name = 'UserScript')

    def run(self):
        logging.info('Thread started...')
        if self.scripy:
            try:
                self.scripy()
            except Exception, e:
                logging.info('Thread aborted... \n*** %s ***', e)
                NBGM_SCRIPT_CONTEX.action_queue.queue.clear()
        NBGM_SCRIPT_CONTEX.action_queue.put(Task.genTerminator())
        logging.info('Thread exited...')

# 
#-----------------------------------------------------------------------------------------------------------------------------------------#
# NBGM Action task
#-----------------------------------------------------------------------------------------------------------------------------------------#
#
class NBGMActionScriptTask(NBGMTask):
    def __init__(self, *args, **argkv):
        NBGMTask.__init__(self, *args, **argkv)

    def __build__(self):
        self.para_count = len(self.para_list)
        assert(self.para_count <= len(self.run_methods))
        if not self.func_ptr:
            return False
        self.run_impl = self.run_methods[self.para_count]
        return True

    def __str__(self):
        if(self.para_count == 1):
            return ('%s(%s)')%(self.func_name, str(self.para_list[0]))
        elif(self.para_count > 1 and self.para_count < 5):
            return ('%s%s')%(self.func_name, self.para_list)
        else:
            return ('%s(...)')%(self.func_name)

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# NBGMTaskActionReader_v1
#-----------------------------------------------------------------------------------------------------------------------------------------#
#
class NBGMActionScriptReader_v1(NBGMTaskReader_v1):
    version = '1.0'

    def __init__(self, *args, **argkv):
        NBGMTaskReader_v1.__init__(self, *args, **argkv)
        self.__cache_size__ = 3
        self.queue = Queue(4096)
        self.user_script = UserScript()
        self.script_context = NBGMScriptContext()

    def initialize(self):
        if not self.loadFiles():
            return False
        self.script_context.action_queue = self.queue
        self.script_context.func_map = self.aux_map.copy()
        global NBGM_SCRIPT_CONTEX
        NBGM_SCRIPT_CONTEX = self.script_context
        try:
            abs_path = os.path.abspath(self.nbgm_log_path)
            m_path, m_name = os.path.split(abs_path)
            module_name = os.path.splitext(m_name)[0]
            sys.path.append(m_path)
            fp, pathname, description = imp.find_module(module_name)
            user_module = imp.load_module(module_name, fp, pathname, description)
            self.user_script.scripy = user_module.Run
            self.user_script.start()
        except Exception, e:
            print (e)
            if fp:
                fp.close()
            logging.error('NBGM Script(%s) is invalid!', self.nbgm_log_path)
            return False
        else:
            fp.close()
        return True

    def stop(self):
        self.script_context.stop()
        self.__stop__ = True

    def readTasks(self):
        if not self.hasMore():
            return []

        tasks = []
        while not self.__stop__:
            task = self.queue.get()
            if task.isTerminator():
                self.queue.task_done()
                self.__done_flag__ = True
                break
            tasks.append(task)
            self.queue.task_done()
            self.total_tasks += 1;
            if len(tasks) == self.__cache_size__:
                return tasks

        self.__done_flag__ = True
        return tasks