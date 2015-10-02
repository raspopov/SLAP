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
#include "IconEdit.h"

#define ICON_SIZE 16

// CIconEdit

IMPLEMENT_DYNAMIC(CIconEdit, CEdit)

CIconEdit::CIconEdit()
	: m_hIcon( NULL )
{
}

CIconEdit::~CIconEdit()
{
	if ( m_hIcon ) DestroyIcon( m_hIcon );
}

BOOL CIconEdit::SetIcon(UINT id)
{
	if ( m_hIcon )
	{
		DestroyIcon( m_hIcon );
		m_hIcon = NULL;
	}

	if ( id )
		m_hIcon = (HICON)::LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( id ), IMAGE_ICON, ICON_SIZE, ICON_SIZE, LR_DEFAULTCOLOR );

	SetWindowPos( NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED );

	return ( m_hIcon != NULL ) || ! id;
}

BEGIN_MESSAGE_MAP(CIconEdit, CEdit)
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

// CIconEdit message handlers

void CIconEdit::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	lpncsp->rgrc->right -= ICON_SIZE + HIWORD( GetMargins() );

	__super::OnNcCalcSize( bCalcValidRects, lpncsp );
}

void CIconEdit::OnNcPaint()
{
	if ( CDC* pDC = GetDC() )
	{
		CRect rcClient;
		GetClientRect( &rcClient );

		CRect rcWindow;
		GetWindowRect( &rcWindow );
		ScreenToClient( &rcWindow );

		const int cx = HIWORD( GetMargins() );

		pDC->FillSolidRect( rcClient.right, rcClient.top, ICON_SIZE + cx, rcClient.Height(), pDC->GetBkColor() );

		if ( m_hIcon )
		{
			const int x = rcClient.right + cx / 2;
			const int y = rcClient.top + ( rcWindow.Height() - rcClient.Height() ) / 2;
			::DrawIconEx( pDC->m_hDC, x, y, m_hIcon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL );
		}

		pDC->ExcludeClipRect( rcClient.right, rcClient.top, rcClient.right + ICON_SIZE + cx - 1, rcClient.top + rcClient.Height() - 1 );

		ReleaseDC( pDC );
	}

	__super::OnNcPaint();
}
