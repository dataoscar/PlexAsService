// notrayinject.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "notrayinject.h"


//HookList g_hookList;
SHookThunk g_notify;

// This is an example of an exported function.
NOTRAYINJECT_API void inject(void)
{
	HookAPIFunction(L"shell32.dll", "Shell_NotifyIconW", MyShell_NotifyIcon, g_notify);
}

BOOL WINAPI MyShell_NotifyIcon(
  _In_  DWORD dwMessage,
  _In_  PNOTIFYICONDATA lpdata
)
{
	OutputDebugString(L"Shell_NotifyIcon");
	return TRUE;
}

BOOL HookAPIFunction(LPCWSTR lpModuleName,LPCSTR lpFunctionName,LPCVOID lpNewFunction,SHookThunk & htOldFunctionCode)
{
	//Get the library handle if loaded into this process.
	HANDLE  hLib= GetModuleHandle(lpModuleName);
	if(!hLib)
		return FALSE;
	//Get the function address if exported from module.
	DWORD dwAddr =  (DWORD)GetProcAddress((HMODULE)hLib,lpFunctionName);
	if(!dwAddr)
		return FALSE;

	SHookThunk ht;
	HookThunkInit(ht,((DWORD)lpNewFunction - dwAddr - 5));

	//Backup old code
	if(!ReadProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, &htOldFunctionCode, sizeof(SHookThunk), NULL))
		return FALSE;
	//Write  new code
	if(WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, &ht, sizeof(SHookThunk), NULL))
		return TRUE;
	return FALSE;;
}
BOOL UnhookAPIFunction(LPCWSTR lpModuleName,LPCSTR lpFunctionName, SHookThunk & htOldFunctionCode)
{
	//Get the library handle if loaded into this process.
	HANDLE  hLib= GetModuleHandle(lpModuleName);
	if(!hLib)
		return FALSE;
	//Get the function address if exported from module.
	DWORD dwAddr =  (DWORD)GetProcAddress((HMODULE)hLib,lpFunctionName);
	if(!dwAddr)
		return FALSE;
	
	// Restore original code of function.
	if (WriteProcessMemory(GetCurrentProcess(), (LPVOID)dwAddr, &htOldFunctionCode,sizeof(SHookThunk), NULL))
		return TRUE;
	return FALSE;
}
VOID HookThunkInit(SHookThunk & ht,DWORD dwAddr)
{
	ht.jmp = 0xe9;
	ht.addr = dwAddr;
	ht.ret = 0xc3;
}