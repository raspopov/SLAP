// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
This file is part of Second Life Avatar Probe (SLAP)

Copyright (C) 2015-2024 Nikolay Raspopov <raspopov@cherubicsoft.com>

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

CStringA GetValue( const CStringA& sText, const CStringA& sName )
{
	int nBegin = sText.Find( sName + "=" );
	if ( nBegin != -1 )
	{
		nBegin += sName.GetLength() + 2;
		const int nEnd = sText.Find( sText.GetAt( nBegin - 1 ), nBegin );
		if ( nEnd != -1 )
		{
			return sText.Mid( nBegin, nEnd - nBegin );
		}
	}
	return CStringA();
}

BOOL ParseCookie(const CString& sCookie, CString& sName, CString& sValue)
{
	int nStart = sCookie.Find( _T( '=' ) );
	if ( nStart != -1 )
	{
		sName = sCookie.Mid( 0, nStart );
		int nEnd = sCookie.Find( _T( ';' ), nStart + 1 );
		if ( nEnd != -1 )
			sValue = sCookie.Mid( nStart + 1, nEnd - nStart - 1 );
		else
			sValue = sCookie.Mid( nStart + 1 );
		return TRUE;
	}
	return FALSE;
}

CString QueryInfo(CHttpFile* pFile, DWORD dwInfoLevel, LPDWORD lpdwIndex = NULL)
{
	CString str;
	DWORD dwLen = 0;
	if ( ! HttpQueryInfo( *pFile, dwInfoLevel, NULL, &dwLen, 0 ) )
	{
		LPTSTR pstr = str.GetBufferSetLength( dwLen / sizeof( TCHAR ) );
		if ( HttpQueryInfo( *pFile, dwInfoLevel, pstr, &dwLen, lpdwIndex ) )
			str.ReleaseBuffer( dwLen / sizeof( TCHAR ) );
		else
			str.ReleaseBuffer( 0 );
	}
	return str;
}

INT_PTR GetContent(CHttpFile* pFile, CByteArray& aContent)
{
	for ( ;; )
	{
		const INT_PTR nOldSize = aContent.GetSize();
		if ( nOldSize >= 1024 * 1024 ) // 1MB maximum size
			break;
		aContent.SetSize( nOldSize + 1024 );
		const UINT read = pFile->Read( aContent.GetData() + nOldSize, 1024 );
		if ( ! read )
			break;
		aContent.SetSize( nOldSize + read );
	}
	return aContent.GetSize();
}

inline CStringA GetString(CByteArray& aContent)
{
	return CStringA( (LPCSTR)aContent.GetData(), (int)aContent.GetSize() );
}

DWORD CSLAPDlg::WebRequest(CInternetSession* pInternet, const CString& sUrl, const CString& sReferer, CByteArray& aContent, CString& sLocation, LPCSTR szParams, BOOL bPOST, BOOL bNoCache)
{
	DWORD nStatus = 0;

	aContent.RemoveAll();
	sLocation.Empty();

	theApp.LogFormat( _T( "Navigating to %s" ), (LPCTSTR)sUrl );

	DWORD dwServiceType;
	CString sServer;
	CString sObject;
	INTERNET_PORT nPort;
	if ( ! AfxParseURL( sUrl, dwServiceType, sServer, sObject, nPort ) )
		return nStatus;

	const BOOL bHTTPS = ( dwServiceType == AFX_INET_SERVICE_HTTPS );

	CString sHeaders;
	if ( bPOST )
	{
		if ( ! sHeaders.IsEmpty() ) sHeaders += _T( "\r\n" );
		sHeaders += _T( "Content-Type: application/x-www-form-urlencoded" );
	}
	else if ( szParams )
	{
		sObject += _T( "?" );
		sObject += szParams;
	}

	// Prepare cookies
	CString sSendCookie = theApp.GetAllCookies();
	if ( ! sSendCookie.IsEmpty() )
	{
		if ( ! sHeaders.IsEmpty() ) sHeaders += _T( "\r\n" );
		sHeaders += _T( "Cookie: " );
		sHeaders += sSendCookie;
	}

	const DWORD dwFlags =
		( bHTTPS ? INTERNET_FLAG_SECURE : 0 ) |
		( bPOST ? INTERNET_FLAG_FORMS_SUBMIT : 0 ) |
		( bNoCache ? INTERNET_FLAG_RELOAD : 0 ) |
		( bNoCache ? INTERNET_FLAG_DONT_CACHE : 0 ) |
		( bNoCache ? INTERNET_FLAG_PRAGMA_NOCACHE : 0 ) |
		INTERNET_FLAG_NO_AUTO_REDIRECT |
		INTERNET_FLAG_NO_COOKIES |
		INTERNET_FLAG_NO_AUTH |
		INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
		INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

	try
	{
		CAutoPtr< CHttpConnection > pConnection( pInternet->GetHttpConnection( sServer, dwFlags, nPort ) );
		if ( pConnection )
		{
			try
			{
				LPCTSTR pszAcceptTypes[] = { _T( "*/*" ), NULL };
				CAutoPtr< CHttpFile > pFile( pConnection->OpenRequest( ( bPOST ? CHttpConnection::HTTP_VERB_POST : CHttpConnection::HTTP_VERB_GET ),
					sObject, sReferer, 1, pszAcceptTypes, _T("HTTP/1.1"), dwFlags ) );
				if ( pFile )
				{
					try
					{
						if ( pFile->SendRequest( sHeaders,
							( bPOST && szParams ? (LPVOID)szParams : NULL ),
							( bPOST && szParams ? (DWORD)strlen( szParams ) : 0 ) ) )
						{
							pFile->QueryInfoStatusCode( nStatus );

							if ( theApp.m_bDebugLog )
								theApp.Log( _T( "Request:\n" ) + QueryInfo( pFile, HTTP_QUERY_RAW_HEADERS_CRLF | HTTP_QUERY_FLAG_REQUEST_HEADERS ) );

							if ( bPOST && szParams && theApp.m_bDebugLog )
								theApp.Log( _T( "Request body:\n" ) + CString( szParams ) );

							if ( theApp.m_bDebugLog )
								theApp.Log( _T( "Response:\n" ) + QueryInfo( pFile, HTTP_QUERY_RAW_HEADERS_CRLF ) );

							if ( GetContent( pFile, aContent ) && theApp.m_bDebugLog )
								theApp.Log( _T( "Response body:\n" ) + CString( CA2T( GetString( aContent ), CP_UTF8 ) ) );

							CString sSetCookie = QueryInfo( pFile, HTTP_QUERY_SET_COOKIE );
							if ( ! sSetCookie.IsEmpty() )
							{
								CString sName, sValue;
								if ( ParseCookie( sSetCookie, sName, sValue ) )
								{
									theApp.AddCookie( sName, sValue );
									theApp.Log( _T( "Detected cookie: " ) + sName + _T( "=" ) + sValue );
								}
							}

							sLocation = QueryInfo( pFile, HTTP_QUERY_LOCATION );
						}
					}
					catch( ... ){}

					pFile->Close();
				}
			}
			catch ( ... ) {}

			pConnection->Close();
		}
	}
	catch ( ... ) {}

	return nStatus;
}

BOOL CSLAPDlg::WebLogin(CInternetSession* pInternet)
{
	DWORD nStatus;
	CStringA sContent;
	CByteArray aContent;
	CString sLocation;

	CString sUsername, sPassword;
	if ( ! theApp.LoadPassword( sUsername, sPassword ) || sUsername.IsEmpty() || sPassword.IsEmpty() )
	{
		// User name or password not set
		SetStatus( IDS_NOPASSWORD );
		return FALSE;
	}

	// Get Login form for CSRF token
	SetStatus( IDS_LOGINING_TOKEN );

	m_sReferer = _T("https://secondlife.com/");
	theApp.ClearCookies();

	CString sUrl = _T("https://id.secondlife.com/openid/login");
	nStatus = WebRequest( pInternet, sUrl, m_sReferer, aContent, sLocation );

	// Some redirects
	for ( int redirects = 1; IsWorkEnabled() && ! sLocation.IsEmpty() && nStatus / 100 == 3 && redirects < 10; ++redirects )
	{
		theApp.LogFormat( _T( "Redirecting #%d to: %s" ), redirects, (LPCTSTR)sLocation );

		m_sReferer = sUrl;
		sUrl = sLocation;
		nStatus = WebRequest( pInternet, sUrl, m_sReferer, aContent, sLocation );
	}
	if ( nStatus / 100 != 2 )
	{
		// HTTP error
		SetStatus( IDS_LOGIN_ERROR_TOKEN );
		m_sReferer.Empty();
		return FALSE;
	}

	// Looking for CSRF token
	const CStringA sTokenName = "csrfmiddlewaretoken";
	sContent = GetString( aContent );
	CString sToken;
	for ( int nStart = 0; nStart < sContent.GetLength(); ++nStart )
	{
		CStringA sPart = sContent.Mid( nStart ).SpanExcluding( ">" );
		if ( const int nSize = sPart.GetLength() )
		{
			sPart.Remove( '\r' );
			sPart.Remove( '\n' );
			sPart.Trim( "\t " );
			if ( _strnicmp( sPart, "<input ", 7 ) == 0 )
			{
				const CStringA sName = GetValue( sPart, "name" );
				if ( sName.CompareNoCase( sTokenName ) == 0 )
				{
					sToken = GetValue( sPart, "value" );
					break;
				}
			}
			nStart += nSize;
		}
	}
	if ( sToken.IsEmpty() )
	{
		// CSRF token not found
		SetStatus( IDS_LOGIN_ERROR_TOKEN );
		m_sReferer.Empty();
		return FALSE;
	}

	// Submit Login form
	SetStatus( IDS_LOGINING );

	m_sReferer = sUrl;
	sUrl = _T("https://id.secondlife.com/openid/loginsubmit");
	const CStringA sParams = sTokenName + "=" + URLEncode( CT2A( sToken, CP_UTF8 ) ) + "&"
		"username=" + URLEncode( CT2A( sUsername, CP_UTF8 ) ) + "&"
		"password=" + URLEncode( CT2A( sPassword, CP_UTF8 ) ) + "&"
		"Submit=&"
		"stay_logged_in=stay_logged_in&"
		"return_to=" + URLEncode( CT2A( _T("https://secondlife.com/my/") ) ) +"&"
		"previous_language=en_US&"
		"language=en_US&"
		"show_join=True&"
		"from_amazon=";
	nStatus = WebRequest( pInternet, sUrl, m_sReferer, aContent, sLocation, sParams, TRUE );
	if ( theApp.GetCookie( _T("agni_sl_session_id") ).IsEmpty() )
	{
		// Login cookie not found
		SetStatus( IDS_LOGIN_ERROR );
		m_sReferer.Empty();
		return FALSE;
	}

	// Some redirects
	for ( int redirects = 1; IsWorkEnabled() && ! sLocation.IsEmpty() && nStatus / 100 == 3 && redirects < 10; ++redirects )
	{
		theApp.LogFormat( _T( "Redirecting #%d to: %s" ), redirects, (LPCTSTR)sLocation );

		m_sReferer = sUrl;
		sUrl = sLocation;
		nStatus = WebRequest( pInternet, sUrl, m_sReferer, aContent, sLocation );
	}
	if ( ! IsWorkEnabled() || nStatus / 100 != 2 )
	{
		// HTTP error
		SetStatus( IDS_LOGIN_ERROR );
		m_sReferer.Empty();
		return FALSE;
	}

	// Until OpenId form
	SetStatus( IDS_LOGINING_OPENID );

	sContent = GetString( aContent );
	if ( sContent.Find( "OpenId transaction in progress" ) == -1 )
	{
		// OpenId form not found
		SetStatus( IDS_LOGIN_ERROR_OPENID );
		m_sReferer.Empty();
		return FALSE;
	}

	// Parse OpenId form
	m_sReferer = sUrl;
	sUrl.Empty();
	CStringA sOpenIdParams;
	BOOL bPOST = FALSE;
	for ( int nStart = 0; nStart < sContent.GetLength(); ++nStart )
	{
		CStringA sPart = sContent.Mid( nStart ).SpanExcluding( ">" );
		if ( const int nSize = sPart.GetLength() )
		{
			sPart.Remove( '\r' );
			sPart.Remove( '\n' );
			sPart.Trim( "\t " );
			if ( _strnicmp( sPart, "<form ", 6 ) == 0 )
			{
				sUrl = GetValue( sPart, "action" );
				bPOST = ( GetValue( sPart, "method" ).CompareNoCase( "POST" ) == 0 );
			}
			else if ( _strnicmp( sPart, "<input ", 7 ) == 0 )
			{
				CStringA sName = GetValue( sPart, "name" );
				if ( sName.IsEmpty() ) sName = "Submit";
				CStringA sValue = GetValue( sPart, "value" );
				if ( ! sOpenIdParams.IsEmpty() ) sOpenIdParams += "&";
				sOpenIdParams += sName + "=" + URLEncode( sValue );
			}
			nStart += nSize;
		}
	}

	if ( sUrl.IsEmpty() || sOpenIdParams.IsEmpty() )
	{
		// OpenId form not found
		SetStatus( IDS_LOGIN_ERROR_OPENID );
		m_sReferer.Empty();
		return FALSE;
	}

	// Complete OpenId login
	nStatus = WebRequest( pInternet, sUrl, m_sReferer, aContent, sLocation, sOpenIdParams, bPOST );

	// Some redirects
	for ( int redirects = 1; IsWorkEnabled() && ! sLocation.IsEmpty() && nStatus / 100 == 3 && redirects < 10; ++redirects )
	{
		theApp.LogFormat( _T( "Redirecting #%d to: %s" ), redirects, (LPCTSTR)sLocation );

		m_sReferer = sUrl;
		sUrl = sLocation;
		nStatus = WebRequest( pInternet, sUrl, m_sReferer, aContent, sLocation );
	}
	if ( nStatus / 100 != 2 || ! theApp.HasCookies() || _tcsnicmp( sUrl, _P("https://secondlife.com/my/") ) != 0 )
	{
		SetStatus( IDS_LOGIN_ERROR_COMPLETE );
		m_sReferer.Empty();
		return FALSE;
	}

	// OK
	m_sReferer = sUrl;
	return TRUE;
}

BOOL CSLAPDlg::WebUpdate(CInternetSession* pInternet)
{
	BOOL bRet = FALSE;

	if ( ! IsWorkEnabled() )
	{
		return FALSE;
	}

	SetStatus( IDS_UPDATING_FRIENDS_ONLINE );
	if ( WebFriendsOnline( pInternet, _T("https://secondlife.com/my/account/friends.php") ) )
	{
		bRet = TRUE;
	}
	else
	{
		SetStatus( IDS_FRIENDS_ONLINE_URL_FAILED );
	}

	if ( ! IsWorkEnabled() )
	{
		return FALSE;
	}

	SetStatus( IDS_UPDATING_FRIENDS );
	// Old widget: _T("https://secondlife.com/my/loadWidgetContent.php?widget=widgetFriends")
	if ( WebFriendsWidget( pInternet, _T("https://secondlife.com/my/widget-friends.php") ) )
	{
		bRet = TRUE;
	}
	else
	{
		SetStatus( IDS_FRIENDS_URL_FAILED );
	}

	return bRet;
}

BOOL CSLAPDlg::WebFriendsOnline(CInternetSession* pInternet, LPCTSTR szUrl)
{
	CByteArray aContent;
	CString sLocation;
	DWORD nStatus = WebRequest( pInternet, szUrl, m_sReferer, aContent, sLocation );
	if ( nStatus / 100 == 2 )
	{
		CStringA sContent = GetString( aContent );
		if ( sContent.Find( "Friends Online" ) != -1 )
		{
			// Parse "Friends Online"
			CString sRealName, sDisplayName, sPlace;
			BOOL bDetected = FALSE;
			for ( int nStart = 0; nStart < sContent.GetLength(); ++nStart )
			{
				CStringA sPart = sContent.Mid( nStart ).SpanExcluding( ">" );
				if ( const int nSize = sPart.GetLength() )
				{
					sPart.Remove( '\r' );
					sPart.Remove( '\n' );
					sPart.Trim( "\t " );

					if ( bDetected )
					{
						if ( _strnicmp( sPart, "<a ", 3 ) == 0 )
						{
							// <a href="SLURL" title='Visit User Name in Second Life!'>
							const CString sHref = static_cast< LPCTSTR >( CA2T( GetValue( sPart, "href" ), CP_UTF8 ) );
							if ( _tcsnicmp( sHref, _T( "secondlife:" ), 11 ) == 0 )
							{
								sPlace = sHref;
							}
						}
						else if ( _strnicmp( sPart.Right( 4 ), "<br/", 4 ) == 0 )
						{
							// Display Name<br/>
							const CStringA str = sPart.Left( sPart.GetLength() - 4 );
							if ( ! str.IsEmpty() )
							{
								sDisplayName = CA2T( str, CP_UTF8 );
							}
						}
						else if ( _strnicmp( sPart.Right( 3 ), "</a", 3 ) == 0 )
						{
							// Display Name</a>
							const CStringA str = sPart.Left( sPart.GetLength() - 3 );
							if ( ! str.IsEmpty() )
							{
								sDisplayName = CA2T( str, CP_UTF8 );
							}
						}
						else if ( _strnicmp( sPart.Right( 7 ), ")</span", 7 ) == 0 )
						{
							// (User Name)</span>
							sRealName = CA2T( sPart.Mid( 1, sPart.GetLength() - 7 - 1 ), CP_UTF8 );
							if ( ! CAvatar::IsValidUsername( sRealName ) )
							{
								sRealName.Empty();
							}
						}
						else if ( _strnicmp( sPart, "<span ", 6 ) == 0 )
						{
							// Skip
						}
						else if ( _strnicmp( sPart.Right( 4 ), "</li", 4 ) == 0 )
						{
							// User Name</li
							if ( sRealName.IsEmpty() )
							{
								sRealName = sPart.Mid( 0, sPart.GetLength() - 4 );
							}
							if ( sRealName.IsEmpty() )
							{
								sRealName = sDisplayName;
							}
							if ( CAvatar::IsValidUsername( sRealName ) )
							{
								if ( sDisplayName.IsEmpty() )
									sDisplayName = sRealName;

								theApp.SetAvatar( sRealName, sDisplayName, sPlace, TRUE, TRUE );
							}
							bDetected = FALSE;
						}
						else
							// Error
							bDetected = FALSE;
					}
					else if ( _strnicmp( sPart, "<li style=", 10 ) == 0 )
					{
						// <li style="">Display Name<br/>
						sPlace.Empty();
						sRealName.Empty();
						sDisplayName.Empty();
						bDetected = TRUE;
					}
					if ( ! bDetected )
					{
						sPlace.Empty();
						sRealName.Empty();
						sDisplayName.Empty();
					}
					nStart += nSize;
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSLAPDlg::WebFriendsWidget(CInternetSession* pInternet, LPCTSTR szUrl)
{
	CByteArray aContent;
	CString sLocation;
	DWORD nStatus = WebRequest( pInternet, szUrl, m_sReferer, aContent, sLocation );
	if ( nStatus / 100 == 2 )
	{
		CStringA sContent = GetString( aContent );
		if ( sContent.Find( "widgetFriendsOnlineContent" ) != -1 )
		{
			// Parse "Friends Widget":
			// <td class="trigger {online/offline} friend"><a href="secondlife://{Place}">{DisplayName}</a><br><span class="username">({UserName})</span></td>
			// <td class="trigger {online/offline} friend"><a href="secondlife://{Place}">{DisplayName = UserName}</a></td>
			// <td class="trigger {online/offline} friend"><span title="{UserFullName}">{DisplayName}</span></td>
			// <td class="trigger {online/offline} friend"><span title="{UserFullName}">{DisplayName}</span><br><span class="username">({UserName})</span></td>

			CString sDisplayName, sRealName, sPlace;
			BOOL bOnline = FALSE;
			BOOL bDetected = FALSE;
			BOOL bUsername = FALSE;
			for ( int nStart = 0; nStart < sContent.GetLength(); ++nStart )
			{
				CStringA sPart = sContent.Mid( nStart ).SpanExcluding( ">" );
				if ( const int nSize = sPart.GetLength() )
				{
					sPart.Remove( '\r' );
					sPart.Remove( '\n' );
					sPart.Trim( "\t " );

					if ( ! bDetected )
					{
						// Parse online status: <td class="trigger {online/offline} friend"
						if ( _strnicmp( sPart, "<td ", 4 ) == 0 )
						{
							const CStringA sClass = GetValue( sPart, "class" );
							if ( sClass.CompareNoCase( "trigger online friend" ) == 0 )
							{
								bOnline = TRUE;
								bDetected = TRUE;
							}
							else if ( sClass.CompareNoCase( "trigger offline friend" ) == 0 )
							{
								bOnline = FALSE;
								bDetected = TRUE;
							}
							ASSERT( sDisplayName.IsEmpty() );
							sDisplayName.Empty();
							ASSERT( sRealName.IsEmpty() );
							sRealName.Empty();
							ASSERT( sPlace.IsEmpty() );
							sPlace.Empty();
						}
					}
					// Parse avatar place: <a href="secondlife://{Place}"
					else if ( _strnicmp( sPart, "<a ", 3 ) == 0 )
					{
						const CString sHref = static_cast< LPCTSTR >( CA2T( GetValue( sPart, "href" ), CP_UTF8 ) );
						if ( _tcsnicmp( sHref, _T( "secondlife:" ), 11 ) == 0 )
						{
							sPlace = sHref;
						}
						else
						{
							ASSERT( FALSE );
						}
					}
					// Parse avatar name:
					// <span title="{UserFullName}"
					// <span class="username"
					else if ( _strnicmp( sPart, "<span ", 6 ) == 0 )
					{
						// Load avatar real name
						CString str = static_cast< LPCTSTR >( CA2T( GetValue( sPart, "title" ), CP_UTF8 ) );
						if ( ! str.IsEmpty() && CAvatar::IsValidUsername( str ) )
						{
							sRealName = str;
						}
						else
						{
							bUsername = TRUE;
						}
					}
					// End
					else if ( _strnicmp( sPart, "</td", 4 ) == 0 )
					{
						if ( sRealName.IsEmpty() && ! sDisplayName.IsEmpty() )
						{
							sRealName = sDisplayName;
						}
						if ( sDisplayName.IsEmpty() )
						{
							sDisplayName = CAvatar::MakePretty( sRealName );
						}
						if ( CAvatar::IsValidUsername( sRealName ) )
						{
							theApp.SetAvatar( sRealName, sDisplayName, sPlace, bOnline, TRUE );
						}
						else
						{
							ASSERT( FALSE );
						}
						sDisplayName.Empty();
						sRealName.Empty();
						sPlace.Empty();
						bOnline = FALSE;
						bDetected = FALSE;
					}
					// {DisplayName = UserName}</a
					// {DisplayName}</span
					// ({UserName})</span
					// <br
					else
					{
						// Load avatar display name
						const int pos = sPart.ReverseFind( '<' );
						if ( pos > 0 )
						{
							const CString str = static_cast< LPCTSTR >( CA2T( sPart.Left( pos ), CP_UTF8 ) );
							if ( sDisplayName.IsEmpty() )
							{
								sDisplayName = str;
							}
							else if ( sRealName.IsEmpty() && str.GetAt( 0 ) == _T('(') && str.GetAt( str.GetLength() - 1 ) == _T(')') )
							{
								sRealName = str.Mid( 1, str.GetLength() - 2 );
							}
						}
					}

					nStart += nSize;
				}
			}
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CSLAPDlg::WebGetImage(CInternetSession* pInternet)
{
	BOOL bRet = FALSE;
	CString sRealName;
	CAvatar* pAvatar;

	{
		CSingleLock pLock( &theApp.m_pSection, TRUE );

		pAvatar = theApp.GetEmptyAvatar();
		if ( ! pAvatar )
			// No work
			return FALSE;

		pAvatar->m_tImage = CTime::GetCurrentTime();
		pAvatar->Save();

		sRealName = pAvatar->m_sRealName;
	}

	CString sUrl = _T("https://my-secondlife-agni.akamaized.net/users/{USERNAME}/thumb_sl_image.png");
	//CString sUrl = _T("https://my-secondlife.s3.amazonaws.com/users/{USERNAME}/sl_image.png");
	sRealName.Replace( _T( ' ' ), _T( '.' ) );
	sUrl.Replace( _T( "{USERNAME}" ), sRealName );

	CByteArray aContent;
	CString sLocation;
	DWORD nStatus = WebRequest( pInternet, sUrl, _T( "" ), aContent, sLocation, NULL, FALSE, FALSE );
	if ( ! IsWorkEnabled() || nStatus / 100 != 2 )
	{
		// HTTP error
		theApp.LogFormat( _T( "Avatar \"%s\" HTTP error: %d" ), (LPCTSTR)sRealName, nStatus );
		return FALSE;
	}

	if ( aContent.GetSize() < 16 || memcmp( aContent.GetData() + 1, "PNG", 3 ) != 0 )
	{
		// PNG not found
		theApp.LogFormat( _T( "Avatar \"%s\" format error" ), (LPCTSTR)sRealName );
		return FALSE;
	}

	if ( HGLOBAL hGlobal = GlobalAlloc( GHND, aContent.GetSize() ) )
	{
		if ( LPBYTE lpByte = (LPBYTE)GlobalLock( hGlobal ) )
		{
			CopyMemory( lpByte, aContent.GetData(), aContent.GetSize() );
			GlobalUnlock( hGlobal );
		}
		IStream* pStream = NULL;
		if ( SUCCEEDED( ::CreateStreamOnHGlobal( hGlobal, FALSE, &pStream ) ) )
		{
			CImage img;
			if ( SUCCEEDED( img.Load( pStream ) ) )
			{
				// Resize image
				CBitmap bmp;
				if ( CDC* pDC = GetDC() )
				{
					if ( bmp.CreateCompatibleBitmap( pDC, pAvatar->m_nBoxSize, pAvatar->m_nBoxSize ) )
					{
						CDC dcMem;
						if ( dcMem.CreateCompatibleDC( pDC ) )
						{
							CBitmap* pbmpOld = static_cast< CBitmap* >( dcMem.SelectObject( &bmp ) );

							bRet = img.Draw( dcMem.m_hDC, CRect( 0, 0, pAvatar->m_nBoxSize, pAvatar->m_nBoxSize ), Gdiplus::InterpolationMode::InterpolationModeHighQuality );

							dcMem.SelectObject( pbmpOld );
						}
						dcMem.DeleteDC();
					}

					ReleaseDC( pDC );
				}
				img.Destroy();

				if ( bRet )
				{
					CSingleLock pLock( &theApp.m_pSection, TRUE );

					if ( theApp.IsValid( pAvatar ) )
					{
						pAvatar->SetImage( (HBITMAP)bmp.Detach() );

						theApp.Refresh( TRUE, pAvatar );
					}
				}

				if ( bmp.m_hObject )
					bmp.DeleteObject();
			}
			pStream->Release();
		}

		GlobalFree( hGlobal );
	}

	return bRet;
}

void CSLAPDlg::SetStatus(UINT nStatus)
{
	SetStatus( LoadString( nStatus ) );
}

void CSLAPDlg::SetStatus(LPCTSTR szStatus)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	m_sStatus = szStatus;
}

CString CSLAPDlg::GetStatus() const
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	return m_sStatus;
}

void CSLAPDlg::AddWork(Work work)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	if ( m_pWorkQueue.Find( work ) == NULL )
	{
		m_pWorkQueue.AddTail( work );
	}
}

CSLAPDlg::Work CSLAPDlg::GetNextWork()
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	return m_pWorkQueue.GetCount() ? m_pWorkQueue.RemoveHead() : Work::Idle;
}

void CSLAPDlg::Start()
{
	m_eventThreadStop.ResetEvent();

	ASSERT( m_pThread == NULL );
	m_pThread = AfxBeginThread( ThreadFn, this );
}

void CSLAPDlg::Stop()
{
	// Ask thread termination
	m_eventThreadStop.SetEvent();

	// Wait for termination
	while ( m_pThread && WaitForSingleObject( m_pThread->m_hThread, 100 ) == WAIT_TIMEOUT )
	{
		// Message loop
		MSG msg;
		while ( PeekMessage( &msg, NULL, NULL, NULL, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		if ( GetWindowThreadProcessId( AfxGetMainWnd()->GetSafeHwnd(), NULL ) == GetCurrentThreadId() )
		{
			LONG lIdle = 0;
			while ( AfxGetApp()->OnIdle( lIdle++ ) );
		}
	}

	// Clean-up
	delete m_pThread;
	m_pThread = NULL;
}

BOOL CSLAPDlg::IsWorkEnabled() const
{
	return ( WaitForSingleObject( m_eventThreadStop, 0 ) == WAIT_TIMEOUT );
}

UINT __cdecl CSLAPDlg::ThreadFn( LPVOID pParam )
{
	reinterpret_cast< CSLAPDlg* >( pParam )->Thread();

	return 0;
}

void CSLAPDlg::Thread()
{
	m_pThread->m_bAutoDelete = FALSE;

	if ( SUCCEEDED( CoInitialize( NULL ) ) )
	{
		// Initialize WinInet
		CAutoPtr< CInternetSession > pInternet;
		try
		{
			pInternet.Attach( new CInternetSession( CString( theApp.m_pszAppName ) + _T("/") + theApp.sVersion ) );
		}
		catch ( ... ) {}

		if ( pInternet && *pInternet )
		{
			while ( IsWorkEnabled() )
			{
				// Relaxing
				SwitchToThread();
				Sleep( 250 );

				Work work = GetNextWork();
				switch ( work )
				{
				case Work::Idle:
					WebGetImage( pInternet );
					break;

				case Work::Login:
					if ( WebLogin( pInternet ) )
					{
						AddWork( Work::Update );
					}
					else
					{
						theApp.Refresh( FALSE );
					}
					break;

				case Work::Update:
					if ( theApp.HasCookies() && WebUpdate( pInternet ) )
					{
						theApp.Refresh( TRUE );
						break;
					}
					// Vital cookies are absent or login expired, so re-login first
					AddWork( Work::Login );
					break;
				}
			}

			pInternet.Free();
		}
		else
		{
			SetStatus( IDP_WININET_INIT_FAILED );
			theApp.Refresh( FALSE );
		}

		theApp.SaveAvatars();
	}

	CoUninitialize();

}
