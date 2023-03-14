#include <Windows.h>
#include <TlHelp32.h>

BOOL TerminateSubProcesses() {
    DWORD currentProcessId = GetCurrentProcessId();
    PROCESSENTRY32W processEntry = {
        .dwSize = sizeof(PROCESSENTRY32W)
    };
    HANDLE processSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processSnapshot == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    BOOL exists = Process32FirstW(processSnapshot, &processEntry);
    while (exists) {
        if (processEntry.th32ParentProcessID == currentProcessId) {
            HANDLE subProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
            if (subProcess) {
                TerminateProcess(subProcess, 0);
                CloseHandle(subProcess);
            }
        }
        exists = Process32NextW(processSnapshot, &processEntry);
    }
    CloseHandle(processSnapshot);
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwUlReasonForCall, LPVOID lpReserved) {
    switch (dwUlReasonForCall) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            TerminateSubProcesses();
            break;
    }
    return TRUE;
}
