/*
This file is part of Second Life Avatar Probe (SLAP)

Copyright (C) 2015 Nikolay Raspopov <raspopov@cherubicsoft.com>

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see < http://www.gnu.org/licenses/>.
*/

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#define WIN32_LEAN_AND_MEAN
#define NO_PRINT

#define _ATL_FREE_THREADED
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_CSTRING_NO_CRT
#define _ATL_NO_COM_SUPPORT

#define _AFX_ALL_WARNINGS

#include "targetver.h"

#include "resource.h"

#include <afxwin.h>				// MFC core and standard components
#include <afxext.h>				// MFC extensions
#include <afxtempl.h>			// CMap<>
#include <afxinet.h>			// MFC internet extensions
#include <afxmt.h>				// MFC multi-threaded extensions
#include <wincred.h>			// CredRead, CredWrite

#include <shlobj.h>				// BIF_RETURNONLYFSDIRS
#pragma comment(lib, "shell32.lib")

#include <winver.h>
#pragma comment(lib, "version.lib")

#include <mmsystem.h>			// PlaySound
#pragma comment(lib, "winmm.lib")

#include <afxeditbrowsectrl.h>	// CMFCEditBrowseCtrl

#include <atlimage.h>			// CImage

#include "winsparkle.h"

template<>
AFX_INLINE UINT AFXAPI HashKey(const CStringW& key)
{
	UINT nHash = 0;
	const wchar_t* pszKey = key;
	for ( int nSize = key.GetLength(); nSize; ++pszKey, --nSize )
	{
		nHash = ( nHash << 5 ) + nHash + *pszKey;
	}
	return nHash;
}

template<>
AFX_INLINE UINT AFXAPI HashKey(const CStringA& key)
{
	UINT nHash = 0;
	const char* pszKey = key;
	for ( int nSize = key.GetLength(); nSize; ++pszKey, --nSize )
	{
		nHash = ( nHash << 5 ) + nHash + *pszKey;
	}
	return nHash;
}

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


