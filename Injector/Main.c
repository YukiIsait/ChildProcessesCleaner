#include "InjectHelper.h"
#include <Windows.h>

int wmain(int argc, wchar_t** argv) {
    /*
        Arg1: Command line to execute (The full path of the executable file is required)
        Arg2: The full path of the dynamic library to be injected
    */
    if (argc != 3) {
        MessageBoxW(NULL, L"Parameters failed.", L"Error", MB_ICONERROR);
        return -1;
    }

    STARTUPINFOW mainSi = {0};
    PROCESS_INFORMATION mainPi = {0};
    if (!CreateProcessW(NULL, argv[1], NULL, NULL, FALSE, 0, NULL, NULL, &mainSi, &mainPi)) {
        MessageBoxW(NULL, L"Process creation failed.", L"Error", MB_ICONERROR);
        return -1;
    }

    BOOL ret = TRUE;
    if (!InjectHelper_SetPrivilege(L"SeDebugPrivilege", TRUE)) {
        ret = FALSE;
        TerminateProcess(mainPi.hProcess, 0);
        MessageBoxW(NULL, L"Debug privilege setup failed.", L"Error", MB_ICONERROR);
    }

    if (!InjectHelper_InjectIntoProcess(mainPi.hProcess, argv[2])) {
        ret = FALSE;
        TerminateProcess(mainPi.hProcess, 0);
        MessageBoxW(NULL, L"Injection failed.", L"Error", MB_ICONERROR);
    }

    CloseHandle(mainPi.hProcess);
    CloseHandle(mainPi.hThread);
    return ret;
}
