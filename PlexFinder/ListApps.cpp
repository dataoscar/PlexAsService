// PlexFinder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#pragma comment (lib, "msi.lib")

const int cchGUID = 38;
const WCHAR* APP_NAME = TEXT("Plex Media Server.exe");
const int APP_NAME_LEN = wcslen(APP_NAME);

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD chSize = MAX_PATH;
	WCHAR* plexLocation = new WCHAR[chSize];
	
	if(findPlexLocation(plexLocation, &chSize))
	{
		wprintf(L" Location: %s\n", plexLocation);
	}
	delete [] plexLocation;
    return 0;
}


DWORD findPlexLocation(WCHAR* szPlexLocation, DWORD *sizeBuff)
{
	UINT uiStatus = ERROR_SUCCESS;
	DWORD dwIndex = 0;
	WCHAR wszAssignmentType[10] = {0};
	WCHAR wszProductCode[cchGUID+1] = {0};
	WCHAR szSid[128] = L"s-1-1-0";
	DWORD cchSid;
	DWORD cchAssignmentType = sizeof(wszAssignmentType)/sizeof(wszAssignmentType[0]);
	DWORD cchProductName = MAX_PATH;
    WCHAR* lpProductName = new WCHAR[cchProductName];
	MSIINSTALLCONTEXT dwInstalledContext;
	bool foundPlex = false;

	do
	{
		uiStatus = MsiEnumProductsEx(NULL,
									NULL,
									MSIINSTALLCONTEXT_USERMANAGED | MSIINSTALLCONTEXT_USERUNMANAGED | MSIINSTALLCONTEXT_MACHINE,
									dwIndex,
									wszProductCode,
									&dwInstalledContext,
									szSid,
									&cchSid);

		if(ERROR_SUCCESS == uiStatus)
		{
			// obtain the user friendly name of the product
            UINT uiReturn = MsiGetProductInfo(wszProductCode,INSTALLPROPERTY_PRODUCTNAME,lpProductName,&cchProductName);
            if (ERROR_MORE_DATA == uiReturn)
            {
                // try again, but with a larger product name buffer
                delete [] lpProductName;

                // returned character count does not include
                // terminating NULL
                ++cchProductName;

                lpProductName = new WCHAR[cchProductName];
                if (!lpProductName)
                {
                    uiStatus = ERROR_OUTOFMEMORY;
                    break;
                }

                uiReturn = MsiGetProductInfo(wszProductCode,INSTALLPROPERTY_PRODUCTNAME,lpProductName,&cchProductName);
            }

            if (ERROR_SUCCESS != uiReturn)
            {
                // This halts the enumeration and fails. Alternatively the error
                // could be logged and enumeration continued for the
                // remainder of the products
                uiStatus = ERROR_FUNCTION_FAILED;
                break;
            }
			if(wcscmp(lpProductName, L"Plex Media Server") == 0)
			{
				// output information
				wprintf(L" Product %s:\n", lpProductName);
				//Enumerate All Components
				DWORD dwComponentIdx = 0;
				DWORD dwClientIdx = 0;
				DWORD maxPath = MAX_PATH; 
				
				WCHAR lpComponentCode[cchGUID+1] = {0};
				WCHAR* lpClientName = new WCHAR[cchGUID+1];

				WCHAR* lpComponentPath = new WCHAR[maxPath];
				WCHAR* lpComponentSubPath;

				do
				{
					uiStatus = MsiEnumComponents(dwComponentIdx, lpComponentCode);
					
					UINT clientStatus = ERROR_SUCCESS; 
					UINT pathStatus = ERROR_SUCCESS;
					//For each component,  enumerate the clients
					do
					{
						clientStatus = MsiEnumClients(lpComponentCode, dwClientIdx, lpClientName);
						//Only get the path if a matching client is found
						if(clientStatus == ERROR_SUCCESS && 0 == wcscmp(lpClientName, wszProductCode))
						{
							maxPath = MAX_PATH;
							pathStatus = MsiGetComponentPath(wszProductCode, lpComponentCode, lpComponentPath, &maxPath);
							if(pathStatus == INSTALLSTATE_LOCAL && wcslen(lpComponentPath) > 0)
							{
								//We need to compare the end string to ensure it matches the name of the app
								//This just does some pointer arithmetic
								lpComponentSubPath = lpComponentPath + maxPath - APP_NAME_LEN;
								if(0 == wcscmp(APP_NAME, lpComponentSubPath))
								{
									foundPlex = true;
									wcscpy_s(szPlexLocation, *sizeBuff, lpComponentPath);
								}
							}
						}
						
						dwClientIdx++;
					}while(ERROR_SUCCESS == clientStatus && !foundPlex);
					
					dwClientIdx = 0;

					dwComponentIdx++;

				}while(ERROR_SUCCESS == uiStatus && !foundPlex);
				
				delete [] lpClientName;
				delete [] lpComponentPath;
				lpClientName = lpComponentPath = lpComponentSubPath = NULL;

				cchProductName = MAX_PATH;
			}
		}
        dwIndex++;
	}
    while (ERROR_SUCCESS == uiStatus && !foundPlex);
	
	if (lpProductName)
    {
        delete [] lpProductName;
        lpProductName = NULL;
    }

	if(foundPlex && szPlexLocation)
	{
		*sizeBuff = (DWORD)wcslen(szPlexLocation);
		return TRUE;
	}

	return FALSE;
	
}

