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

#include "stdafx.h"
#include "SLAP.h"
#include "SLAPDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


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


// CAvatar

CAvatar::CAvatar()
	: m_bOnline			( FALSE )
	, m_bFriend			( FALSE )
	, m_bLoopSound		( FALSE )
	, m_bNewOnline		( FALSE )
	, m_bNewFriend		( FALSE )
	, m_bDirty			( TRUE )
{
}

CAvatar::~CAvatar()
{
	m_pImage.Destroy();
}

CString CAvatar::GetSortName() const
{
	return ( m_bFriend ? ( m_bOnline ? _T( "A" ) : _T( "B" ) ) : _T( "C" ) ) + m_sDisplayName + _T( " " ) + m_sRealName;
}

CString CAvatar::GetSound() const
{
	const CString sSound = m_bOnline ? m_sOnlineSound : m_sOfflineSound;
	return ( sSound == NO_SOUND ) ? CString() : ( sSound.IsEmpty() ? theApp.GetProfileString( SETTINGS, m_bOnline ? SOUND_ONLINE : SOUND_OFFLINE ) : sSound );
}

BOOL CAvatar::IsValidUsername(LPCTSTR szUsername)
{
	if ( ! szUsername || ! *szUsername )
		return FALSE;

	UINT nLength = 0;
	BOOL bSpace = FALSE;
	for ( LPCTSTR ch = szUsername; *ch ; ++ch )
	{
		if ( ( *ch >= _T('0') && *ch <= _T('9') ) || ( *ch >= _T('a') && *ch <= _T('z') ) || ( *ch >= _T('A') && *ch <= _T('Z') ) ) // Can be alpha-numeric
		{
			if ( ++ nLength > 31 ) // Maximum length is 31 symbols
			{
				ASSERT( FALSE );
				return FALSE;
			}
		}
		else if ( *ch == _T(' ') && ch != szUsername && *( ch + 1 ) && ! bSpace ) // Can be one space in the middle
		{
			bSpace = TRUE;
			nLength = 0;
		}
		else
		{
			ASSERT( FALSE );
			return FALSE;
		}
	}

	return TRUE;
}

CString CAvatar::MakePretty(const CString& sName)
{
	int nPos = sName.FindOneOf( _T(" .") );
	if ( nPos != -1 )
		return sName.Left( 1 ).MakeUpper() + sName.Mid( 1, nPos ).MakeLower() + _T(" ") +
			sName.Mid( nPos + 1, 1 ).MakeUpper() + sName.Mid( nPos + 2 ).MakeLower();
	else
		return sName.Left( 1 ).MakeUpper() + sName.Mid( 1 ).MakeLower(); // No "Resident"
}

// CSLAPApp

BEGIN_MESSAGE_MAP(CSLAPApp, CWinApp)
END_MESSAGE_MAP()

// CSLAPApp construction

CSLAPApp::CSLAPApp()
	: m_bDebugLog	( FALSE )
	, m_bTray		( FALSE )
{
#ifdef _DEBUG
	afxMemDF |= allocMemDF;
#endif
}

BOOL CSLAPApp::SavePassword(const CString& sUsername, const CString& sPassword)
{
	CREDENTIAL cred = {};
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = (LPWSTR)(LPCTSTR)sFullTitle;
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
	if ( CredRead( sFullTitle, CRED_TYPE_GENERIC, 0, &pcred ) )
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
		CString sText;
		sText.LoadString( nID );
		Log( sText );
	}
}

void CSLAPApp::Log(const CString& sText)
{
	if ( m_bDebugLog && ! m_pLog )
	{
		// Open file
		TRY
		{
			m_pLog.Attach( new CStdioFile( sDesktop + CTime::GetCurrentTime().Format( _T( "\\SLAP-%Y%m%d.txt" ) ),
				CFile::modeCreate | CFile::modeWrite | CFile::typeUnicode | CFile::shareDenyWrite ) );

			CString sWarning;
			sWarning.LoadString( IDS_WARNING );
			Log( CString( _T("This file created by ") ) + m_pszAppName + _T(" ") + sVersion + _T(" (") + _T( __DATE__ ) + _T(" ") + _T( __TIME__ ) + _T(")\n") + sWarning );
		}
		CATCH_ALL( e )
		END_CATCH_ALL
	}
	else if ( ! m_bDebugLog && m_pLog )
	{
		// Close file
		TRY
		{
			m_pLog->Close();
		}
		CATCH_ALL( e )
		END_CATCH_ALL
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
						TRY
						{
							m_pLog->WriteString( sLog );
							m_pLog->Flush();
						}
						CATCH_ALL( e )
						END_CATCH_ALL
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

CString RegGetString(CRegKey& key, LPCTSTR szValueName = NULL, DWORD nMaxSize = MAX_PATH, LPCTSTR szDefault = NULL)
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
				sRealName.ReleaseBuffer( nLength );
			else
			{
				sRealName.ReleaseBuffer( 0 );
				break;
			}

			CRegKey keyAvatar;
			ret = keyAvatar.Create( HKEY_CURRENT_USER, GetAvatarsKey() + _T("\\") + sRealName );
			ASSERT( ret == ERROR_SUCCESS );
			if ( ret == ERROR_SUCCESS )
			{
				CAutoPtr< CAvatar > pAvatar( new CAvatar() );
				if ( pAvatar )
				{
					pAvatar->m_sRealName = sRealName;
					pAvatar->m_sDisplayName = RegGetString( keyAvatar );

					DWORD bLoop = 0;
					keyAvatar.QueryDWORDValue( LOOP_SOUND, bLoop );
					pAvatar->m_bLoopSound = ( bLoop != 0 );

					DWORD bOnline = 0;
					keyAvatar.QueryDWORDValue( ONLINE, bOnline );
					pAvatar->m_bOnline = ( bOnline != 0 );

					DWORD bFriend = 0;
					keyAvatar.QueryDWORDValue( FRIEND, bFriend );
					pAvatar->m_bFriend = ( bFriend != 0 );

					ULONGLONG nOnline = 0;
					keyAvatar.QueryQWORDValue( LAST_ONLINE, nOnline );
					pAvatar->m_tLastOnline = CTime( (__time64_t)nOnline );

					ULONGLONG nImage = 0;
					keyAvatar.QueryQWORDValue( IMAGE_TIME, nImage );
					pAvatar->m_tImage = CTime( (__time64_t)nImage );

					pAvatar->m_sPlace = RegGetString( keyAvatar, PLACE );
					pAvatar->m_sOnlineSound = RegGetString( keyAvatar, SOUND_ONLINE );
					pAvatar->m_sOfflineSound = RegGetString( keyAvatar, SOUND_OFFLINE );

					if ( ! sImageCache.IsEmpty() )
					{
						const CString sCached = sImageCache + _T( "\\" ) + pAvatar->m_sRealName + _T( ".png" );
						if ( FAILED( pAvatar->m_pImage.Load( sCached ) ) )
							// Mark for reload
							pAvatar->m_tLastOnline = CTime();
					}

					CString sKey( sRealName );
					sKey.MakeLower();
					m_pAvatars.SetAt( sKey, pAvatar.Detach() );
				}
			}
		}
	}
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

		SaveAvatar( pAvatar );
	}
}

void CSLAPApp::SaveAvatar(CAvatar* pAvatar) const
{
	CRegKey keyAvatar;
	LSTATUS ret = keyAvatar.Create( HKEY_CURRENT_USER, GetAvatarsKey() + _T( "\\" ) + pAvatar->m_sRealName );
	ASSERT( ret == ERROR_SUCCESS );
	if ( ret == ERROR_SUCCESS )
	{
		keyAvatar.SetStringValue( NULL, pAvatar->m_sDisplayName );
		keyAvatar.SetDWORDValue( ONLINE, pAvatar->m_bOnline ? 1 : 0 );
		keyAvatar.SetDWORDValue( FRIEND, pAvatar->m_bFriend ? 1 : 0 );
		keyAvatar.SetQWORDValue( LAST_ONLINE, (ULONGLONG)(__time64_t)pAvatar->m_tLastOnline.GetTime() );
		keyAvatar.SetQWORDValue( IMAGE_TIME, (ULONGLONG)(__time64_t)pAvatar->m_tImage.GetTime() );
		keyAvatar.SetStringValue( PLACE, pAvatar->m_sPlace );
		keyAvatar.SetStringValue( SOUND_ONLINE, pAvatar->m_sOnlineSound );
		keyAvatar.SetStringValue( SOUND_OFFLINE, pAvatar->m_sOfflineSound );
		keyAvatar.SetDWORDValue( LOOP_SOUND, pAvatar->m_bLoopSound ? 1 : 0 );
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

void CSLAPApp::SetAvatar(const CString& sRealName, const CString& sDisplayName, const CString& sPlace, BOOL bOnline, BOOL bFriend)
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
		pAvatar = new CAvatar();
		if ( ! pAvatar )
			return;
		pAvatar->m_sRealName = sKey;
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
		pAvatar->m_tLastOnline = CTime::GetCurrentTime();

	// Prefer online status
	if ( bOnline ) pAvatar->m_bNewOnline = TRUE;

	// Prefer friend status
	if ( bFriend ) pAvatar->m_bNewFriend = TRUE;

	if ( bSave )
		SaveAvatar( pAvatar );

	Log( _T( "Updated avatar: \"" ) + pAvatar->m_sDisplayName + _T( "\" (" ) + pAvatar->m_sRealName + _T( ") " ) +
		( ( pAvatar->m_bOnline != pAvatar->m_bNewOnline ) ?  ( pAvatar->m_bOnline ? _T("offline -> online") : _T("online -> offline") ) : _T("") ) );
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

BOOL CSLAPApp::IsValid(CAvatar* pTestAvatar) const
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

			SHDeleteKey( HKEY_CURRENT_USER, GetAvatarsKey() + _T("\\") + pAvatar->m_sRealName );

			if ( ! sImageCache.IsEmpty() )
			{
				const CString sCached = sImageCache + _T( "\\" ) + pAvatar->m_sRealName + _T( ".png" );
				DeleteFile( sCached );
			}

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
		! GetCookie( sLoginCookie ).IsEmpty() &&
		! GetCookie( sSessionCookie ).IsEmpty() &&
		! GetCookie( sMemberCookie ).IsEmpty();
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

	// Load well-known cookie names
	sLoginCookie.LoadString( IDS_LOGIN_COOKIE );
	sSessionCookie.LoadString( IDS_SESSION_COOKIE );
	sMemberCookie.LoadString( IDS_MEMBER_COOKIE );

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

		SetRegistryKey( AFX_IDS_COMPANY_NAME );

		SetAppID( CString( m_pszRegistryKey ) + _T(".") + m_pszAppName );

		ParseCommandLine( *this );

		sFullTitle.LoadString( IDS_FULLTITLE );

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
				if ( char* ver = (char*)GlobalAlloc( GPTR, size ) )
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

				CSLAPDlg dlg;
				m_pMainWnd = &dlg;
				dlg.DoModal();

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

BOOL CSLAPApp::ProcessMessageFilter(int code, LPMSG lpMsg)
{
	if ( lpMsg->hwnd == m_pMainWnd->m_hWnd || ::IsChild( m_pMainWnd->m_hWnd, lpMsg->hwnd ) )
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
}

CSLAPApp theApp;
