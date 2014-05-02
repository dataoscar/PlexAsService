#include "stdafx.h"

// Tries to open up a process, create a thread and load the 
// library
BOOL InjectLibrary(
	HANDLE   hProcess,
	LPCTSTR lpcszLibraryPath,
	DWORD   dwRemoteThreadWait
)
{
	HMODULE hKernel32Module			= NULL; // Module handle of kernel32
	FARPROC pLoadLibrary			= NULL; //
	LPVOID  lpvReservedMemorySpace  = NULL; // Reserved memory space
	SIZE_T  nLibraryPathSize		= 0;	// Size of library path
	SIZE_T  nBytesWriteSize			= 0;	// Size of bytes write
	HANDLE  hRemoteThread			= NULL; // Handle of remote thread

	// Retrieves address of functions which is located in the module kernel32
	if ( !( hKernel32Module = GetModuleHandle( TEXT("kernel32") ) ) )
		 return FALSE; // Return false (0)
	if ( !( pLoadLibrary = GetProcAddress( hKernel32Module, LoadLibraryGeneric ) ) )
		return FALSE; // Return false (0)
	FreeLibrary( hKernel32Module ); //

	// Reserve memory space
	nLibraryPathSize = ( wcslen( lpcszLibraryPath ) + 1) * sizeof( WCHAR );
	if ( !( lpvReservedMemorySpace = VirtualAllocEx( hProcess, NULL, nLibraryPathSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE ) ) )
	{
		return FALSE; // Return false (0)
	}
	// Copy library path into the memory space that we just allocated by calling WriteProcessMemory()
	if ( !WriteProcessMemory( hProcess, lpvReservedMemorySpace, (LPCVOID)lpcszLibraryPath, nLibraryPathSize, &nBytesWriteSize ) )
	{
		// Free virtual and close handle
		VirtualFreeEx( hProcess, lpvReservedMemorySpace, nLibraryPathSize, MEM_RELEASE ); // Free reserved memory with VirtualFreeEx()
		// Return false (0)
		return FALSE;
	}
	// Create remote thread
	if ( !( hRemoteThread = MyCreateRemoteThread( hProcess, (LPTHREAD_START_ROUTINE)pLoadLibrary, lpvReservedMemorySpace) ) )
	{
		// Free virtual and close handle
		VirtualFreeEx( hProcess, lpvReservedMemorySpace, nLibraryPathSize, MEM_RELEASE ); // Free reserved memory with VirtualFreeEx()
		// Return false (0)
		return FALSE;
	}
	// Wait until the thread terminates by calling WaitForSingleObject()
	if ( WaitForSingleObject( hRemoteThread, dwRemoteThreadWait ) == WAIT_FAILED )
	{
		// Free virtual and close handles
		VirtualFreeEx( hProcess, lpvReservedMemorySpace, nLibraryPathSize, MEM_RELEASE ); // Free reserved memory with VirtualFreeEx()
		CloseHandle( hRemoteThread ); // Close remote thread handle
		// Return false (0)
		return FALSE;
	}

	// Free virtual and close handles
	VirtualFreeEx( hProcess, lpvReservedMemorySpace, nLibraryPathSize, MEM_RELEASE ); // Free reserved memory with VirtualFreeEx()
	CloseHandle( hRemoteThread ); // Close remote thread handle
	// Return true (1)
	return TRUE;
}

HANDLE MyCreateRemoteThread(
	HANDLE hProcess,
	LPVOID lpvRemoteThreadStart,
	LPVOID lpvRemoteCallback
)
{
	//
	HANDLE  hReturnHandle   = NULL;
	HMODULE hNtDllModule	= NULL;



	// NtCreateThreadEx function
	if ( ( hNtDllModule = GetModuleHandle( TEXT("ntdll.dll") ) ) )
		if ( GetProcAddress( hNtDllModule, "NtCreateThreadEx" ) )
			if ( ( hReturnHandle = NtCreateThreadEx( hProcess, lpvRemoteThreadStart, lpvRemoteCallback ) ) )
				return hReturnHandle;

	// CreateRemoteThread function
	if ( ( hReturnHandle = CreateRemoteThread( hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpvRemoteThreadStart, lpvRemoteCallback, 0, NULL ) ) )
		return hReturnHandle;

	//
	return NULL;
}



//
HANDLE NtCreateThreadEx(
	HANDLE hProcess,
	LPVOID lpvRemoteThreadStart,
	LPVOID lpvRemoteCallback
)
{
	//
	typedef DWORD (WINAPI * functypeNtCreateThreadEx)(
		PHANDLE				 ThreadHandle,
		ACCESS_MASK			 DesiredAccess,
		LPVOID				  ObjectAttributes,
		HANDLE				  ProcessHandle,
		LPTHREAD_START_ROUTINE  lpStartAddress,
		LPVOID				  lpParameter,
		BOOL					CreateSuspended,
		DWORD				   dwStackSize,
		DWORD				   Unknown1,
		DWORD				   Unknown2,
		LPVOID				  Unknown3
	);



	// Setup & initialization variables
	HANDLE					  hRemoteThread		   = NULL;
	HMODULE					 hNtDllModule			= NULL;
	functypeNtCreateThreadEx	funcNtCreateThreadEx	= NULL;



	//
	if ( ( hNtDllModule = GetModuleHandle( TEXT("ntdll.dll") ) ) )
		if ( !( funcNtCreateThreadEx = (functypeNtCreateThreadEx)GetProcAddress( hNtDllModule, "NtCreateThreadEx" ) ) )
			return NULL;


	//
	funcNtCreateThreadEx( &hRemoteThread,  GENERIC_ALL, NULL, hProcess, (LPTHREAD_START_ROUTINE)lpvRemoteThreadStart, lpvRemoteCallback, FALSE, NULL, NULL, NULL, NULL );

	//
	return hRemoteThread;
}

HANDLE remoteTerminate(HANDLE hProcess)
{
	 return MyCreateRemoteThread( hProcess, (LPTHREAD_START_ROUTINE)ExitProcess, (PVOID)0);
}

BOOL GetLibraryPath(_Out_ LPWSTR szLibraryPath)
{
	WCHAR szBuff[MAX_PATH];

	GetModuleFileName(NULL, szBuff, MAX_PATH);
	std::wstring fullPath(szBuff);
	size_t found = fullPath.find_last_of(L"\\");
	
	if(found == fullPath.npos) 
		return FALSE;

	size_t length = fullPath.copy(szLibraryPath, found, 0);
	szLibraryPath[length] = '\0';
	
	return TRUE;
}