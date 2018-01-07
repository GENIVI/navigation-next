import logging
from ctypes import CDLL
from threading import Thread, Event
from time import sleep

# a simple state query implement
class SimpleStateQuery(Thread):
    def __init__(self, query_func, tm_interval = 0.33, tname = 'StateControler'):
        Thread.__init__(self, name = tname)
        self.__time = tm_interval
        self.__stopEvent = Event()
        self.__query_func = query_func
        self.__respond_func = {}

    def registerKyeFunc(self, c, func):
        assert(func)
        self.__respond_func[c] = func

    def stop(self):
        self.__stopEvent.set()

    def run(self):
        self.__stopEvent.clear()
        while not self.__stopEvent.is_set():
            self.__stopEvent.wait(self.__time)
            if self.__query_func and self.__respond_func:
                for k, func in self.__respond_func.iteritems():
                    if(self.__query_func(k)):
                        func()

class NBGMDevice:
    '''a python class for wrapping nbgm native device
When you want to use this class, you should follow these steps:
     1 create device first
     2 init NBGM
     3 register key events, such as, r, s, p
     4 start the mainloop
     5 deinit NBGM
     6 destroy device last
Please note that the main thread and the render thread could be seperated. 
    '''

    def __init__(self, native_dll_path, work_folder, tile_folder, useOpengl = True, width = 800, height = 600, dpi = 96):
        assert(native_dll_path and work_folder and tile_folder)
        self.device = CDLL(native_dll_path)
        self.workPath = work_folder
        self.tilePath = tile_folder
        self.width = width
        self.height = height
        self.stateThread = SimpleStateQuery(self.device.IsKeyDown)
        self.useOpenGL = useOpengl
        self.dpi = int(dpi)

    def create(self):
        return self.device.CreateNativeDevice(self.workPath, self.tilePath, self.width, self.height, self.dpi, self.useOpenGL)

    def destroy(self):
        self.device.DestroyNativeDevice()

    def startMainLoop(self):
        self.stateThread.start()
        self.device.StartMainLoop()
        self.stateThread.stop() 
        self.stateThread.join()

    def registerKeyEvent(self, char, func):
        if char and func:
            self.stateThread.registerKyeFunc(ord(char), func)