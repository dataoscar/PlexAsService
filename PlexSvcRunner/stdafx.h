// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>


#include <windows.h>
#include <TlHelp32.h>
#include <string>
#include <list>
#include <strsafe.h>

#pragma comment(lib, "advapi32.lib")


#ifdef UNICODE
#define LoadLibraryGeneric  "LoadLibraryW"
#else
#define LoadLibraryGeneric  "LoadLibraryA"
#endif

void getModulePath(LPWSTR,LPWSTR);
int InjectLibrary(HANDLE,LPCTSTR,DWORD);
BOOL GetLibraryPath(LPWSTR);
HANDLE NtCreateThreadEx(HANDLE ,LPVOID ,LPVOID );
HANDLE MyCreateRemoteThread(HANDLE ,LPVOID ,LPVOID );
HANDLE remoteTerminate(HANDLE hProcess);
BOOL CtrlHandler( DWORD fdwCtrlType );
void closePlexChild();
BOOL CALLBACK EnumWindowsCallback(HWND hwnd,LPARAM lParam);

//Service defines
#define SVCNAME TEXT("PlexSvcLoader")
#define SVCDESCRIPTION TEXT("Plex Application Service Loader")
VOID SvcInstall(void);
VOID WINAPI SvcCtrlHandler( DWORD ); 
VOID WINAPI SvcMain( DWORD, LPTSTR * ); 
VOID ReportSvcStatus( DWORD, DWORD, DWORD );
VOID SvcInit( DWORD, LPTSTR * ); 
VOID SvcReportEvent( LPTSTR );

//Event Log Defines
DWORD install_message_source();
DWORD remove_message_source();