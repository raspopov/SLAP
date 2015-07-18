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
#include "NotifyDlg.h"

// CNotifyDlg dialog

CNotifyDlg::CNotifyDlg(LPCTSTR szTitle, LPCTSTR szText)
	: CDialog	( CNotifyDlg::IDD, GetDesktopWindow() )
	, m_sText	( szText )
{
	Create( CNotifyDlg::IDD, GetDesktopWindow() );
	SetWindowText( szTitle );
}

void CNotifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange( pDX );

	DDX_Text( pDX, IDC_TEXT, m_sText );
}

BEGIN_MESSAGE_MAP(CNotifyDlg, CDialog)
END_MESSAGE_MAP()

// CNotifyDlg message handlers

void CNotifyDlg::OnOK()
{
	theApp.Notify( NULL );

	CDialog::OnOK();
}

void CNotifyDlg::OnCancel()
{
	theApp.Notify( NULL );

	CDialog::OnCancel();
}
