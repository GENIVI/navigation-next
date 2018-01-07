from ctypes import *

WORD   = c_ushort
DWORD  = c_ulong
LPBYTE = POINTER(c_ubyte)
LPTSTR = POINTER(c_char)
HANDLE = c_void_p

WAIT_OBJECT_0  = 0x00000000
WAIT_TIMEOUT   = 0x00000102
WAIT_FAILED    = 0xFFFFFFFF

RASTER  = 0
LABEL   = 1
DVR_DVA = 2

class STARTUPINFO(Structure):
    _fields_ = [
        ("cb",            DWORD),
        ("lpReserved",    LPTSTR),
        ("lpDesktop",     LPTSTR),
        ("lpTitle",       LPTSTR),
        ("dwX",           DWORD),
        ("dwY",           DWORD),
        ("dwXSize",       DWORD),
        ("dwYSize",       DWORD),
        ("dwXCountChars", DWORD),
        ("dwYCountChars", DWORD),
        ("dwFillAttribute",DWORD),
        ("dwFlags",       DWORD),
        ("wShowWindow",   WORD),
        ("cbReserved2",   WORD),
        ("lpReserved2",   LPBYTE),
        ("hStdInput",     HANDLE),
        ("hStdOutput",    HANDLE),
        ("hStdError",     HANDLE),
        ]

class PROCESS_INFORMATION(Structure):
    _fields_ = [
        ("hProcess",    HANDLE),
        ("hThread",     HANDLE),
        ("dwProcessId", DWORD),
        ("dwThreadId",  DWORD),
        ]

class LoaderWin32():
    def __init__(self):
        self.hProcess = None
        self.pid = None
        self.lastError = 0
        self.waitResult = ''

    def load(self, command_line, creation_flags = 0):
        startupinfo = STARTUPINFO()
        process_information = PROCESS_INFORMATION()
        startupinfo.dwFlags = 1
        startupinfo.wShowWindow = 0
        startupinfo.cb = sizeof(startupinfo)
        if windll.kernel32.CreateProcessA(None,
                                   command_line,
                                   None,
                                   None,
                                   False,
                                   creation_flags,
                                   None,
                                   None,
                                   byref(startupinfo),
                                   byref(process_information)):
            self.hProcess = process_information.hProcess
            self.pid = process_information.dwProcessId
            return True
        else:
            self.lastError = windll.kernel32.GetLastError()
            return False

    def wait(self, max_wait_time = 99999999):
        if max_wait_time <= 0 or self.lastError != 0 or not self.hProcess:
            self.waitResult = WAIT_FAILED
            return
        self.waitResult = windll.kernel32.WaitForSingleObject(self.hProcess, max_wait_time*1000)