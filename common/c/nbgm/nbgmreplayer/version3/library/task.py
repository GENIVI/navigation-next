import logging
from threading import Thread, Event
from queue import Queue 
from time import strftime, localtime, sleep, time
from os import path
from struct import unpack

# a trick to declare a pure virtual method for base class
def abstract_method():
    raise NotImplementedError("abstract method, need to be implemented in subclass!")

class Task:
    '''Base class for a task. The most important attributes:

       func_name : string type, used to get function address from a native dll

       func_id   : uint8 type, used to identify the function, must be unique

       time_stamp: float type, used to record the time of task, with millisecond precision

    Please note that tasks must be builted and verified vaild first.
    '''
    Terminator_ID = -1

    def __init__(self, id = 0, tm = 0.0, func = None, name = 'unknown'):
        self.func_name    = name
        self.func_id      = id
        self.func_ptr     = func
        self.time_stamp   = tm

    @staticmethod
    def genTerminator():
        return Task(id = Task.Terminator_ID)

    def isTerminator(self):
        return self.func_id <= Task.Terminator_ID

    def __str__(self):
        return ('%03d - %s')%(self.func_id, self.func_name)

    def build(self):
        valid_flag = self.__build__()
        if not valid_flag:
            logging.error('Cannot build this task: %s', str(self))
        return valid_flag

    def run(self):
        self.__run__()

    # must impletment this!
    def __build__(self): abstract_method()
    # must impletment this!
    def __run__(self)  : abstract_method()



class ITaskReader:
    '''Interface for task reader, used to parse files to generate tasks.
Usually, you should derive a class from this, and implement the readTasks method
    '''
    def __init__(self, cache_size = 1024):
        self.__valid_flag__ = True
        self.__done_flag__  = False
        self.__cache_size__ = cache_size
        self.__task_cache__ = []
        self.source_size = 0
        self.total_tasks = 0

    def initialize(self):
        return True

    def deinitialize(self):
        return True

    def hasMore(self):
        return self.__valid_flag__ and not self.__done_flag__

    def reset(self):
        self.__valid_flag__ = True
        self.__done_flag__  = False
        self.__task_cache__ = []
        self.total_tasks = 0

    # must impletment this!
    def readTasks(self):abstract_method()


# interface for task scheduler
class ITaskScheduler :
    '''Interface for task scheduler, used to run tasks by some scheduling policy.
Usually, you should derive a class from this, and implement the scheduleTasks method
    '''
    def __init__(self):
        self.total_tasks = 0
        self.total_time = 0.0

    def initialize(self):
        return True

    def deinitialize(self):
        return True

    def pause(self, pause_flag):
        pass

    def stop(self):
        pass

    def reset(self):
        pass

    # must impletment this!
    def scheduleTasks(self, task):abstract_method()