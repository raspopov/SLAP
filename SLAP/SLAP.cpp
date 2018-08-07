// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
This file is part of Second Life Avatar Probe (SLAP)

Copyright (C) 2015-2017 Nikolay Raspopov <raspopov@cherubicsoft.com>

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

#include "stdafx.h"
#include "SLAP.h"
#include "SLAPDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

LPCTSTR szAppURL			= _T("http://www.cherubicsoft.com/projects/slap");
LPCSTR  szAppCastURL		= "https://raw.githubusercontent.com/raspopov/SLAP/master/Setup/appcast.xml";
LPCTSTR szLoginCookie		= _T("agni_sl_session_id");
LPCTSTR szSessionCookie		= _T("session-token");
LPCTSTR szMemberCookie		= _T("second-life-member");
LPCTSTR szLoginURL			= _T("https://id.secondlife.com/openid/loginsubmit");
LPCTSTR szFriendsURL		= _T("https://secondlife.com/my/loadWidgetContent.php?widget=widgetFriends");
LPCTSTR szFriendsOnlineURL	= _T("https://secondlife.com/my/account/friends.php");
LPCTSTR szLoginReferer		= _T("https://id.secondlife.com/openid/login");
LPCSTR  szLoginForm			= "username={USERNAME}&password={PASSWORD}&Submit=&stay_logged_in=stay_logged_in&return_to={RETURNTO}&previous_language=en_US&language=en_US&show_join=True&from_amazon=";
LPCTSTR szImageURL			= _T("https://my-secondlife.s3.amazonaws.com/users/{USERNAME}/sl_image.png");
LPCTSTR pszAcceptTypes[]	= { _T( "*/*" ), NULL };
LPCTSTR szVersion			= _T( "HTTP/1.1" );


// Returns string from registry
CString RegGetString(CRegKey& key, LPCTSTR szValueName, DWORD nMaxSize, LPCTSTR szDefault)
{
	CString str;
	DWORD nLength = nMaxSize;
	LSTATUS ret = key.QueryStringValue( szValueName, str.GetBuffer( nLength ), &nLength );
	if ( ret == ERROR_SUCCESS && nLength )
		str.ReleaseBuffer( nLength - 1 );
	else
		str.ReleaseBuffer( 0 );
	return ( ret == ERROR_SUCCESS || ! szDefault ) ? str : CString( szDefault );
}

// Case independent string search
LPCTSTR _tcsistr(LPCTSTR pszString, LPCTSTR pszSubString)
{
	// Return null if string or substring is empty
	if ( !*pszString || !*pszSubString )
		return NULL;

	// Return if string is too small to hold the substring
	size_t nString(_tcslen(pszString));
	size_t nSubString(_tcslen(pszSubString));
	if ( nString < nSubString )
		return NULL;

	// Get the first character from the substring and lowercase it
	const TCHAR cFirstPatternChar = _totlower(*pszSubString);

	// Loop over the part of the string that the substring could fit into
	LPCTSTR pszCutOff = pszString + nString - nSubString;
	while ( pszString <= pszCutOff )
	{
		// Search for the start of the substring
		while ( pszString <= pszCutOff && _totlower(*pszString) != cFirstPatternChar )
		{
			++pszString;
		}

		// Exit loop if no match found
		if ( pszString > pszCutOff )
			break;

		// Check the rest of the substring
		size_t nChar(1);
		while ( pszSubString[ nChar ] && _totlower(pszString[ nChar ]) == _totlower(pszSubString[ nChar ]) )
		{
			++nChar;
		}

		// If the substring matched return a pointer to the start of the match
		if ( !pszSubString[ nChar ] )
			return pszString;

		// Move on to the next character and continue search
		++pszString;
	}

	// No match found, return a null pointer
	return NULL;
}


CString URLEncode(LPCTSTR szText)
{
	CString res;
	for ( ; *szText; ++szText )
	{
		if ( ( *szText >= _T( 'a' ) && *szText <= _T( 'z' ) ) ||
			 ( *szText >= _T( 'A' ) && *szText <= _T( 'Z' ) ) ||
			 ( *szText >= _T( '0' ) && *szText <= _T( '9' ) ) ||
			 *szText == _T( '-' ) || *szText == _T( '_' ) || *szText == _T( '.' ) || *szText == _T( '~' ) )
		{
			res += *szText;
		}
		else if ( *szText == _T( ' ' ) )
		{
			res += _T( '+' );
		}
		else
		{
			res.AppendFormat( _T( "%%%02X" ), (BYTE)*szText );
		}
	}
	return res;
}

CStringA URLEncode(LPCSTR szText)
{
	CStringA res;
	for ( ; *szText; ++szText )
	{
		if ( ( *szText >= 'a' && *szText <= 'z' ) ||
			 ( *szText >= 'A' && *szText <= 'Z' ) ||
			 ( *szText >= '0' && *szText <= '9' ) ||
			 *szText == '-' || *szText == '_' || *szText == '.' || *szText == '~' )
		{
			res += *szText;
		}
		else if ( *szText == ' ' )
		{
			res += '+';
		}
		else
		{
			res.AppendFormat( "%%%02X", (BYTE)*szText );
		}
	}
	return res;
}

// CSLAPApp

BEGIN_MESSAGE_MAP(CSLAPApp, CWinApp)
END_MESSAGE_MAP()

// CSLAPApp construction

CSLAPApp::CSLAPApp()
	: m_bDebugLog	( FALSE )
	, m_bTray		( FALSE )
	, m_nLangID		( GetUserDefaultLangID() )
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

#ifdef _DEBUG
	afxMemDF |= allocMemDF;
#endif
}

BOOL CSLAPApp::SavePassword(const CString& sUsername, const CString& sPassword)
{
	CREDENTIAL cred = {};
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = (LPWSTR)APP_TITLE;
	cred.CredentialBlobSize = ( sPassword.GetLength() + 1 ) * sizeof( TCHAR );
	cred.CredentialBlob = (LPBYTE)(LPCTSTR)sPassword;
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;
	cred.UserName = (LPWSTR)(LPCTSTR)sUsername;
	BOOL bRet = CredWrite( &cred, 0 );
	SecureZeroMemory( &cred, sizeof( cred ) );
	return bRet;
}

BOOL CSLAPApp::LoadPassword(CString& sUsername, CString& sPassword)
{
	PCREDENTIAL pcred = NULL;
	if ( CredRead( APP_TITLE, CRED_TYPE_GENERIC, 0, &pcred ) )
	{
		sUsername = pcred->UserName;
		sPassword = CString( (LPCTSTR)pcred->CredentialBlob, pcred->CredentialBlobSize / sizeof( TCHAR ) );
		CredFree( pcred );
		return TRUE;
	}
	return FALSE;
}

void CSLAPApp::LogFormat(LPCTSTR szFormat, ...)
{
#ifndef _DEBUG
	if ( m_bDebugLog )
#endif
	{
		va_list pArgs;
		va_start( pArgs, szFormat );

		CString strTemp;
		strTemp.FormatV( szFormat, pArgs );

		Log( strTemp );

		va_end( pArgs );
	}
}

void CSLAPApp::Log( UINT nID )
{
#ifndef _DEBUG
	if ( m_bDebugLog )
#endif
	{
		Log( LoadString( nID ) );
	}
}

void CSLAPApp::Log(const CString& sText)
{
	if ( m_bDebugLog && ! m_pLog )
	{
		// Open file
		try
		{
			m_pLog.Attach( new CStdioFile( sDesktop + CTime::GetCurrentTime().Format( _T( "\\SLAP-%Y%m%d.txt" ) ),
				CFile::modeCreate | CFile::modeWrite | CFile::typeUnicode | CFile::shareDenyWrite ) );

			Log( CString( _T("This file created by ") ) + m_pszAppName + _T(" ") + sVersion + _T(" (") + _T( __DATE__ ) + _T(" ") +
				_T( __TIME__ ) + _T(")\n") + LoadString( IDS_WARNING ) );
		}
		catch ( ... ) {}
	}
	else if ( ! m_bDebugLog && m_pLog )
	{
		// Close file
		try
		{
			m_pLog->Close();
		}
		catch ( ... ) {}
		m_pLog.Free();
	}

#ifndef _DEBUG
	if ( m_bDebugLog && m_pLog )
#endif
	{
		const int nLength = sText.GetLength();
		if ( nLength )
		{
			CSingleLock pLock( &m_pSection, TRUE );

			for ( int nStart = 0; nStart < nLength; ++nStart )
			{
				const CString sPart = sText.Mid( nStart ).SpanExcluding( _T( "\r\n" ) );
				if ( const int nSize = sPart.GetLength() )
				{
					const CString sLog = CTime::GetCurrentTime().Format( _T( "[%H:%M:%S] " ) ) + ( nStart ? _T( " | " ) : _T( "" ) ) + sPart + _T( "\n" );
					if ( m_pLog )
					{
						try
						{
							m_pLog->WriteString( sLog );
							m_pLog->Flush();
						}
						catch ( ... ) {}
					}
#ifdef _DEBUG
					OutputDebugString( sLog );
#endif
					nStart += nSize;
				}
			}
		}
	}
}

void CSLAPApp::LoadAvatars()
{
	LSTATUS ret;

	CSingleLock pLock( &m_pSection, TRUE );

	ClearAvatars();

	CRegKey keyAvatars;
	ret = keyAvatars.Create( HKEY_CURRENT_USER, GetAvatarsKey() );
	if ( ret == ERROR_SUCCESS )
	{
		for ( int i = 0;; ++i )
		{
			CString sRealName;
			DWORD nLength = MAX_PATH;
			ret = keyAvatars.EnumKey( i, sRealName.GetBuffer( nLength ), &nLength, NULL );
			if ( ret == ERROR_SUCCESS )
			{
				sRealName.ReleaseBuffer( nLength );
				sRealName.MakeLower();
				sRealName.Replace( _T( " resident" ), _T( "" ) ); // Cut off "Resident" family
			}
			else
			{
				sRealName.ReleaseBuffer( 0 );
				break;
			}

			CAutoPtr< CAvatar > pAvatar( new CAvatar( sRealName ) );
			if ( pAvatar )
			{
				if ( pAvatar->Load() )
				{
					m_pAvatars.SetAt( sRealName, pAvatar.Detach() );
				}
			}
		}
	}

#ifdef _DEBUG
	if ( GetAvatarCount() == 0 )
	{
		CAvatar* pAvatar = SetAvatar( _T("Demo1 Resident"), _T("Demo1 Avatar"), _T("Demo1 Location"), FALSE, FALSE );
		for ( int i = 0; i < 24 ; ++i ) pAvatar->m_nTimeline[ i ] = i;
		pAvatar = SetAvatar( _T( "Demo2 Resident" ), _T( "Demo2 Avatar" ), _T( "Demo2 Location" ), FALSE, FALSE );
		for ( int i = 0; i < 24; ++i ) pAvatar->m_nTimeline[ i ] = 100000 + rand();
		pAvatar = SetAvatar( _T( "Demo3 Resident" ), _T( "Demo3 Avatar" ), _T( "Demo3 Location" ), FALSE, FALSE );
		for ( int i = 0; i < 12; ++i ) pAvatar->m_nTimeline[ i ] = 1000000;
	}
#endif
}

void CSLAPApp::SaveAvatars() const
{
	CSingleLock pLock( &m_pSection, TRUE );

	// Create a new one
	for ( POSITION pos = m_pAvatars.GetStartPosition(); pos; )
	{
		CString sKey;
		CAvatar* pAvatar = NULL;
		m_pAvatars.GetNextAssoc( pos, sKey, pAvatar );

		pAvatar->Save();
	}
}

void CSLAPApp::ClearAvatars()
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = m_pAvatars.GetStartPosition(); pos; )
	{
		CString sKey;
		CAvatar* pAvatar = NULL;
		m_pAvatars.GetNextAssoc( pos, sKey, pAvatar );

		delete pAvatar;
	}

	m_pAvatars.RemoveAll();
}

CAvatar* CSLAPApp::SetAvatar(const CString& sRealName, const CString& sDisplayName, const CString& sPlace, BOOL bOnline, BOOL bFriend)
{
	BOOL bSave = FALSE;

	CSingleLock pLock( &m_pSection, TRUE );

	CString sKey( sRealName );
	sKey.MakeLower();
	sKey.Replace( _T( " resident" ), _T( "" ) ); // Cut off "Resident" family

	CAvatar* pAvatar;
	if ( ! m_pAvatars.Lookup( sKey, pAvatar ) )
	{
		bSave = TRUE;
		pAvatar = new CAvatar( sKey );
		if ( ! pAvatar )
			// Out of memory
			return NULL;
		m_pAvatars.SetAt( sKey, pAvatar );
	}

	if ( pAvatar->m_sDisplayName != sDisplayName )
	{
		pAvatar->m_sDisplayName = sDisplayName;
		pAvatar->m_bDirty = TRUE;
		bSave = TRUE;
	}

	if ( pAvatar->m_sPlace != sPlace )
	{
		pAvatar->m_sPlace = sPlace;
		bSave = TRUE;
	}

	if ( pAvatar->m_bOnline != bOnline )
	{
		bSave = TRUE;
	}

	if ( pAvatar->m_bOnline )
	{
		const CTime tNow = CTime::GetCurrentTime();
		const int nHour = tNow.GetHour();
		if ( nHour >= 0 && nHour <= 23 )
		{
			++ pAvatar->m_nTimeline[ nHour ];
		}
		pAvatar->m_tLastOnline = tNow;
	}

	// Prefer online status
	if ( bOnline ) pAvatar->m_bNewOnline = TRUE;

	// Prefer friend status
	if ( bFriend ) pAvatar->m_bNewFriend = TRUE;

	if ( bSave )
		pAvatar->Save();

	Log( _T( "Updated avatar: \"" ) + pAvatar->m_sDisplayName + _T( "\" (" ) + pAvatar->m_sRealName + _T( ") " ) +
		( ( pAvatar->m_bOnline != pAvatar->m_bNewOnline ) ?  ( pAvatar->m_bOnline ? _T("offline -> online") : _T("online -> offline") ) : _T("") ) );

	return pAvatar;
}

POSITION CSLAPApp::GetAvatarIterator() const
{
	return m_pAvatars.GetStartPosition();
}

CAvatar* CSLAPApp::GetNextAvatar(POSITION& pos) const
{
	CString sKey;
	CAvatar* pAvatar = NULL;
	m_pAvatars.GetNextAssoc( pos, sKey, pAvatar );

	return pAvatar;
}

UINT CSLAPApp::GetAvatarCount(BOOL bOnlineOnly) const
{
	CSingleLock pLock( &m_pSection, TRUE );

	if ( bOnlineOnly )
	{
		UINT nOnline = 0;
		for ( POSITION pos = m_pAvatars.GetStartPosition(); pos; )
		{
			CString sKey;
			CAvatar* pAvatar = NULL;
			m_pAvatars.GetNextAssoc(pos, sKey, pAvatar);
			if ( pAvatar->m_bOnline )
				++nOnline;
		}
		return nOnline;
	}
	else
		return (UINT)m_pAvatars.GetCount();
}

CAvatar* CSLAPApp::GetEmptyAvatar() const
{
	if ( sImageCache.IsEmpty() )
		// Disabled cache
		return NULL;

	CSingleLock pLock( &m_pSection, TRUE );

	const CTime tNow = CTime::GetCurrentTime();

	// Select online avatars first
	for ( POSITION pos = m_pAvatars.GetStartPosition(); pos; )
	{
		CString sKey;
		CAvatar* pAvatar = NULL;
		m_pAvatars.GetNextAssoc( pos, sKey, pAvatar );

		if ( pAvatar->m_bOnline && ( tNow - pAvatar->m_tImage ).GetTotalHours() > CACHE_TIME )
			return pAvatar;
	}

	// Offline second
	for ( POSITION pos = m_pAvatars.GetStartPosition(); pos; )
	{
		CString sKey;
		CAvatar* pAvatar = NULL;
		m_pAvatars.GetNextAssoc( pos, sKey, pAvatar );

		if ( ! pAvatar->m_bOnline && ( tNow - pAvatar->m_tImage ).GetTotalHours() > CACHE_TIME )
			return pAvatar;
	}

	return NULL;
}

BOOL CSLAPApp::IsValid(const CAvatar* pTestAvatar) const
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = m_pAvatars.GetStartPosition(); pos; )
	{
		CString sKey;
		CAvatar* pAvatar = NULL;
		m_pAvatars.GetNextAssoc( pos, sKey, pAvatar );

		if ( pAvatar == pTestAvatar )
			return TRUE;
	}

	return FALSE;
}

void CSLAPApp::DeleteAvatar(CAvatar* pAvatar)
{
	CSingleLock pLock( &m_pSection, TRUE );

	for ( POSITION pos = m_pAvatars.GetStartPosition(); pos; )
	{
		CString sKey;
		CAvatar* pListAvatar = NULL;
		m_pAvatars.GetNextAssoc( pos, sKey, pListAvatar );

		if ( pAvatar == pListAvatar )
		{
			m_pAvatars.RemoveKey( sKey );

			pAvatar->Delete();

			delete pAvatar;

			break;
		}
	}
}

void CSLAPApp::AddCookie( const CString& sName, const CString& sValue )
{
	CSingleLock pLock( &m_pSection, TRUE );

	m_pCookies.SetAt( CString( sName ).MakeLower(), sValue );
}

CString CSLAPApp::GetCookie( const CString& sName ) const
{
	CSingleLock pLock( &m_pSection, TRUE );

	CString sValue;
	m_pCookies.Lookup( CString( sName ).MakeLower(), sValue );
	return sValue;
}

BOOL CSLAPApp::HasCookies() const
{
	CSingleLock pLock( &m_pSection, TRUE );

	return m_pCookies.GetCount() >= 3 &&
		! GetCookie( szLoginCookie ).IsEmpty() &&
		! GetCookie( szSessionCookie ).IsEmpty() &&
		! GetCookie( szMemberCookie ).IsEmpty();
}

CString CSLAPApp::GetAllCookies()
{
	CSingleLock pLock( &m_pSection, TRUE );

	CString sCookies;
	for ( POSITION pos = m_pCookies.GetStartPosition(); pos; )
	{
		CString sName, sValue;
		m_pCookies.GetNextAssoc( pos, sName, sValue );
		if ( ! sCookies.IsEmpty() ) sCookies += _T( "; " );
		sCookies += sName + _T( "=" ) + sValue;
	}
	return sCookies;
}

void CSLAPApp::LoadCookies()
{
	LSTATUS ret;

	CSingleLock pLock( &m_pSection, TRUE );

	ClearCookies();

	CRegKey keyCookies;
	ret = keyCookies.Create( HKEY_CURRENT_USER, GetCookiesKey() );
	if ( ret == ERROR_SUCCESS )
	{
		for ( int i = 0;; ++i )
		{
			CString sName;
			DWORD nLength = MAX_PATH;
			ret = keyCookies.EnumKey( i, sName.GetBuffer( nLength ), &nLength, NULL );
			if ( ret == ERROR_SUCCESS )
				sName.ReleaseBuffer( nLength );
			else
			{
				sName.ReleaseBuffer( 0 );
				break;
			}

			CRegKey keyCookie;
			ret = keyCookie.Create( HKEY_CURRENT_USER, GetCookiesKey() + _T( "\\" ) + sName );
			ASSERT( ret == ERROR_SUCCESS );
			if ( ret == ERROR_SUCCESS )
			{
				CString sValue;
				nLength = 1024;
				ret = keyCookie.QueryStringValue( NULL, sValue.GetBuffer( nLength ), &nLength );
				if ( ret == ERROR_SUCCESS && nLength )
					sValue.ReleaseBuffer( nLength - 1 );
				else
					sValue.ReleaseBuffer( 0 );

				AddCookie( sName, sValue );
			}
		}
	}
}

void CSLAPApp::SaveCookies() const
{
	LSTATUS ret;

	CSingleLock pLock( &m_pSection, TRUE );

	// Remove old cookie list
	ret = SHDeleteKey( HKEY_CURRENT_USER, GetCookiesKey() );

	// Create a new one
	for ( POSITION pos = m_pCookies.GetStartPosition(); pos; )
	{
		CString sName, sValue;
		m_pCookies.GetNextAssoc( pos, sName, sValue );

		CRegKey keyCookie;
		ret = keyCookie.Create( HKEY_CURRENT_USER, GetCookiesKey() + _T( "\\" ) + sName );
		ASSERT( ret == ERROR_SUCCESS );
		if ( ret == ERROR_SUCCESS )
		{
			ret = keyCookie.SetStringValue( NULL, sValue );
			ASSERT( ret == ERROR_SUCCESS );
		}
	}
}

void CSLAPApp::ClearCookies()
{
	CSingleLock pLock( &m_pSection, TRUE );

	m_pCookies.RemoveAll();

	// Add typical cookies
	AddCookie( _T( "locale" ), _T( "en-US" ) );
	AddCookie( _T( "rating_filter" ), _T( "gma" ) );
}

BOOL CSLAPApp::InitInstance()
{
	if ( SUCCEEDED( CoInitialize( NULL ) ) )
	{
		INITCOMMONCONTROLSEX InitCtrls = { sizeof(InitCtrls), ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES | ICC_LINK_CLASS };
		InitCommonControlsEx( &InitCtrls );

		__super::InitInstance();

		CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows ) );

		SetRegistryKey( AFX_IDS_COMPANY_NAME );

		SetAppID( CString( m_pszRegistryKey ) + _T(".") + m_pszAppName );

		m_nLangID = (LANGID)GetProfileInt( SETTINGS, LANGUAGE, m_nLangID );

		ParseCommandLine( *this );

		theLoc.Load();
		theLoc.Select( m_nLangID );

		// Get application .exe-file path
		GetModuleFileName( AfxGetInstanceHandle(), sModuleFileName.GetBuffer( MAX_PATH ), MAX_PATH );
		sModuleFileName.ReleaseBuffer();

		// Get image cache folder
		SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, sImageCache.GetBuffer( MAX_PATH ) );
		sImageCache.ReleaseBuffer();
		sImageCache = sImageCache + _T( "\\" ) + m_pszRegistryKey + _T( "\\" ) + m_pszAppName;
		SHCreateDirectory( NULL, sImageCache );

		// Get user desktop folder
		SHGetFolderPath( NULL, CSIDL_DESKTOP | CSIDL_FLAG_CREATE, NULL, 0, sDesktop.GetBuffer( MAX_PATH ) );
		sDesktop.ReleaseBuffer();

		// Get user desktop folder
		SHGetFolderPath( NULL, CSIDL_STARTUP | CSIDL_FLAG_CREATE, NULL, 0, sStartup.GetBuffer( MAX_PATH ) );
		sStartup.ReleaseBuffer();

		// Allow only one instance of application
		HANDLE hAppMutex = CreateMutex( NULL, TRUE, CString( _T( "Global\\" ) ) + m_pszAppName );
		if ( hAppMutex && GetLastError() != ERROR_ALREADY_EXISTS )
		{
			// Create log file at Desktop
			m_bDebugLog	= ( GetProfileInt( SETTINGS, DEBUGLOG, 0 ) != 0 );

			// Log version information and warnings
			DWORD handle = NULL;
			if ( DWORD size = GetFileVersionInfoSize( sModuleFileName, &handle ) )
			{
				if ( char* ver = reinterpret_cast< char* >( GlobalAlloc( GPTR, size ) ) )
				{
					if ( GetFileVersionInfo( sModuleFileName, handle, size, ver ) )
					{
						UINT len = 0;
						VS_FIXEDFILEINFO* fix = NULL;
						if ( VerQueryValue( ver, _T( "\\" ), (void**)&fix, &len ) && len )
						{
							sVersion.Format( _T( "%u.%u.%u.%u" ),
								HIWORD( fix->dwFileVersionMS ), LOWORD( fix->dwFileVersionMS ),
								HIWORD( fix->dwFileVersionLS ), LOWORD( fix->dwFileVersionLS ) );

							LPCTSTR szLegalCopyright;
							if ( VerQueryValue( ver, _T( "\\StringFileInfo\\000004b0\\LegalCopyright" ), (void**)&szLegalCopyright, &len ) && len )
							{
								sCopyright = szLegalCopyright;
							}
						}
					}
					GlobalFree( ver );
				}
			}

			// Initialize WinSock
			WSADATA wsaData = {};
			if ( WSAStartup( MAKEWORD( 1, 1 ), &wsaData ) == 0 )
			{
				LoadCookies();

				// Setup WinSparkle update system
				win_sparkle_set_appcast_url( szAppCastURL );
				win_sparkle_set_langid( m_nLangID );
				win_sparkle_set_shutdown_request_callback( &CSLAPApp::OnShutdown );

				CSLAPDlg dlg;
				m_pMainWnd = &dlg;
				dlg.DoModal();

				WriteProfileInt( SETTINGS, LANGUAGE, m_nLangID );

				SaveCookies();

				ClearAvatars();

				WSACleanup();
			}
			else
			{
				Log( IDP_SOCKETS_INIT_FAILED );
				AfxMessageBox( IDP_SOCKETS_INIT_FAILED, MB_OK | MB_ICONSTOP );
			}
		}
		else if ( ! m_bTray )
		{
			// Show existing window
			if ( HWND hWnd = FindWindow( NULL, m_pszAppName ) )
			{
				ShowWindow( hWnd, SW_SHOW );
				SetForegroundWindow( hWnd );
			}
		}

		CoUninitialize();
	}
	return FALSE;
}

void __cdecl CSLAPApp::OnShutdown()
{
	if ( theApp.m_pMainWnd )
		::PostMessage( theApp.m_pMainWnd->GetSafeHwnd(), WM_CLOSE, 0, 0 );
}

BOOL CSLAPApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	if ( m_pMainWnd && ( lpMsg->hwnd == m_pMainWnd->m_hWnd || ::IsChild( m_pMainWnd->m_hWnd, lpMsg->hwnd ) ) )
	{
		if ( lpMsg->message == WM_KEYDOWN )
		{
			// Emulate key down message for dialog
			if ( ((CSLAPDlg*)m_pMainWnd)->OnKeyDown( (UINT)lpMsg->wParam, (UINT)( lpMsg->lParam & 0xffff ), (UINT)( ( lpMsg->lParam >> 16 ) & 0xffff ) ) )
				return TRUE;
		}
	}

	return CWinApp::ProcessMessageFilter( code, lpMsg );
}

void CSLAPApp::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL /*bLast*/)
{
	if ( _tcsicmp( pszParam, _T( "tray" ) ) == 0 && bFlag )
	{
		m_bTray = TRUE;
	}
	else if ( _tcsnicmp( pszParam, _T( "lang:" ), 5 ) == 0 && bFlag )
	{
		m_nLangID = (LANGID)_tcstoul( pszParam + 5, NULL, 16 );
	}
}

CSLAPApp		theApp;
CLocalization	theLoc;
