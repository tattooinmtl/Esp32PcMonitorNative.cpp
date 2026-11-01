import time
import ctypes
import serial

PORT = "COM4"
BAUD = 115200

class MEMORYSTATUSEX(ctypes.Structure):
    _fields_ = [
        ("dwLength", ctypes.c_ulong),
        ("dwMemoryLoad", ctypes.c_ulong),
        ("ullTotalPhys", ctypes.c_ulonglong),
        ("ullAvailPhys", ctypes.c_ulonglong),
        ("ullTotalPageFile", ctypes.c_ulonglong),
        ("ullAvailPageFile", ctypes.c_ulonglong),
        ("ullTotalVirtual", ctypes.c_ulonglong),
        ("ullAvailVirtual", ctypes.c_ulonglong),
        ("sullAvailExtendedVirtual", ctypes.c_ulonglong),
    ]

class FILETIME(ctypes.Structure):
    _fields_ = [
        ("dwLowDateTime", ctypes.c_ulong),
        ("dwHighDateTime", ctypes.c_ulong),
    ]

def filetime_to_int(ft):
    return (ft.dwHighDateTime << 32) + ft.dwLowDateTime

def get_ram():
    mem = MEMORYSTATUSEX()
    mem.dwLength = ctypes.sizeof(MEMORYSTATUSEX)
    ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(mem))

    total = mem.ullTotalPhys
    avail = mem.ullAvailPhys
    used = total - avail
    percent = used * 100.0 / total

    return percent, used / 1024**3, total / 1024**3

def get_cpu_sample():
    idle = FILETIME()
    kernel = FILETIME()
    user = FILETIME()
    ctypes.windll.kernel32.GetSystemTimes(
        ctypes.byref(idle),
        ctypes.byref(kernel),
        ctypes.byref(user)
    )
    return filetime_to_int(idle), filetime_to_int(kernel), filetime_to_int(user)

def cpu_percent(prev, cur):
    idle_prev, kernel_prev, user_prev = prev
    idle_cur, kernel_cur, user_cur = cur

    idle_delta = idle_cur - idle_prev
    kernel_delta = kernel_cur - kernel_prev
    user_delta = user_cur - user_prev

    total = kernel_delta + user_delta
    if total == 0:
        return 0.0

    return max(0.0, min(100.0, 100.0 * (1.0 - idle_delta / total)))

with serial.Serial(PORT, BAUD, timeout=1) as ser:
    time.sleep(2)
    prev_cpu = get_cpu_sample()

    while True:
        time.sleep(1)
        cur_cpu = get_cpu_sample()
        cpu = cpu_percent(prev_cpu, cur_cpu)
        prev_cpu = cur_cpu

        ram_percent, ram_used, ram_total = get_ram()

        line = f"CPU={cpu:.1f},RAM={ram_percent:.1f},USED={ram_used:.1f},TOTAL={ram_total:.1f}\n"
        print(line.strip())
        ser.write(line.encode("utf-8"))