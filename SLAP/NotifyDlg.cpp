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
{
	if ( Create( CNotifyDlg::IDD, GetDesktopWindow() ) )
	{
		SetWindowText( szTitle );
		m_wndText.SetWindowText( szText );

		AnimateWindow( 500, AW_ACTIVATE | AW_BLEND );
		FlashWindowEx( FLASHW_ALL, INFINITE, 0 );
	}
}

void CNotifyDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_TEXT, m_wndText );
}

BEGIN_MESSAGE_MAP(CNotifyDlg, CDialog)
END_MESSAGE_MAP()

BOOL CNotifyDlg::OnInitDialog()
{
	__super::OnInitDialog();

	Translate( GetSafeHwnd(), CNotifyDlg::IDD );

	LOGFONT lf = {};
	SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof( lf ), &lf, 0 );
	lf.lfWeight = FW_SEMIBOLD;
	lf.lfHeight -= 2;
	m_pBigFont.CreateFontIndirect( &lf );

	m_wndText.SetFont( &m_pBigFont );

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

// CNotifyDlg message handlers

void CNotifyDlg::OnOK()
{
	theApp.Notify( NULL );

	if ( m_pBigFont.m_hObject )
		m_pBigFont.DeleteObject();

	__super::OnOK();
}

void CNotifyDlg::OnCancel()
{
	theApp.Notify( NULL );

	if ( m_pBigFont.m_hObject )
		m_pBigFont.DeleteObject();

	__super::OnCancel();
}
