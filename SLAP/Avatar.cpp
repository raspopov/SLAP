// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "Avatar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


#define BOX_GAP			4
#define BOX_TIMELINE	9
#define BOX_INDICATOR	7
#define BOX_STATUS		16
#define BOX_ICON		(48 + 2)
#define RGB_ONLINE		RGB( 64, 240, 64 )		// Minimum color value must be 64
#define RGB_OFFLINE		RGB( 200, 200, 200 )	// Minimum color value must be 64
#define RGB_UNKNOWN		RGB( 240, 64, 64 )		// Minimum color value must be 64


// CAvatar

HICON	CAvatar::m_hUserIcon	= NULL;
HICON	CAvatar::m_hSoundIcon	= NULL;
HICON	CAvatar::m_hNoSoundIcon	= NULL;
HICON	CAvatar::m_hAlertIcon	= NULL;
CFont	CAvatar::m_fntBold;
CFont	CAvatar::m_fntNormal;
CFont	CAvatar::m_fntTimeline;
int		CAvatar::m_nItemHeight	= 0;
int		CAvatar::m_nTitleHeight	= 0;
int		CAvatar::m_nTextHeight	= 0;

CAvatar::CAvatar(const CString& sRealName)
	: m_sRealName	( sRealName )
	, m_bOnline		( FALSE )
	, m_bFriend		( FALSE )
	, m_nTimeline	()
	, m_bLoopSound	( FALSE )
	, m_bNewOnline	( FALSE )
	, m_bNewFriend	( FALSE )
	, m_bDirty		( TRUE )
	, m_bFiltered	( TRUE )
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

void CAvatar::Filter( const CString& sFilter )
{
	BOOL bFiltered = sFilter.IsEmpty() || _tcsistr( m_sDisplayName, sFilter ) || _tcsistr( m_sRealName, sFilter );

	if ( bFiltered != m_bFiltered )
	{
		m_bFiltered = bFiltered;
		m_bDirty = TRUE;
	}
}

BOOL CAvatar::IsValidUsername( LPCTSTR szUsername )
{
	if ( !szUsername || !*szUsername )
		return FALSE;

	UINT nLength = 0;
	BOOL bSpace = FALSE;
	for ( LPCTSTR ch = szUsername; *ch; ++ch )
	{
		if ( ( *ch >= _T( '0' ) && *ch <= _T( '9' ) ) || ( *ch >= _T( 'a' ) && *ch <= _T( 'z' ) ) || ( *ch >= _T( 'A' ) && *ch <= _T( 'Z' ) ) ) // Can be alpha-numeric
		{
			if ( ++nLength > 31 ) // Maximum length is 31 symbols
			{
				ASSERT( FALSE );
				return FALSE;
			}
		}
		else if ( *ch == _T( ' ' ) && ch != szUsername && *( ch + 1 ) && !bSpace ) // Can be one space in the middle
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

CString CAvatar::MakePretty( const CString& sName )
{
	int nPos = sName.FindOneOf( _T( " ." ) );
	if ( nPos != -1 )
		return sName.Left( 1 ).MakeUpper() + sName.Mid( 1, nPos ).MakeLower() + _T( " " ) +
		sName.Mid( nPos + 1, 1 ).MakeUpper() + sName.Mid( nPos + 2 ).MakeLower();
	else
		return sName.Left( 1 ).MakeUpper() + sName.Mid( 1 ).MakeLower(); // No "Resident"
}

int CAvatar::OnMeasureItem(CWnd* pWnd)
{
	if ( ! m_nItemHeight )
	{
		// Load icons
		if ( ! m_hUserIcon )
			m_hUserIcon = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_USER ), IMAGE_ICON, 48, 48, LR_DEFAULTCOLOR );
		if ( ! m_hSoundIcon )
			m_hSoundIcon = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_SOUND ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
		if ( ! m_hNoSoundIcon )
			m_hNoSoundIcon = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_NOSOUND ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );
		if ( ! m_hAlertIcon )
			m_hAlertIcon = (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_ALERT ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR );

		// Create fonts
		LOGFONT lf = {};
		SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof( lf ), &lf, 0 );
		lf.lfWeight = FW_SEMIBOLD;
		if ( ! m_fntBold.m_hObject ) m_fntBold.CreateFontIndirect( &lf );
		lf.lfHeight += 1;
		lf.lfWeight = FW_NORMAL;
		if ( ! m_fntNormal.m_hObject ) m_fntNormal.CreateFontIndirect( &lf );
		lf.lfHeight += 2;
		if ( ! m_fntTimeline.m_hObject ) m_fntTimeline.CreateFontIndirect( &lf );

		// Calculate text metrics
		if ( CDC* pDC = pWnd->GetDC() )
		{
			CFont* pOldFont = static_cast< CFont* >( pDC->SelectObject( &m_fntBold ) );
			m_nTitleHeight = pDC->GetTextExtent( _T( "Xg|" ) ).cy + 1;
			pDC->SelectObject( &m_fntNormal );
			m_nTextHeight = pDC->GetTextExtent( _T( "Xg|" ) ).cy + 1;

			pDC->SelectObject( pOldFont );
			pWnd->ReleaseDC( pDC );
		}

		// Calculate listbox item height
		m_nItemHeight = BOX_GAP + max( BOX_ICON, m_nTitleHeight + m_nTextHeight * 2 + BOX_TIMELINE + 1 ) + BOX_GAP;
	}

	return m_nItemHeight;
}

void CAvatar::OnDrawItem(LPDRAWITEMSTRUCT pDIS) const
{
	const BOOL bSelected = ( pDIS->itemState & ODS_SELECTED );
	const COLORREF rgbIndicator = m_bFriend ? ( m_bOnline ? RGB_ONLINE : RGB_OFFLINE ) : RGB_UNKNOWN;
	const COLORREF rgbIndicatorBack = RGB(
		max( 64, GetRValue( rgbIndicator ) ) - 64,
		max( 64, GetGValue( rgbIndicator ) ) - 64,
		max( 64, GetBValue( rgbIndicator ) ) - 64 );
	const COLORREF rgbBack = GetSysColor( COLOR_BTNFACE );
	const int cx = pDIS->rcItem.right - pDIS->rcItem.left;
	const int cy = pDIS->rcItem.bottom - pDIS->rcItem.top;

	CDC* pDC = CDC::FromHandle( pDIS->hDC );
	pDC->SetBkMode( TRANSPARENT );
	pDC->SetTextColor( GetSysColor( COLOR_BTNTEXT ) );
	pDC->SetBkColor( rgbBack );

	TRIVERTEX vertex[ 2 ] =
	{
		{ pDIS->rcItem.left, pDIS->rcItem.top,
			(COLOR16)( GetRValue( rgbBack ) << 8 ),
			(COLOR16)( GetGValue( rgbBack ) << 8 ),
			(COLOR16)( GetBValue( rgbBack ) << 8 ),
			(COLOR16)0 },
		{ pDIS->rcItem.right, pDIS->rcItem.bottom,
			(COLOR16)( ( max( 16, GetRValue( rgbBack ) ) - 16 ) << 8 ),
			(COLOR16)( ( max( 16, GetGValue( rgbBack ) ) - 16 ) << 8 ),
			(COLOR16)( ( max( 16, GetBValue( rgbBack ) ) - 16 ) << 8 ),
			(COLOR16)0 }
	};
	GRADIENT_RECT gRect = { 0, 1 };
	pDC->GradientFill( vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V );

	const CRect rcIndicator(
		pDIS->rcItem.left + BOX_GAP,
		pDIS->rcItem.top + BOX_GAP,
		pDIS->rcItem.left + BOX_GAP + BOX_INDICATOR,
		pDIS->rcItem.top + BOX_GAP + BOX_ICON );
	TRIVERTEX vertexIndicator[ 2 ] =
	{
		{ rcIndicator.left, rcIndicator.top,
			(COLOR16)( GetRValue( rgbIndicator ) << 8 ),
			(COLOR16)( GetGValue( rgbIndicator ) << 8 ),
			(COLOR16)( GetBValue( rgbIndicator ) << 8 ),
			(COLOR16)0 },
		{ rcIndicator.right, rcIndicator.bottom,
			(COLOR16)( GetRValue( rgbIndicatorBack ) << 8 ),
			(COLOR16)( GetGValue( rgbIndicatorBack ) << 8 ),
			(COLOR16)( GetBValue( rgbIndicatorBack ) << 8 ),
			(COLOR16)0 }
	};
	GRADIENT_RECT gRectIndicator = { 0, 1 };
	pDC->GradientFill( vertexIndicator, 2, &gRectIndicator, 1, GRADIENT_FILL_RECT_H );

	CRect rcIcon(
		rcIndicator.right,
		rcIndicator.top,
		rcIndicator.right + BOX_ICON,
		rcIndicator.bottom );
	pDC->Draw3dRect( &rcIcon, rgbIndicatorBack, rgbIndicatorBack );
	rcIcon.DeflateRect( 1, 1, 1, 1 );
	if ( m_pImage.IsNull() )
		DrawIconEx( pDIS->hDC, rcIcon.left, rcIcon.top, m_hUserIcon, 48, 48, NULL, NULL, DI_NORMAL );
	else
		m_pImage.Draw( pDIS->hDC, rcIcon.left, rcIcon.top );

	CRect rcTitle(
		rcIndicator.right + BOX_ICON + BOX_GAP,
		pDIS->rcItem.top + BOX_GAP,
		pDIS->rcItem.right - BOX_GAP - BOX_STATUS - 1,
		pDIS->rcItem.top + BOX_GAP + m_nTitleHeight );
	CFont* pOldFont = static_cast< CFont* >( pDC->SelectObject( &m_fntBold ) );
	pDC->DrawText( m_sDisplayName, &rcTitle, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOPREFIX | DT_END_ELLIPSIS );

	const CPoint rcSoundIcon(
		pDIS->rcItem.right - BOX_GAP - BOX_STATUS,
		pDIS->rcItem.top + BOX_GAP );
	if ( !m_sOnlineSound.IsEmpty() )
	{
		if ( m_sOnlineSound == NO_SOUND )
			DrawIconEx( pDIS->hDC, rcSoundIcon.x, rcSoundIcon.y, m_hNoSoundIcon, 16, 16, NULL, NULL, DI_NORMAL );
		else
			DrawIconEx( pDIS->hDC, rcSoundIcon.x, rcSoundIcon.y, m_hSoundIcon, 16, 16, NULL, NULL, DI_NORMAL );
	}
	if ( m_bLoopSound )
		DrawIconEx( pDIS->hDC, rcSoundIcon.x, rcSoundIcon.y + 17, m_hAlertIcon, 16, 16, NULL, NULL, DI_NORMAL );

	CRect rcRealName(
		rcTitle.left,
		rcTitle.bottom,
		rcTitle.right,
		rcTitle.bottom + m_nTextHeight );
	pDC->SelectObject( &m_fntNormal );
	pDC->DrawText( _T( "(" ) + m_sRealName + _T( ")" ), &rcRealName, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOPREFIX | DT_END_ELLIPSIS );

	CString sOnline;
	if ( !m_bOnline && m_tLastOnline.GetTime() )
	{
		sOnline = m_tLastOnline.Format( IDS_AVATAR_LAST_ONLINE );
	}
	else if ( !m_sPlace.IsEmpty() )
	{
		pDC->SetTextColor( GetSysColor( COLOR_HOTLIGHT ) );
		sOnline = _T( "@ " ) + m_sPlace;
	}
	if ( !sOnline.IsEmpty() )
	{
		CRect rcOnline(
			rcRealName.left,
			rcRealName.bottom,
			rcRealName.right,
			rcRealName.bottom + m_nTextHeight );
		pDC->DrawText( sOnline, &rcOnline, DT_LEFT | DT_SINGLELINE | DT_TOP | DT_NOPREFIX | DT_END_ELLIPSIS );
	}

	const CRect rcTimeline(
		rcIcon.right + BOX_GAP,
		pDIS->rcItem.bottom - BOX_GAP - BOX_TIMELINE,
		pDIS->rcItem.right - BOX_GAP - ( ( m_nTitleHeight + m_nTextHeight * 2 > 16 * 2 + 3 ) ? 0 : ( BOX_STATUS + 1 ) ),
		pDIS->rcItem.bottom - BOX_GAP );
	PaintTimeline( pDC, &rcTimeline );

	if ( bSelected )
	{
		CDC dcSelection;
		dcSelection.CreateCompatibleDC( pDC );
		CBitmap bmSelection;
		bmSelection.CreateCompatibleBitmap( pDC, cx, cy );
		CBitmap* pOldBitmap = static_cast< CBitmap* >( dcSelection.SelectObject( &bmSelection ) );
		dcSelection.FillSolidRect( 0, 0, cx, cy, GetSysColor( COLOR_HIGHLIGHT ) );
		const BLENDFUNCTION fn = { AC_SRC_OVER, 0, 48, 0 };
		pDC->AlphaBlend( pDIS->rcItem.left, pDIS->rcItem.top, cx, cy, &dcSelection, 0, 0, cx, cy, fn );
		dcSelection.SelectObject( pOldBitmap );
		bmSelection.DeleteObject();
		dcSelection.DeleteDC();
	}

	pDC->SelectObject( pOldFont );

	pDC->ExcludeClipRect( &pDIS->rcItem );
}

void CAvatar::PaintTimeline(CDC* pDC, const CRect* pRect) const
{
	const BOOL bExtended = ( pRect->Height() > 24 );

	DWORD nMax = m_nTimeline[ 0 ], nMin = m_nTimeline[ 0 ];
	for ( int i = 1; i < 24; ++i )
	{
		if ( nMax < m_nTimeline[ i ] )
			nMax = m_nTimeline[ i ];
		if ( nMin > m_nTimeline[ i ] )
			nMin = m_nTimeline[ i ];
	}
	const ULONGLONG d = max( ( nMax - nMin ) / 128, 1 );

	int width = ( pRect->Width() - 2 ) / 24;
	int offset = ( pRect->Width() - 2 - width * 24 ) / 2;

	CRect rcHour, rcText;
	rcHour.top = pRect->top;
	rcHour.bottom = pRect->bottom;

	if ( bExtended )
	{
		rcText.top = rcHour.top + 1;
		//rcHour.top += pRect->Height() / 2;
		rcText.bottom = rcHour.bottom - 1; //.top;
	}

	TCHAR szText[ 16 ] = {};
	SYSTEMTIME time = {};
	CFont* pOldFont = static_cast< CFont* >( pDC->SelectObject( &m_fntTimeline ) );
	pDC->SetBkMode( TRANSPARENT );

	for ( int i = 0; i < 24; ++i )
	{
		if ( i < 12 )
		{
			rcHour.left = pRect->left + offset + width * i;
			rcHour.right = rcHour.left + width - 1;
		}
		else
		{
			rcHour.left = pRect->right - offset - width * ( 24 - i );
			rcHour.right = rcHour.left + width - 1;
		}
		rcText.left = rcHour.left;
		rcText.right = rcHour.right;

		const BYTE nBlend = ( nMax * d ) ? ( 255 - (BYTE)min( ( ( m_nTimeline[ i ] * d ) * 255 ) / ( nMax * d ), 255 ) ) : 255;
		pDC->FillSolidRect( rcHour, RGB( nBlend, nBlend, 255 ) );

		if ( bExtended )
		{
			time.wHour = (WORD)i;
			//MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ), SORT_DEFAULT )
			GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOMINUTESORSECONDS, &time, NULL, szText, _countof( szText ) );
			for ( TCHAR* c = szText; *c; ++c )
				if ( * c == _T(' ') ) *c = _T('\n');
			pDC->SetTextColor( ( nBlend > 127 ) ? RGB( 5, 5, 5 ) : RGB( 250, 250, 250 ) );
			pDC->DrawText( szText, rcText, DT_CENTER | DT_TOP | DT_NOPREFIX );
		}

		pDC->ExcludeClipRect( rcHour );
	}

	pDC->SelectObject( pOldFont );
}

//int CAvatar::GetHour(const CRect* pRect, CPoint pt) const
//{
//	const int width = pRect->Width() - 2;
//
//	CRect rcHour;
//	rcHour.top = pRect->top;
//	rcHour.bottom = pRect->bottom;
//	for ( int i = 0; i < 24; ++i )
//	{
//		if ( i < 12 )
//		{
//			rcHour.left = pRect->left + ( width * i ) / 24;
//			rcHour.right = pRect->left + ( width * ( i + 1 ) ) / 24 - 1;
//		}
//		else
//		{
//			rcHour.left = pRect->right - ( width * ( 24 - i - 1 ) ) / 24 - 1;
//			rcHour.right = pRect->right - ( width * ( 24 - i ) ) / 24;
//		}
//		if ( rcHour.PtInRect( pt ) )
//		{
//			return i;
//		}
//	}
//	return -1;
//}

void CAvatar::Clear()
{
	if ( m_hUserIcon )
	{
		DestroyIcon( m_hUserIcon );
		m_hUserIcon = NULL;
	}
	if ( m_hSoundIcon )
	{
		DestroyIcon( m_hSoundIcon );
		m_hSoundIcon = NULL;
	}
	if ( m_hNoSoundIcon )
	{
		DestroyIcon( m_hNoSoundIcon );
		m_hNoSoundIcon = NULL;
	}
	if ( m_hAlertIcon )
	{
		DestroyIcon( m_hAlertIcon );
		m_hAlertIcon = NULL;
	}

	if ( m_fntBold.m_hObject )
	{
		m_fntBold.DeleteObject();
		ASSERT( m_fntBold.m_hObject == NULL );
	}
	if ( m_fntNormal.m_hObject )
	{
		m_fntNormal.DeleteObject();
		ASSERT( m_fntNormal.m_hObject == NULL );
	}
	if ( m_fntTimeline.m_hObject )
	{
		m_fntTimeline.DeleteObject();
		ASSERT( m_fntTimeline.m_hObject == NULL );
	}

	m_nItemHeight = 0;
	m_nTitleHeight = 0;
	m_nTextHeight = 0;
}

void CAvatar::Delete()
{
	SHDeleteKey( HKEY_CURRENT_USER, theApp.GetAvatarsKey() + _T( "\\" ) + m_sRealName );

	if ( ! theApp.sImageCache.IsEmpty() )
	{
		const CString sCached = theApp.sImageCache + _T( "\\" ) + m_sRealName + _T( ".png" );
		DeleteFile( sCached );
	}
}

BOOL CAvatar::Load()
{
	CRegKey keyAvatar;
	LSTATUS ret = keyAvatar.Create( HKEY_CURRENT_USER, theApp.GetAvatarsKey() + _T( "\\" ) + m_sRealName );
	ASSERT( ret == ERROR_SUCCESS );
	if ( ret == ERROR_SUCCESS )
	{
		m_sDisplayName = RegGetString( keyAvatar );

		ULONG nSize = sizeof( m_nTimeline );
		keyAvatar.QueryBinaryValue( TIMELINE, m_nTimeline, &nSize );

		DWORD bLoop = 0;
		keyAvatar.QueryDWORDValue( LOOP_SOUND, bLoop );
		m_bLoopSound = ( bLoop != 0 );

		DWORD bOnline = 0;
		keyAvatar.QueryDWORDValue( ONLINE, bOnline );
		m_bOnline = ( bOnline != 0 );

		DWORD bFriend = 0;
		keyAvatar.QueryDWORDValue( FRIEND, bFriend );
		m_bFriend = ( bFriend != 0 );

		ULONGLONG nOnline = 0;
		keyAvatar.QueryQWORDValue( LAST_ONLINE, nOnline );
		m_tLastOnline = CTime( (__time64_t)nOnline );

		ULONGLONG nImage = 0;
		keyAvatar.QueryQWORDValue( IMAGE_TIME, nImage );
		m_tImage = CTime( (__time64_t)nImage );

		m_sPlace = RegGetString( keyAvatar, PLACE );
		m_sOnlineSound = RegGetString( keyAvatar, SOUND_ONLINE );
		m_sOfflineSound = RegGetString( keyAvatar, SOUND_OFFLINE );

		if ( ! theApp.sImageCache.IsEmpty() )
		{
			const CString sCached = theApp.sImageCache + _T( "\\" ) + m_sRealName + _T( ".png" );
			if ( FAILED( m_pImage.Load( sCached ) ) )
				// Mark for reload
				m_tLastOnline = CTime();
		}

		return TRUE;
	}
	return FALSE;
}

BOOL CAvatar::Save() const
{
	CRegKey keyAvatar;
	LSTATUS ret = keyAvatar.Create( HKEY_CURRENT_USER, theApp.GetAvatarsKey() + _T( "\\" ) + m_sRealName );
	ASSERT( ret == ERROR_SUCCESS );
	if ( ret == ERROR_SUCCESS )
	{
		keyAvatar.SetStringValue( NULL, m_sDisplayName );
		keyAvatar.SetDWORDValue( ONLINE, m_bOnline ? 1 : 0 );
		keyAvatar.SetDWORDValue( FRIEND, m_bFriend ? 1 : 0 );
		keyAvatar.SetQWORDValue( LAST_ONLINE, (ULONGLONG)(__time64_t)m_tLastOnline.GetTime() );
		keyAvatar.SetQWORDValue( IMAGE_TIME, (ULONGLONG)(__time64_t)m_tImage.GetTime() );
		keyAvatar.SetStringValue( PLACE, m_sPlace );
		keyAvatar.SetStringValue( SOUND_ONLINE, m_sOnlineSound );
		keyAvatar.SetStringValue( SOUND_OFFLINE, m_sOfflineSound );
		keyAvatar.SetDWORDValue( LOOP_SOUND, m_bLoopSound ? 1 : 0 );
		keyAvatar.SetBinaryValue( TIMELINE, m_nTimeline, (ULONG)sizeof( m_nTimeline ) );

		return TRUE;
	}
	return FALSE;
}

BOOL CAvatar::SetImage(HBITMAP hBitmap)
{
	m_pImage.Destroy();
	m_pImage.Attach( hBitmap );

	// Save to cache
	const CString sCached = theApp.sImageCache + _T( "\\" ) + m_sRealName + _T( ".png" );
	SHCreateDirectory( NULL, theApp.sImageCache );
	if ( SUCCEEDED( m_pImage.Save( sCached ) ) )
	{
		theApp.Log( _T( "Successfuly saved image: " ) + sCached );
		return TRUE;
	}
	else
	{
		theApp.Log( _T( "Failed to save image: " ) + sCached );
		return FALSE;
	}
}
