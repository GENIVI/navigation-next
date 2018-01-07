from xml.etree.ElementTree import ElementTree 
from task    import *
from taskapp import *
from ctypes  import *
from struct  import *

def Char2Ctype(t):
    if t == 'i':
        return c_int
    elif t == 'I':
        return c_uint 
    elif t == 'f':
        return c_float
    elif t == 'd':
        return c_double
    elif t == 'c':
        return c_char
    elif t == 'b':
        return c_byte
    elif t == 'B':
        return c_ubyte
    elif t == 'h':
        return c_short
    elif t == 'H':
        return c_ushort
    elif t == 'q':
        return c_int64
    elif t == 'Q':
        return c_uint64
    elif t == 's':
        return c_char_p
    elif t == 'a':
        return c_void_p
    else:
        assert(False)
        return None

def GetFuncArgTypes(arg_format, arg_types):
    for c in arg_format:
        if not c.isalpha():
            continue
        arg_types.append(Char2Ctype(c))

def GetFuncRetType(return_type):
    return Char2Ctype(return_type)

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# NBGM task
#-----------------------------------------------------------------------------------------------------------------------------------------#
#
class NBGMTask(Task):
    def __init__(self, cnt = 0, data = None, fm = '', arg_list = [], *args, **argkv):
        Task.__init__(self, *args, **argkv)
        self.para_data = data
        self.para_format = fm
        self.para_count = cnt
        self.para_list = arg_list
        self.run_impl = self.__run0
        self.run_methods = (self.__run0, self.__run1, self.__run2,self.__run3, self.__run4, self.__run5, self.__run6, self.__run7, self.__run8)

    def __str__(self):
        if(self.para_count == 1):
            return ('%03d - %s(%s)')%(self.func_id, self.func_name, str(self.para_list[0]))
        elif(self.para_count > 1 and self.para_count < 5):
            return ('%03d - %s%s')%(self.func_id, self.func_name, self.para_list)
        else:
            return ('%03d - %s(...)')%(self.func_id, self.func_name)

    def __run__(self):
        self.run_impl()
    def __run0(self):
        self.func_ptr()
    def __run1(self):
        self.func_ptr(self.para_list[0])
    def __run2(self):
        self.func_ptr(self.para_list[0], self.para_list[1])
    def __run3(self):
        self.func_ptr(self.para_list[0], self.para_list[1], self.para_list[2])
    def __run4(self):
        self.func_ptr(self.para_list[0], self.para_list[1], self.para_list[2], self.para_list[3])
    def __run5(self):
        self.func_ptr(self.para_list[0], self.para_list[1], self.para_list[2], self.para_list[3], self.para_list[4])
    def __run6(self):
        self.func_ptr(self.para_list[0], self.para_list[1], self.para_list[2], self.para_list[3], self.para_list[4], self.para_list[5])
    def __run7(self):
        self.func_ptr(self.para_list[0], self.para_list[1], self.para_list[2], self.para_list[3], self.para_list[4], self.para_list[5], self.para_list[6])
    def __run8(self):
        self.func_ptr(self.para_list[0], self.para_list[1], self.para_list[2], self.para_list[3], self.para_list[4], self.para_list[5], self.para_list[6], self.para_list[7])

    def __build__(self):
        assert(self.para_count >= 0 and self.para_count < len(self.run_methods))
        if self.func_ptr is None:
            return False
        self.run_impl = self.run_methods[self.para_count]
        if self.para_count == 0:
            return True
        if self.para_count > 0:
            if len(self.para_format) == 0 or self.para_data is None:
                return False
            # little end
            self.para_list = unpack(''.join(('<', self.para_format)), self.para_data)
            if len(self.para_list) != self.para_count:
                return False
        return True

class FuncInfo:
    def __init__(self, name = '', format = '', count = 0, return_type = None, func = None):
        self.name = name
        self.count = count
        self.format = format
        self.func = func
        self.arg_types = []
        self.ret_type = return_type

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# NBGMTaskReader_v1
#-----------------------------------------------------------------------------------------------------------------------------------------#
#
class NBGMTaskReader_v1(ITaskReader):
    version = '1.0'

    def __init__(self, nbgm_log_path, nbgm_dll_path, nbgm_xml_path):
        ITaskReader.__init__(self)
        self.nbgm_log_path = nbgm_log_path
        self.nbgm_dll_path = nbgm_dll_path
        self.nbgm_xml_path = nbgm_xml_path
        self.nbgm_log  = None
        self.func_map = {}
        self.type_map = {}
        self.id_map = {}
        self.aux_map = {}
        self.nbgm_dll = None
        self.__stop__  = False

    def initialize(self):
        if not self.loadFiles():
            return False
        self.nbgm_log = open(self.nbgm_log_path, 'rb')
        return True

    def deinitialize(self):
        if self.nbgm_log:
            self.nbgm_log.close()
            self.nbgm_log = None
        self.func_map.clear()
        self.type_map.clear()
        self.id_map.clear()
        self.aux_map.clear()
        return True
    
    def stop(self):
        self.__stop__  = True

    def loadFiles(self):
        #load log
        if not self.loadLog():
            self.__vaild_flag__ = False
            logging.error('Load NBGM Target failed! %s', self.nbgm_log_path)
            return False

        #load xml
        if not self.loadXML():
            self.__vaild_flag__ = False
            logging.error('Load NBGM XML failed! %s', self.nbgm_xml_path)
            return False

        #load dll
        if not self.loadNBGM():
            self.__vaild_flag__ = False
            logging.error('Load NBGM DLL failed! %s', self.nbgm_dll_path)
            return False
        return True

    def loadLog(self):
        assert(path.isfile(self.nbgm_log_path))
        self.source_size= path.getsize(self.nbgm_log_path)
        if self.source_size == 0:
            return False
        return True

    def loadNBGM(self):
        assert(path.isfile(self.nbgm_dll_path))
        # load nbgm
        self.nbgm_dll = CDLL(self.nbgm_dll_path)
        # update func pointer
        for func_id,func_info in self.func_map.iteritems():
            try:
                func_ptr = self.nbgm_dll.__getitem__(func_info.name)
            except Exception,e:
                logging.error('Cannot find the function in NBGM DLL function: %s', func_info.name)
                return False
            else:
                function_info = self.func_map[func_id]
                function_info.func = func_ptr
                GetFuncArgTypes(function_info.format, function_info.arg_types)
                function_info.func.argtypes = function_info.arg_types
                if function_info.ret_type:
                    function_info.func.restype = GetFuncRetType(function_info.ret_type)
                self.aux_map[function_info.name] = function_info
        return True

    def loadXML(self):
        assert(path.isfile(self.nbgm_xml_path))
        # build xml tree
        self.nbgm_tree = ElementTree()
        try:
            self.nbgm_tree.parse(self.nbgm_xml_path)
        except Exception, e:
            print(e)
            logging.error('NBGM XML file is invalid!')
            return False

        # check nbgm version
        nbgm_root = self.nbgm_tree.getroot()
        if nbgm_root is None or nbgm_root.tag != 'nbgm':
            logging.error('NBGM XML file is invalid, please check the root node!')
            return False
        nbgm_version = nbgm_root.attrib.get('version')
        if nbgm_version != NBGMTaskReader_v1.version:
            logging.error('NBGM XML file is invalid, please check the version!')
            return False

        # read arg types
        types = self.nbgm_tree.find('arg_types')
        if types is None or len(types)== 0:
            logging.error('Argument type table is empty in NBGM XML file!')
            return False

        for t in types:
            t_name = t.attrib.get('name')
            t_id = t.attrib.get('id')
            if t_name is None or t_id is None:
                continue
            if t_name in self.type_map:
                logging.error('Type name(%s) is not unique in NBGM XML file!', t_name)
                return False
            self.type_map[t_name] = t_id

        # read function ids
        ids = self.nbgm_tree.find('func_ids')
        if ids is None or len(ids)== 0:
            logging.error('Function id table is empty in NBGM XML file!')
            return False

        for function_id in ids:
            f_name = function_id.attrib.get('name')
            if f_name is None:
                continue
            temp = function_id.attrib.get('id')
            if temp is None:
                continue
            f_id = int(temp)
            if f_name in self.id_map:
                logging.error('Function Name(%s) is not unique in NBGM XML file!', f_name)
                return False
            self.id_map[f_name] = f_id

        # read function info
        func_container = self.nbgm_tree.findall('function')
        if func_container is None or len(func_container) == 0:
            logging.error('Function list is empty in NBGM XML file!')
            return False	    
        for func in func_container:
            f_name = func.attrib.get('name')
            if f_name is None:
                logging.error('Function (%s) is invalid in NBGM XML file: %d', f_name)
                return False
            f_id = self.id_map.get(f_name)
            if f_id is None:
                logging.error('Function (%s) is invalid in NBGM XML file: %d', f_name)
                return False
            f_return = func.attrib.get('return')
            if f_return:
                f_return = self.type_map.get(f_return)
            arg_list = func.find('arg_list')
            if arg_list is None:
                logging.error('Function (%s) is invalid in NBGM XML file: %d', f_name)
                return False
            arg_count = int(arg_list.attrib.get('count'))
            list_count = len(arg_list)
            if arg_count is not None:
                if arg_count != list_count:
                    logging.error('Function (%s) arg count is invalid in the NBGM XML file!', f_name)
                    return False
            arg_format = ''
            for arg in arg_list:
                type_id = self.type_map.get(arg.attrib.get('type'))
                if type_id is None:
                    logging.error('Function (%s) arg type is invalid in NBGM XML file!', f_name)
                    return False
                arg_format += type_id

            self.func_map[f_id] = FuncInfo(f_name, arg_format, arg_count, f_return)
        return True

    def processString(self, arg_format):
        count = arg_format.count('s')
        if count == 0:
            return arg_format
        assert(arg_format.count('a')== 0)

        ret = arg_format
        try:
            for i in range(count):
                length = unpack("<B", self.nbgm_log.read(1))
                assert(length[0] < 256)
                ret = ret.replace('s', str(length[0])+'$', 1)
            ret = ret.replace('$', 's')
        except Exception, e:
            return ''
        else:
            return ret

    def readTasks(self):
        if not self.hasMore():
            return []
        tasks = []
        try:
            while not self.__stop__:
                header = unpack("<BI", self.nbgm_log.read(5))
                assert(header)
                func_id = header[0]
                func_tm = header[1]/1000.0
                func_info = self.func_map.get(func_id)
                if func_info is None:
                    logging.error('Function id(%d) in the log file is invalid ', func_id)
                    self.__done_flag__ = True
                    return []

                args_data = None
                arg_format = ''
                if func_info.count > 0:
                    arg_format = self.processString(func_info.format)
                    if len(arg_format) == 0:
                        logging.error('Found Function call(id : %d) is invalid in the log file', func_id)
                        self.__done_flag__ = True
                        return []

                byte_size = calcsize(arg_format)
                if  byte_size > 0:
                    args_data = self.nbgm_log.read(byte_size)

                task = NBGMTask(id = func_id,\
                                tm = func_tm,\
                                func = func_info.func,\
                                cnt = func_info.count,\
                                data = args_data,\
                                fm = arg_format,\
                                name = func_info.name)
                if not task.build():
                    logging.error('Found a function call(%s)is invalid in the log file', str(task))
                    continue
                tasks.append(task)
                self.total_tasks += 1;
                if len(tasks) == self.__cache_size__:
                    return tasks
        except Exception, e:
            self.__done_flag__ = True
            return tasks
        else:
            self.__done_flag__ = True
            return tasks

#
#-----------------------------------------------------------------------------------------------------------------------------------------#
# NBGMTaskScheduler
#-----------------------------------------------------------------------------------------------------------------------------------------#
#
class NBGMTaskScheduler(ITaskScheduler):
    def __init__(self, device):
        assert(device)
        ITaskScheduler.__init__(self)
        self.device = device
        self.__pause_event = Event()
        self.__sleep_event = Event()
        self.__last_run_time  = 0.0
        self.__last_task_time = 0.0
        self.__first_run_time = 0.0
        self.__min_time  = 0.0025
        self.__pause = False
        self.__stop  = False

    def initialize(self):
        return True

    def deinitialize(self):
        self.total_time = self.__last_run_time - self.__first_run_time
        return True

    def pause(self, pause_flag):
        self.__pause = pause_flag
        if not pause_flag:
            self.__pause_event.set()

    def stop(self):
        self.__stop  = True
        self.__pause = False
        self.__sleep_event.set()
        self.__pause_event.set()

    def reset(self):
        self.__stop  = False
        self.__pause = False
        self.__last_task_time = 0.0
        self.__last_run_time  = 0.0
        self.__first_run_time = 0.0
        self.__sleep_event.clear()

    def scheduleTasks(self, task):
        assert(task)
        # first one
        if self.__last_run_time == 0.0:
            self.__last_task_time = task.time_stamp
            self.__last_run_time = self.__first_run_time = time()
            self.runTask(task)
            return
        dt_task = task.time_stamp - self.__last_task_time
        dt_run  = time() - self.__last_run_time 
        delay   = dt_task - dt_run
        # delay task
        if delay > self.__min_time:
            self.__sleep_event.wait(delay)
        if self.__stop:
            return
        if self.__pause:
            self.__pause_event.clear()
            self.__pause_event.wait()
        self.__last_task_time = task.time_stamp
        self.__last_run_time = time()
        # run task
        self.runTask(task)

    def runTask(self, task):
        task.run()
        self.total_tasks += 1
        info = str(task)
        if info:
            logging.debug(info)