#!/usr/bin/env python
# -*- coding: utf-8 -*-

## TODO: Moving threading into GuiApp.py!!
import os
import time
import threading
import sys

sys.path.append(os.path.join('TestFramework', 'Platform'))

class WorkingThread(threading.Thread):
    """
    """

    def __init__(self, lock, executer):
        """
        """
        super(WorkingThread, self).__init__(name='Workign thread')
        self.executer = executer

    def run(self):
        """
        """
        r = self.executer.GenerateCode()
        print("r = %d\n"%(r))
        r = self.executer.Compile() if r == 0 else r
        print("r = %d\n"%(r))
        r = self.executer.Run() if r == 0 else r
        print("r = %d\n"%(r))
        self.executer.callback.OnFinish() # improve this

class TestExecutor(object):
    """
    """

    def __init__(self, logger, async=True):
        """

        Arguments:
        - `logger`:
        """
        self.async = async
        print sys.modules['TestExecutor'].__file__
        if logger is None:
            class InternalLogger(object):
                """
                """

                def __init__(self):
                    """
                    """
                    pass
                def WriteLog(self, fmt, *args):
                    """
                    """
                    print fmt%args
            logger = InternalLogger()

        self._logger = logger
        self.lock = None

        self._impl = __import__("Executor", globals(), locals(),
                                ['*']).get_executor(Logger=self)

        ## TODO: parse platform from arguments...
        # try:
        #     self._impl = __import__(OPTS.platform, globals(), locals(),
        #                             ['*']).get_compiler(self)
        # except:
        #     print("Failed to load platform utilities, using the default one...\n")
        #     self._impl = __import__("Executor", globals(), locals(),
        #                             ['*']).get_compiler(self)

    def SetTestCases(self, cases):
        """

        Arguments:

        - `cases`:
        """
        self._cases = cases

    def SetCallback(self, cb):
        """

        Arguments:

        - `cb`:
        """
        self.callback = cb

    def Begin(self):
        """
        """
        if self.async:
            self.lock = threading.Lock()
            thread = WorkingThread(self.lock, self)
            thread.start()
        else:
            self.GenerateCode()
            self.Compile()
            self.Run()

    def GenerateCode(self):
        """
        """
        return self._impl.generate_codes(self._cases)

    def Compile(self):
        """
        """
        return self._impl.compile();

    def Run(self):
        """
        """
        # Should be moved to another thread.
        return self._impl.run()

    def Stop(self):
        """
        """
        #TODO: kill thread!
        pass

    def WriteLog(self, fmt, *args):
        """
        """
        if self.lock:
            self.lock.acquire()
        self._logger.WriteLog(fmt % args if args else fmt)
        if self.lock:
            self.lock.release()
