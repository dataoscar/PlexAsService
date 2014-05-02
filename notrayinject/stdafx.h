// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <shellapi.h>
BOOL APIENTRY DllMain( HMODULE, DWORD, LPVOID);


// TODO: reference additional headers your program requires here
#include <list>

#define PARAMS(_c) (sizeof(void*)*_c)
#define ARRAY_LEN(_array) (sizeof(_array)/sizeof(_array[0]))

#pragma pack(push,1)
struct SHookThunk{
	BYTE jmp;
	DWORD addr;
	BYTE  ret;
};
#pragma pack(pop)

typedef struct
{
   LPCWSTR module;
   LPCSTR functionName;
   USHORT pmSize;
   LPCVOID handler;
   int flags;
}HookDesc;

VOID HookThunkInit(SHookThunk &,DWORD);
BOOL HookAPIFunction(LPCWSTR, LPCSTR, LPCVOID, SHookThunk &);
BOOL UnhookAPIFunction(LPCWSTR, LPCSTR, SHookThunk &);
BOOL WINAPI MyShell_NotifyIcon(DWORD, PNOTIFYICONDATA);