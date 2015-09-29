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

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif


#define UPDATE_PERIOD_DEFAULT	60
#define UPDATE_PERIOD_MIN		30
#define UPDATE_PERIOD_MAX		3600

#define DEAD_TIME_DEFAULT		300
#define DEAD_TIME_MIN			30
#define DEAD_TIME_MAX			3600

#define CACHE_TIME				48		// Avatar image cache time (hours)

#define SETTINGS			_T("Settings")
#define SHOW_ONLINE_ONLY	_T("ShowOnlineOnly")
#define DEBUGLOG			_T("DebugLog")
#define UPDATE_PERIOD		_T("UpdatePeriod")
#define DEAD_TIME			_T("DeadTime")
#define WINDOW_X			_T("X")
#define WINDOW_Y			_T("Y")
#define WINDOW_WIDTH		_T("Width")
#define WINDOW_HEIGHT		_T("Height")
#define ONLINE_TRAY			_T("OnlineTray")
#define OFFLINE_TRAY		_T("OfflineTray")
#define AVATARS				_T("Avatars")
#define PLACE				_T("Place")
#define ONLINE				_T("Online")
#define FRIEND				_T("Friend")
#define LAST_ONLINE			_T("LastOnline")
#define SOUND_ONLINE		_T("SoundOnline")
#define SOUND_OFFLINE		_T("SoundOffline")
#define TIMELINE			_T("Timeline")
#define LOOP_SOUND			_T("LoopSound")
#define IMAGE_TIME			_T("ImageTime")
#define COOKIES				_T("Cookies")
#define NO_SOUND			_T("<SOUND_OFF>")

#define WM_APP_REFRESH		(WM_APP + 1)
#define WM_APP_NOTIFY		(WM_APP + 2)


#include "Avatar.h"


class CSLAPApp : public CWinApp, CCommandLineInfo
{
public:
	CSLAPApp();

	CString sFullTitle;				// Full title of application
	CString	sVersion;				// Application version ("X.X.X.X")
	CString sCopyright;				// Copyright information
	CString sModuleFileName;		// Full path of application exe-file
	CString sImageCache;			// Path to image cache
	CString sDesktop;				// Path to user Desktop
	CString sStartup;				// Path to startup folder
	BOOL	m_bDebugLog;			// Enabled debug log
	BOOL	m_bTray;				// Send to tray on start

	BOOL SavePassword(const CString& sUsername, const CString& sPassword);
	BOOL LoadPassword(CString& sUsername, CString& sPassword);

	void LogFormat(LPCTSTR szFormat, ...);
	void Log(const CString& sText = CString());
	void Log(UINT nID);

	// Refresh avatar list (NULL - whole list)
	inline void Refresh(BOOL bSuccess, CAvatar* pAvatar = NULL)
	{
		m_pMainWnd->PostMessage( WM_APP_REFRESH, bSuccess, (LPARAM)pAvatar );
	}

	// Show avatar notification (NULL - hide)
	inline void Notify(const CAvatar* pAvatar)
	{
		m_pMainWnd->PostMessage( WM_APP_NOTIFY, NULL, (LPARAM)pAvatar );
	}

	inline CString GetAvatarsKey() const
	{
		return CString( _T( "Software\\" ) ) + m_pszRegistryKey + _T( "\\" ) + m_pszAppName + _T( "\\" ) AVATARS;
	}

	inline CString GetCookiesKey() const
	{
		return CString( _T( "Software\\" ) ) + m_pszRegistryKey + _T( "\\" ) + m_pszAppName + _T( "\\" ) COOKIES;
	}

	void LoadAvatars();
	void SaveAvatars() const;
	void ClearAvatars();
	CAvatar* SetAvatar(const CString& sRealName, const CString& sDisplayName, const CString& sPlace, BOOL bOnline, BOOL bFriend);
	POSITION GetAvatarIterator() const;
	CAvatar* GetNextAvatar(POSITION& pos) const;
	UINT GetAvatarCount(BOOL bOnlineOnly = FALSE) const;
	CAvatar* GetEmptyAvatar() const;
	BOOL IsValid(const CAvatar* pTestAvatar) const;
	void DeleteAvatar(CAvatar* pAvatar);

	void AddCookie(const CString& sName, const CString& sValue);	// Add cookie by name
	CString GetCookie(const CString& sName) const;					// Get cookie by name
	BOOL HasCookies() const;										// Check for vital cookies
	CString GetAllCookies();										// Get all cookies in form of "Cookie: " HTTP header
	void LoadCookies();												// Load all cookies
	void SaveCookies() const;										// Save all cookies
	void ClearCookies();											// Delete all cookies

	mutable CCriticalSection		m_pSection;

protected:
	typedef CMap< CString, const CString&, CAvatar*, CAvatar* const & > CAvatarMap;
	typedef CMap< CString, const CString&, CString, const CString& > CCookieMap;

	CAutoPtr< CStdioFile >			m_pLog;
	CAvatarMap						m_pAvatars;
	CCookieMap						m_pCookies;
	DWORD							m_nCacheTime;

	virtual BOOL InitInstance();
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);

	DECLARE_MESSAGE_MAP()
};

extern CSLAPApp theApp;

// Homepage
extern LPCTSTR szAppURL;

// WinSparkle AppCast feed
extern LPCSTR szAppCastURL;

// Well-Known cookie names
extern LPCTSTR szLoginCookie;
extern LPCTSTR szSessionCookie;
extern LPCTSTR szMemberCookie;

extern LPCTSTR szLoginURL;
extern LPCTSTR szFriendsURL;
extern LPCTSTR szFriendsOnlineURL;
extern LPCTSTR szLoginReferer;
extern LPCSTR szLoginForm;
extern LPCTSTR szImageURL;
extern LPCTSTR pszAcceptTypes[];
extern LPCTSTR szVersion;

CString RegGetString(CRegKey& key, LPCTSTR szValueName = NULL, DWORD nMaxSize = MAX_PATH, LPCTSTR szDefault = NULL);
LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszSubString);
CString URLEncode(LPCTSTR szText);
CStringA URLEncode(LPCSTR szText);
