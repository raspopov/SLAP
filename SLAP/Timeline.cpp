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
#include "Timeline.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// CTimeline

IMPLEMENT_DYNAMIC(CTimeline, CStatic)

CTimeline::CTimeline(CAvatar* pAvatar)
	: m_pAvatar( pAvatar )
{
}

BEGIN_MESSAGE_MAP(CTimeline, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()

// CTimeline message handlers

void CTimeline::OnPaint()
{
	CPaintDC dc( this );

	CRect rc;
	GetClientRect( &rc );

	CSingleLock pLock( &theApp.m_pSection, TRUE );

	if ( theApp.IsValid( m_pAvatar ) )
	{
		m_pAvatar->PaintTimeline( &dc, &rc );
	}
}
