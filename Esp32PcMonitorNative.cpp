#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

#define BAUD_RATE CBR_115200
#define SEND_MS 1000

static HANDLE gSerial = INVALID_HANDLE_VALUE;
static ULONGLONG lastIdle = 0, lastKernel = 0, lastUser = 0;

ULONGLONG FileTimeToU64(const FILETIME& ft) {
    return ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
}

double GetCpuPercent() {
    FILETIME idle, kernel, user;
    if (!GetSystemTimes(&idle, &kernel, &user)) return 0.0;

    ULONGLONG i = FileTimeToU64(idle);
    ULONGLONG k = FileTimeToU64(kernel);
    ULONGLONG u = FileTimeToU64(user);

    ULONGLONG idleDelta = i - lastIdle;
    ULONGLONG totalDelta = (k - lastKernel) + (u - lastUser);

    lastIdle = i;
    lastKernel = k;
    lastUser = u;

    if (totalDelta == 0) return 0.0;

    double cpu = 100.0 * (1.0 - ((double)idleDelta / (double)totalDelta));
    if (cpu < 0) cpu = 0;
    if (cpu > 100) cpu = 100;
    return cpu;
}

void GetRam(double* percent, double* usedGb, double* totalGb) {
    MEMORYSTATUSEX mem;
    ZeroMemory(&mem, sizeof(mem));
    mem.dwLength = sizeof(mem);

    if (!GlobalMemoryStatusEx(&mem)) {
        *percent = *usedGb = *totalGb = 0;
        return;
    }

    ULONGLONG total = mem.ullTotalPhys;
    ULONGLONG free = mem.ullAvailPhys;
    ULONGLONG used = total - free;

    *percent = (double)used * 100.0 / (double)total;
    *usedGb = (double)used / 1024.0 / 1024.0 / 1024.0;
    *totalGb = (double)total / 1024.0 / 1024.0 / 1024.0;
}

double GetDiskCPercent() {
    ULARGE_INTEGER freeBytes, totalBytes, totalFree;
    if (!GetDiskFreeSpaceExW(L"C:\\", &freeBytes, &totalBytes, &totalFree))
        return 0.0;

    ULONGLONG total = totalBytes.QuadPart;
    ULONGLONG free = totalFree.QuadPart;
    ULONGLONG used = total - free;

    return (double)used * 100.0 / (double)total;
}

bool OpenSerial(const wchar_t* portName) {
    wchar_t fullPort[32];
    swprintf_s(fullPort, L"\\\\.\\%s", portName);

    gSerial = CreateFileW(
        fullPort,
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (gSerial == INVALID_HANDLE_VALUE)
        return false;

    DCB dcb;
    ZeroMemory(&dcb, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(gSerial, &dcb))
        return false;

    dcb.BaudRate = BAUD_RATE;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;

    if (!SetCommState(gSerial, &dcb))
        return false;

    COMMTIMEOUTS timeouts;
    ZeroMemory(&timeouts, sizeof(timeouts));
    timeouts.WriteTotalTimeoutConstant = 500;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    SetCommTimeouts(gSerial, &timeouts);

    return true;
}

void CloseSerial() {
    if (gSerial != INVALID_HANDLE_VALUE) {
        CloseHandle(gSerial);
        gSerial = INVALID_HANDLE_VALUE;
    }
}

bool ConnectAnyPort() {
    const wchar_t* ports[] = {
        L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", L"COM9", L"COM10"
    };

    for (int i = 0; i < sizeof(ports) / sizeof(ports[0]); i++) {
        if (OpenSerial(ports[i])) {
            Sleep(1500);
            return true;
        }
        CloseSerial();
    }

    return false;
}

bool SendLine(const char* line) {
    if (gSerial == INVALID_HANDLE_VALUE)
        return false;

    DWORD written = 0;
    BOOL ok = WriteFile(gSerial, line, (DWORD)strlen(line), &written, NULL);

    return ok && written == strlen(line);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    HANDLE mutex = CreateMutexW(NULL, TRUE, L"Esp32PcMonitorNativeSingleInstance");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    FILETIME idle, kernel, user;
    GetSystemTimes(&idle, &kernel, &user);
    lastIdle = FileTimeToU64(idle);
    lastKernel = FileTimeToU64(kernel);
    lastUser = FileTimeToU64(user);

    while (true) {
        if (gSerial == INVALID_HANDLE_VALUE) {
            if (!ConnectAnyPort()) {
                Sleep(3000);
                continue;
            }
        }

        double cpu = GetCpuPercent();

        double ramPercent, usedGb, totalGb;
        GetRam(&ramPercent, &usedGb, &totalGb);

        double diskC = GetDiskCPercent();
        ULONGLONG uptimeSec = GetTickCount64() / 1000;

        char line[256];
        sprintf_s(
            line,
            "CPU=%.1f,RAM=%.1f,USED=%.1f,TOTAL=%.1f,DISKC=%.1f,UP=0.0,DOWN=0.0,UPTIME=%llu\n",
            cpu,
            ramPercent,
            usedGb,
            totalGb,
            diskC,
            uptimeSec
        );

        if (!SendLine(line)) {
            CloseSerial();
            Sleep(3000);
            continue;
        }

        Sleep(SEND_MS);
    }

    return 0;
}
