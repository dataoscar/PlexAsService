// Entry point for console based service
#include "stdafx.h"
#include "messages.h"

PROCESS_INFORMATION pi;
SERVICE_STATUS          gSvcStatus; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
HANDLE                  ghSvcStopEvent = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
	// TO_DO: Add any additional services for the process to this table.
    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 
 
    // This call returns when the service has stopped. 
    // The process should simply terminate when the call returns.
    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        //SvcReportEvent(TEXT("StartServiceCtrlDispatcher")); 
		if(GetLastError() == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			SvcInit( argc, argv );
		}
    } 

	
}

void getModulePath(LPWSTR libName, LPWSTR path)
{
	HMODULE hModule = GetModuleHandleW(NULL);
	GetModuleFileNameW(hModule, path, MAX_PATH);
}

BOOL CtrlHandler( DWORD fdwCtrlType ) 
{ 
  switch( fdwCtrlType ) 
  { 
    case CTRL_C_EVENT: 
    case CTRL_CLOSE_EVENT: 
    case CTRL_BREAK_EVENT: 
    case CTRL_LOGOFF_EVENT: 
    case CTRL_SHUTDOWN_EVENT: 
		closePlexChild();
		return FALSE; 
 
    default: 
      return FALSE; 
  } 
}

// Purpose: 
//   Best effort to close the Plex Server process.
//   It first tries to message Plex Server to quit gracefully and waits 15 seconds for Plex to exit
//   If Plex does not exit, it will terminate the process.
void closePlexChild()
{
	HWND hwnd = FindWindow(L"PlexMediaServer_TrayIconClass", L"");
	if(hwnd)
	{
		PostMessage(hwnd, WM_COMMAND, 40004, 0);
		WaitForSingleObject(pi.hProcess, 15000); // Wait 15 seconds
	}
	PROCESSENTRY32 pe;
	ZeroMemory(&pe, sizeof(pe));
	pe.dwSize = sizeof(pe);

	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	std::list<HANDLE> children;
	if(Process32First(snap, &pe))
	{
		BOOL bContinue = TRUE;
		while(bContinue)
		{
			if(pe.th32ParentProcessID == pi.dwProcessId)
			{
				HANDLE hChild = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
				wprintf(L"pid %d %s\n", pe.th32ProcessID, pe.szExeFile);
				if(hChild)
				{
					children.push_front(hChild);
				}
			}
			bContinue = Process32Next(snap, &pe);
		}
	}
	TerminateProcess(pi.hProcess, 1);
	for(std::list<HANDLE>::iterator i = children.begin() ; i != children.end(); ++i)
	{
		HANDLE hChild = *i;
		TerminateProcess(hChild,0);
		CloseHandle(hChild);
	}
	children.clear();
}


//
// Purpose: 
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None.
//
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandler( 
        SVCNAME, 
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
    { 
        //SvcReportEvent(TEXT("RegisterServiceCtrlHandler")); 
        return; 
    } 

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    // Report initial status to the SCM

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit( dwArgc, lpszArgv );
}

//
// Purpose: 
//   Service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
// 
// Return value:
//   None
//
VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    //   Be sure to periodically call ReportSvcStatus() with 
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.

	HANDLE ghEvents[2];

    ghSvcStopEvent = CreateEvent(
                         NULL,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         NULL);   // no name

    if ( ghSvcStopEvent == NULL)
    {
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }
	install_message_source();

    // Report running status when initialization is complete.
	ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
	STARTUPINFO si;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));

	si.cb = sizeof(si);

	WCHAR plexApp[MAX_PATH] =  TEXT("C:\\Program Files (x86)\\Plex\\Plex Media Server\\Plex Media Server.exe");
	WCHAR libraryPath[MAX_PATH];

	if(!CreateProcess(NULL,
		plexApp,
		NULL, //SECURITY ATTRIBUTES
		NULL, //THREAD ATTRIBUTES
		FALSE, //INHERIT HANDLES
		NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED,
		NULL, //ENVIRONMENT
		NULL, //CURRENT DIRECTORY
		&si, //STARTUP INFO
		&pi //PROCESS INFO
		))
	{
		printf( "CreateProcess failed (%d).\n", GetLastError() );
		ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
		return;
	}

	if(! GetLibraryPath(libraryPath))
	{
		ReportSvcStatus( SERVICE_STOPPED, ERROR_FILE_NOT_FOUND, 0 );
		return;
	}

	HRESULT result = StringCchCat(libraryPath, MAX_PATH, L"\\notrayinject.dll");
	if(result != S_OK)
	{
		ReportSvcStatus( SERVICE_STOPPED, ERROR_FILE_NOT_FOUND, 0 );
		return;
	}
	OutputDebugString(libraryPath);

	if(! InjectLibrary(pi.hProcess, libraryPath, INFINITE))
	{
		printf( "Inject Failed (%d).\n", GetLastError() );
		ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
		return;
	}
	OutputDebugString(L"finish loading thread");
	ResumeThread(pi.hThread);
	
	ghEvents[0] = ghSvcStopEvent;
	ghEvents[1] = pi.hProcess;


	while(1)
    {
        WaitForMultipleObjects( 
			2,           // number of objects in array
			ghEvents,     // array of objects
			FALSE,       // wait for any object
			INFINITE);       // forever

        break;
    }
	closePlexChild();
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
}

//
// Purpose: 
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation, 
//     in milliseconds
// 
// Return value:
//   None
//
VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

//
// Purpose: 
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
// 
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

         // Signal the service to stop.

         SetEvent(ghSvcStopEvent);
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         
         return;
 
      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   } 
   
}


