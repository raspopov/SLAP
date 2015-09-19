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

CIconEdit::CIconEdit(UINT id)
	: m_hIcon( NULL )
{
	SetIcon( id );
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

	return ( m_hIcon != NULL ) || ! id;
}

BEGIN_MESSAGE_MAP(CIconEdit, CEdit)
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
END_MESSAGE_MAP()

// CIconEdit message handlers

void CIconEdit::OnPaint()
{
	CRect rect;
	GetClientRect( &rect );

	const int x = rect.right;
	const int y = rect.top + ( rect.Height() - ICON_SIZE ) / 2;

	__super::OnPaint();

	if ( m_hIcon )
	{
		if ( CDC* pDC = GetDC() )
		{
			pDC->FillSolidRect( x, y, ICON_SIZE, ICON_SIZE, pDC->GetBkColor() );
			::DrawIconEx( pDC->m_hDC, x, y, m_hIcon, ICON_SIZE, ICON_SIZE, 0, NULL, DI_NORMAL );
			ReleaseDC( pDC );
		}
	}
}

void CIconEdit::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	lpncsp->rgrc->right -= ICON_SIZE + HIWORD( GetMargins() );

	__super::OnNcCalcSize( bCalcValidRects, lpncsp );
}
