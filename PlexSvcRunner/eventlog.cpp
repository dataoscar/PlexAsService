#include "stdafx.h"
#include "messages.h"

DWORD install_message_source()
{
	WCHAR szBuff[MAX_PATH] ;
	DWORD lastError;

	wprintf(szBuff, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"), SVCNAME);

	HKEY key;
	lastError = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
				   szBuff,
				   0,
				   0,
				   REG_OPTION_NON_VOLATILE,
				   KEY_SET_VALUE,
				   0,
				   &key,
				   0);
	
	if(lastError == 0)
	{
		GetModuleFileName( NULL, szBuff, MAX_PATH );
		lastError = RegCreateKey( HKEY_LOCAL_MACHINE, szBuff, &key );
		DWORD dwTypes = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE; 
		lastError = RegSetValueEx( key, 
								   TEXT("TypesSupported"), 
								   0, 
								   REG_DWORD, 
								   (LPBYTE) &dwTypes, 
								   sizeof dwTypes );
		RegCloseKey( key );
	}
	
	return lastError;
}

DWORD remove_message_source()
{
  WCHAR szBuff[ MAX_PATH ];

  wprintf( szBuff, TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s"), SVCNAME );
  return RegDeleteKey( HKEY_LOCAL_MACHINE, szBuff );
}

//
// Purpose: 
//   Logs messages to the event log
//
// Parameters:
//   szFunction - name of function that failed
// 
// Return value:
//   None
//
// Remarks:
//   The service must have an entry in the Application event log.
//
VOID SvcReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if( NULL != hEventSource )
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    SVC_ERROR,           // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}