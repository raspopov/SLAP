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
#include "SLAPDlg.h"
#include "UsersBox.h"

// CUsersBox

IMPLEMENT_DYNAMIC(CUsersBox, CListBox)

BEGIN_MESSAGE_MAP(CUsersBox, CListBox)
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CUsersBox message handlers

void CUsersBox::OnRButtonUp(UINT nFlags, CPoint point)
{
	__super::OnRButtonUp( nFlags, point );

	((CSLAPDlg*)GetParent())->OnUsersRButtonUp( nFlags, point );
}
