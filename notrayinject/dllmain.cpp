#include "stdafx.h"
#include "notrayinject.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	// Get rid of compiler warnings since we do not use this parameter
    UNREFERENCED_PARAMETER(lpReserved);
	// If we are attaching to a process
    switch(ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
		OutputDebugString(L"notrayinject in a thread");
		inject();
		break;
    }
	
	// Signal for Loading/Unloading
    return (TRUE);
}