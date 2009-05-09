/*

FileName:	RemoveWatermark.h

Project:	Remove Watermark (Sample Code)
Author:		deepxw
E-mail:		deepxw#gmail.com
Blog:		http://deepxw.blogspot.com

Comment:	If you are using this code, please let me know.


Change log:

2009-05-09, Created.


*//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __RemoveWatermark_H__
#define __RemoveWatermark_H__
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include <windows.h>
#include <tchar.h>

#define		MALLOC(x)	HeapAlloc(GetProcessHeap(), 0, (x))
#define		FREE(x)		HeapFree(GetProcessHeap(), 0, (x))

FORCEINLINE
PSINGLE_LIST_ENTRY
PopEntryList(
    PSINGLE_LIST_ENTRY ListHead
    )
{
    PSINGLE_LIST_ENTRY FirstEntry;
    FirstEntry = ListHead->Next;
    if (FirstEntry != NULL) {
        ListHead->Next = FirstEntry->Next;
    }

    return FirstEntry;
}


FORCEINLINE
VOID
PushEntryList(
    PSINGLE_LIST_ENTRY ListHead,
    PSINGLE_LIST_ENTRY Entry
    )
{
    Entry->Next = ListHead->Next;
    ListHead->Next = Entry;
}


// Struct for string list
typedef struct _RES_STRING_INFO
{
	UINT	uStringID;						// String ID
	DWORD	dwFileOffset;					// File Offset of the string
	DWORD	dwBytes;						// Size of the string

	LPWSTR	pszText;						// String

	SINGLE_LIST_ENTRY	link;				// list link
} RES_STRING_INFO, *PRES_STRING_INFO;


// Struct for PE file version info
typedef struct _MYVERSIONINFO
{
	DWORD	dwSize;							// struct size

	WORD	wMajorVersion;					// Major version number
	WORD	wMinorVersion;					// Minor version number
	WORD	wBuildNumber;					// Build number
	WORD	wRevisionNumber;				// Revision number

	WORD	wLangID;						// Language ID;

	TCHAR	szShortVersion[32];				// File version string, likes "1.0.2.2000"
	TCHAR	szOriginalFilename[64];			// Original file name
} MYVERSIONINFO, *PMYVERSIONINFO;








//////////////////////////////////////////////////////////////////////////////////////////////////

#endif