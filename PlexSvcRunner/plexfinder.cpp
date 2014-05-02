
#include "stdafx.h"
#include <Msi.h>

#pragma comment (lib, "msi.lib")

const int cchGUID = 38;

// Tries to find the location where plex is installed
int findPlex()
{
	UINT uiStatus = ERROR_SUCCESS;
	DWORD dwIndex = 0;
	WCHAR wszAssignmentType[10] = {0};
	WCHAR wszProductCode[cchGUID+1] = {0};
	DWORD cchAssignmentType = sizeof(wszAssignmentType)/sizeof(wszAssignmentType[0]);
	DWORD cchProductName = MAX_PATH;
    WCHAR* lpProductName = new WCHAR[cchProductName];

	do
	{
		uiStatus = MsiEnumProducts(dwIndex, wszProductCode);

		if(ERROR_SUCCESS == uiStatus){
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

            // output information
            wprintf(L" Product %s:\n", lpProductName);
            wprintf(L"\t%s\n", wszProductCode);
                        wprintf(L"\tInstalled %s %s\n");
		}
	}
    while (ERROR_SUCCESS == uiStatus);
	
	if (lpProductName)
    {
        delete [] lpProductName;
        lpProductName = NULL;
    }

    return (ERROR_NO_MORE_ITEMS == uiStatus) ? ERROR_SUCCESS : uiStatus;
}