// RemoveWatermark.cpp : Defines the entry point for the console application.
//
/*

The MIT License

Copyright (c) 2009 deepxw

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//////////////////////////////////////////////////////////////////////////////////////////////////
/*

FileName:	RemoveWatermark.cpp

Project:	Remove Watermark (Sample Code)
Author:		deepxw
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com

Comment:	If you are using this code, please let me know.


Change log:

2009-05-09, Created.


*//////////////////////////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "RemoveWatermark.h"

#include <windows.h>
#include <Imagehlp.h>
#include <shlwapi.h>

#pragma comment(lib, "ImageHlp.lib" )
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "shlwapi.lib")

//
// Using the Strsafe.h Functions
//
#define STRSAFE_LIB					// use the functions in library form
#include <strsafe.h>
//

//
// Variables
//

HANDLE		m_hConsole;				// Handle of console



//
// Functions
//

// Get the PE file version info
BOOL GetDllFileVersion(LPCTSTR lpszFileName, PMYVERSIONINFO pVersionInfo);

// Get watermark string from user32.dll.mui
BOOL GetWatermarkFromMuiFile(LPTSTR pszFile);

// Load string from resource with special langID
BOOL LoadStringExx(
					 HINSTANCE hInst,			// Hinstance of lib
					 WORD wLangID,				// Language ID of resource
					 PRES_STRING_INFO pInfo		// Pointer to the string info
					 );

// Zero watermark string From Mui File
BOOL ZeroWatermarkFromMuiFile(
							  LPCTSTR pszFile,						// The MUI file name
							  PSINGLE_LIST_ENTRY pStringsHead		// Pointer to the string info
							  );

// Moves the cursor to x, y in console window
void gotoX(SHORT x);


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Main entry
//
int _tmain(int argc, _TCHAR* argv[])
{
	SetConsoleTitle(_T("Remove Watermark Demo  (Code by deepxw)"));

	_tcprintf(_T("Notice:\n"));
	_tcprintf(_T("  This program is not a full function tool, it's only a demo for programer!\n\n"));

	// get handle of current console
	m_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get Watermark string
	// GetWatermarkFromMuiFile(_T("b:\\user32.dll.mui"));

	//
	// Check args
	//

	if (argc == 2)
	{
		// Get Watermark string
		GetWatermarkFromMuiFile(argv[1]);
	}
	else
	{
		_tcprintf(_T("Usage:\n"));
		_tcprintf(_T("  RemoveWatermark   MuiFileName\n"));
	}

	getchar();

	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Get watermark string from user32.dll.mui
/*
715, 	"%wsWindows %ws"
716, 	"%ws Build %ws"
717, 	"Evaluation copy."
718, 	"For testing purposes only."
723, 	"%wsMicrosoft (R) Windows (R) (Build %ws: %ws)"
737, 	"This copy of Windows is not genuine"
738, 	"Test Mode"
*/
BOOL GetWatermarkFromMuiFile(LPTSTR pszFile)
{
	_tcprintf(_T("File name:\t%s\n"), pszFile);

	if (!PathFileExists(pszFile))
	{
		_tcprintf(_T("File not found!\n"));
		return FALSE;
	}


	//
	// Check file version, we need to get the language ID of the mui file.
	//

	MYVERSIONINFO	vi;

	ZeroMemory(&vi, sizeof(MYVERSIONINFO));
	vi.dwSize = sizeof(MYVERSIONINFO);
	if (!GetDllFileVersion(pszFile, &vi))
	{
		_tcprintf(_T("Fail to get file version info!\n")); 
		return FALSE;
	}

	_tcprintf(_T("File version:\t%s\n"), vi.szShortVersion);


	//
	// Load mui file to memory
	//

	HINSTANCE		hInstLib  = NULL;

	hInstLib = LoadLibraryEx(pszFile, NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
	if( NULL == hInstLib )
	{
		_tcprintf(_T("Fail to open file user32.dll.mui!\n")); 
		return FALSE;
	}


	//
	// Get file type
	//

	PIMAGE_DOS_HEADER	pDOSHeader	= (PIMAGE_DOS_HEADER)((DWORD_PTR)hInstLib - 1);
	PIMAGE_NT_HEADERS	pNTHeader	= (PIMAGE_NT_HEADERS) (pDOSHeader->e_lfanew + (DWORD_PTR)pDOSHeader);
	
	_tcprintf(_T("File type:\t"));
	switch (pNTHeader->FileHeader.Machine)
	{
	case IMAGE_FILE_MACHINE_I386:
		_tcprintf(_T("x86"));
		break;
	case IMAGE_FILE_MACHINE_AMD64:
		_tcprintf(_T("x64"));
		break;
	case IMAGE_FILE_MACHINE_IA64:
		_tcprintf(_T("ia64"));
		break;

	default:
		_tcprintf(_T("Unknown\nThis is not a valid file.\n"));

		FreeLibrary(hInstLib);
		return FALSE;
	}

	UINT		uStringID;
	UINT		uStringIDS[]	= {715, 716, 717, 718, 738, 723, 737};
	BOOL		bHasPatched		= FALSE;
	UINT		i				= 0;
	UINT		uMatchingString	= 0;

	// Create string info lists
	SINGLE_LIST_ENTRY	StringsHead;
	PSINGLE_LIST_ENTRY	psLink;
	PRES_STRING_INFO	pStrInfo;

	StringsHead.Next = NULL;

	_tcprintf(_T("\n\n   ID  String                                                Offset  Len Mod"));
	_tcprintf(  _T("\n-----  ----------------------------------------------------  ------  --- ---"));

	for (i=0; i < sizeof(uStringIDS)/sizeof(UINT); i++)
	{
		// Add a entry
		pStrInfo	= (PRES_STRING_INFO)MALLOC(sizeof(RES_STRING_INFO));
		ZeroMemory(pStrInfo, sizeof(RES_STRING_INFO));
		
		pStrInfo->uStringID	= uStringIDS[i];

		LoadStringExx(hInstLib, (WORD)vi.wLangID, pStrInfo);

		if (lstrlen(pStrInfo->pszText) > 0)
		{
			uMatchingString++;
		}

		_tcprintf(_T("\n%5d  %s"), pStrInfo->uStringID, pStrInfo->pszText);
		gotoX(61);
		_tcprintf(_T("0x%4X  %3d"), pStrInfo->dwFileOffset, pStrInfo->dwBytes);

		PushEntryList(&StringsHead, &(pStrInfo->link));

	} // for(i)


	// importance
	FreeLibrary(hInstLib);

	_tcprintf(_T("\n\n"));


	if ( (uMatchingString > 0) && (StringsHead.Next != NULL) )
	{
		_tcprintf(_T("Do you want to patch this file?\n"));
		_tcprintf(_T(" (Y=Yes  /  N=No)\n:"));

		int	iChoice	= getchar();

		if ( (iChoice == _T('y')) || (iChoice == _T('Y')) )
		{
			TCHAR	szFileBackup[MAX_PATH];

			StringCbCopy(szFileBackup, sizeof(szFileBackup), pszFile);
			StringCbCat(szFileBackup, sizeof(szFileBackup), _T(".backup"));

			// make a backup
			CopyFile(pszFile, szFileBackup, FALSE);

			// In real life, if you want to patch \windows\system32\en-us\user32.dll.mui,
			// because the file is in using, you must copy a temp file to do ZeroWatermarkFromMuiFile().
			// Last, using MoveFileEx() to replace the file.

			if (ZeroWatermarkFromMuiFile(pszFile, &StringsHead))
			{
				_tcprintf(_T("\nPatch OK!\n"));
			}
			else
			{
				_tcprintf(_T("\nFail to patch.\n"));
			}


		} // choice y
	}
	else
	{
		_tcprintf(_T("Watermark string is not found, no need to patch.\n"));
	}


	//
	// Removes all string infos, free memory
	//
	psLink	= PopEntryList(&StringsHead);
	while(psLink)
	{
		pStrInfo	= CONTAINING_RECORD(psLink, RES_STRING_INFO, link);

		// free memory
		if (pStrInfo->pszText)
			FREE((LPVOID)(pStrInfo->pszText));

		FREE((LPVOID)pStrInfo);

		// Removes the first entry
		psLink	= PopEntryList(&StringsHead);

	} // while(psLink)


	return TRUE;

} // GetWatermarkFromMuiFile


//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Load string from resource with special langID
//
BOOL LoadStringExx(
					 HINSTANCE hInst,			// Hinstance of lib
					 WORD wLangID,				// Language ID of resource
					 PRES_STRING_INFO pInfo		// Pointer to the string info
					 )

{
	HRSRC			hFindRes;		// Handle of the resources has been found
	HGLOBAL			hLoadRes;		// Handle of the resources has been loaded
	LPVOID			pRes;			// Pointer to the resources
	UINT			nBlockID;		// String block ID

	pInfo->dwFileOffset	= 0;		// String offset in the file
	pInfo->dwBytes		= 0;		// String length, in bytes
	pInfo->pszText		= NULL;

	nBlockID = pInfo->uStringID / 16 + 1;

	__try
	{
		// find the string block
		hFindRes = FindResourceEx(hInst, RT_STRING, MAKEINTRESOURCE(nBlockID), wLangID);
		if(!hFindRes )
		{
			__leave;
		}	

		hLoadRes = LoadResource(hInst, hFindRes);
		if(!hLoadRes )
		{
			__leave;
		}

		pRes = LockResource(hLoadRes);
		if(!pRes )
		{
			__leave;
		}

		WCHAR*		pParse		= (WCHAR *)pRes;				// Pointer to the String block
		UINT		nIndex		= pInfo->uStringID % 16;		// Calculate the string index
		int			nLen;
		UINT		i;

		// 16 strings per block
		for( i = 0; i < (nIndex & 15); i++ )
		{
			pParse += 1 + (int)*pParse;
		}

		// OK, we get it
		nLen = (UINT)*pParse;		// The length of the target string.
		pParse += 1;				// Pointer to the target string


		// Main point, calculate the string offset
		pInfo->dwFileOffset	= (DWORD) ( (DWORD_PTR)pParse - (DWORD_PTR)hInst ) + 1;
		pInfo->dwBytes		= nLen * sizeof(WCHAR);

		// allocate memory
		pInfo->pszText = (LPWSTR)MALLOC((nLen + 1) * sizeof(WCHAR));
		if (!pInfo->pszText)
			__leave;

		// copy string for return
		CopyMemory((LPVOID)pInfo->pszText, (LPVOID)pParse, pInfo->dwBytes);
		*(PWCHAR)((DWORD_PTR)pInfo->pszText + pInfo->dwBytes) = 0;


		//TRACEF(_T("String ID: %5d \t%s"), pszText);

	}
	__finally
	{
		// Clean up, free memory

		if (pRes)
			UnlockResource(pRes);

		if (hFindRes)
			FreeResource(hFindRes);
	}

	// if pointer is null, we return a NULL string
	if (!pInfo->pszText)
	{
		pInfo->pszText		= (LPWSTR)MALLOC(sizeof(WCHAR));
		pInfo->pszText[0]	= 0;
	}

	return TRUE;

} // LoadStringExx()


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Zero watermark string From Mui File
//
// In order to make the procedure simple, so use the simplest method.
//
BOOL ZeroWatermarkFromMuiFile(
							  LPCTSTR pszFile,						// The MUI file name
							  PSINGLE_LIST_ENTRY pStringsHead		// Pointer to the string info
							  )
{
	BOOL				bRet			= FALSE;

	HANDLE				hFile			= NULL;
	HANDLE				hMapping		= NULL;
	PUCHAR				pView			= NULL;
	PIMAGE_NT_HEADERS	pNTHeader		= NULL;
	PIMAGE_DOS_HEADER	pDOSHeader		= NULL;
	DWORD				dwHeaderSum		= 0;
	DWORD				dwCorrectSum	= 0;
	DWORD				dwFileSize		= 0;

	__try
	{
		// Open file
		hFile	= CreateFile(pszFile, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
		if (INVALID_HANDLE_VALUE == hFile)
			__leave;

		dwFileSize	= GetFileSize(hFile, NULL);
		
		// Create mapping
		hMapping	= CreateFileMapping(hFile, 0, PAGE_READWRITE, 0, 0, NULL);
		if (!hMapping)
			__leave;

		// MapView of the PE file
		pView	= (PUCHAR)MapViewOfFile(hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (!pView)
			__leave;

		// Make sure it's a valid PE file
		pDOSHeader	= (PIMAGE_DOS_HEADER)pView;
		pNTHeader	= (PIMAGE_NT_HEADERS)(pDOSHeader->e_lfanew + (ULONG_PTR)pDOSHeader);		
		if(pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE || pNTHeader->Signature != IMAGE_NT_SIGNATURE)
			__leave;


		//
		// OK, Ready to Zero the watermark string
		//

		PRES_STRING_INFO	pStrInfo;
		PSINGLE_LIST_ENTRY	psLink		= pStringsHead->Next;

		// Enumerate all entrys
		while(psLink)
		{
			// Get the string info
			pStrInfo	= CONTAINING_RECORD(psLink, RES_STRING_INFO, link);

			// Make sure the info is correct
			if ( (pStrInfo->dwFileOffset > 0x200)
				&& (pStrInfo->dwFileOffset < dwFileSize)
				&& (pStrInfo->dwBytes > 0) )
			{
				// Real work, zero the string
				ZeroMemory((LPVOID)((ULONG_PTR)pView + pStrInfo->dwFileOffset), pStrInfo->dwBytes);
			}

			// Get next entry
			psLink	= psLink->Next;

		} // while(psLink)
			

		//
		// OK, we are try to fix the checksum of this file
		//

		// Get correct checksum of the PE file
		pNTHeader = CheckSumMappedFile((LPVOID)pView, dwFileSize, &dwHeaderSum, &dwCorrectSum);

		//TRACEF(_T("Header checksum: %08X \tCorrect checksum: %0X\n"), dwHeaderSum, dwCorrectSum);

		if (!pNTHeader)
		{
			//TRACEF(_T("Fail to re-checksum.\n"));
			__leave;
		}

		// Update the correct checksum to the file header
		pNTHeader->OptionalHeader.CheckSum = dwCorrectSum;

		//
		// All done, OK!
		//
		bRet	= TRUE;

	} // end try
	__finally
	{
		//
		// Clean up
		//

		if (pView)
			UnmapViewOfFile((LPCVOID)pView);

		if (hMapping)
		{
			if (!CloseHandle(hMapping))
				bRet	= FALSE;
		}

		if (hFile)
		{
			if (!CloseHandle(hFile))
				bRet	= FALSE;
		}

	} //__finally

	return bRet;

} // ZeroWatermarkFromMuiFile()


//////////////////////////////////////////////////////////////////////////////////////////////////
// Get the PE file version info
// Last modified, 
// 2008.12.24, added bOK
//
BOOL GetDllFileVersion(LPCTSTR pszFileName, PMYVERSIONINFO pVersionInfo) 
{
	// Check struct size
	if (sizeof(MYVERSIONINFO) != pVersionInfo->dwSize)
		return FALSE;

	BOOL	bOK			= FALSE;
	DWORD   dwHandle	= NULL;   
	DWORD   dwVerSize;
	
	// Get the file version info size
	dwVerSize = GetFileVersionInfoSize(pszFileName, &dwHandle);
	if(dwVerSize == 0)
		return FALSE;

	LPVOID				pbuf		= NULL; 
	UINT				uLen		= 0;   
	VS_FIXEDFILEINFO	*pFileInfo;   

	pbuf = MALLOC(dwVerSize);
	if(!pbuf)
		return FALSE;
  
	__try
	{
		bOK = GetFileVersionInfo(pszFileName, dwHandle, dwVerSize, pbuf);
		if (!bOK)
			__leave;

		bOK	= VerQueryValue(pbuf, (LPTSTR)("\\"), (LPVOID*)&pFileInfo, &uLen);   
		if (!bOK)
			__leave;

		// get data
		pVersionInfo->wMajorVersion		= HIWORD(pFileInfo->dwProductVersionMS);     
		pVersionInfo->wMinorVersion		= LOWORD(pFileInfo->dwProductVersionMS);   
		pVersionInfo->wBuildNumber		= HIWORD(pFileInfo->dwProductVersionLS);   
		pVersionInfo->wRevisionNumber	= LOWORD(pFileInfo->dwProductVersionLS);

		StringCbPrintf(pVersionInfo->szShortVersion, 
						sizeof(pVersionInfo->szShortVersion), _T("%u.%u.%u.%u"), \
						pVersionInfo->wMajorVersion, pVersionInfo->wMinorVersion, \
						pVersionInfo->wBuildNumber, pVersionInfo->wRevisionNumber
						);


		bOK		= TRUE;


	}
	__finally
	{
		// clean up
		
		if (pbuf)
			FREE(pbuf);

	}

	return   bOK;

} // GetDllFileVersion()



//////////////////////////////////////////////////////////////////////////////////////////////////
//
// Moves the cursor to x, y in console window
// ie x=left\right y=top\bottom
//
void gotoX(SHORT x)
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo; 

	// Get the current screen buffer size and window position. 
	if (!GetConsoleScreenBufferInfo(m_hConsole, &csbiInfo)) 
	{
		return;
	}

	COORD	point;

	point.X = x;
	point.Y = csbiInfo.dwCursorPosition.Y;
	SetConsoleCursorPosition(m_hConsole, point);
}