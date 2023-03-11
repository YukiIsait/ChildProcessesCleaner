#include "InjectHelper.h"
#include <strsafe.h>

BOOL InjectHelper_SetPrivilege(LPCWSTR lpszPrivilege, BOOL bEnablePrivilege) {
	HANDLE hToken;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		return FALSE;
	}
	LUID luid;
	if (!LookupPrivilegeValueW(NULL, lpszPrivilege, &luid)) {
		return FALSE;
	}
	TOKEN_PRIVILEGES tp = {
		.PrivilegeCount = 1,
		.Privileges[0].Luid = luid
	};
	if (bEnablePrivilege) {
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	} else {
		tp.Privileges[0].Attributes = 0;
	}
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		return FALSE;
	}
	return TRUE;
}

BOOL InjectHelper_LoadRemoteLibrary(HANDLE hProcess, LPVOID lpRemoteDllWideNameAddr) {
	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (!kernel32) {
		return FALSE;
	}
	LPTHREAD_START_ROUTINE threadFun = (LPTHREAD_START_ROUTINE) GetProcAddress(kernel32, "LoadLibraryW");
	HANDLE thread = CreateRemoteThread(hProcess, NULL, 0, threadFun, lpRemoteDllWideNameAddr, 0, NULL);
	if (!thread) {
		return FALSE;
	}
	WaitForSingleObject(thread, INFINITE);
	DWORD exitCode = 0;
	GetExitCodeThread(thread, &exitCode); // The 64-bit address is truncated to 32 bits (The 64-bit user program starts at 7FFE0000)
	CloseHandle(thread);
	return !!exitCode; // Convert non-boolean value to 0 or 1
}

BOOL InjectHelper_InjectIntoProcess(HANDLE hProcess, LPCWSTR lpDllName) {
	size_t dllNameCbSize = 0;
	if (StringCbLengthW(lpDllName, MAX_PATH * sizeof(WCHAR), &dllNameCbSize) != S_OK) {
		return FALSE;
	}
	BOOL ret = FALSE;
	LPVOID dllNameRemoteAddr = VirtualAllocEx(hProcess, 0, dllNameCbSize, MEM_COMMIT, PAGE_READWRITE);
	if (dllNameRemoteAddr) {
		size_t written = 0;
		if (WriteProcessMemory(hProcess, dllNameRemoteAddr, lpDllName, dllNameCbSize, &written) &&
			written == dllNameCbSize) {
			ret = InjectHelper_LoadRemoteLibrary(hProcess, dllNameRemoteAddr);
		}
		VirtualFreeEx(hProcess, dllNameRemoteAddr, 0, MEM_RELEASE);
	}
	return ret;
}
