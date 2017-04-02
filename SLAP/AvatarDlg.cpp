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
#include "AvatarDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// CAvatarDlg dialog

CAvatarDlg::CAvatarDlg(CAvatar* pAvatar, CWnd* pParent /*=NULL*/)
	: CDialog		( CAvatarDlg::IDD, pParent )
	, m_pAvatar		( pAvatar )
	, m_bLoop		( FALSE )
	, m_wndTimeline	( pAvatar )
{
}

void CAvatarDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange( pDX );

	DDX_Control( pDX, IDC_SOUND_ONLINE, m_wndOnlineSound );
	DDX_Control( pDX, IDC_SOUND_OFFLINE, m_wndOfflineSound );
	DDX_Control( pDX, IDC_PLAY_ONLINE, m_wndPlayOnline );
	DDX_Control( pDX, IDC_PLAY_OFFLINE, m_wndPlayOffline );
	DDX_Check( pDX, IDC_LOOP, m_bLoop );
	DDX_Control( pDX, IDC_TIMELINE, m_wndTimeline );
}

BEGIN_MESSAGE_MAP(CAvatarDlg, CDialog)
	ON_BN_CLICKED( IDC_ONLINE_NO_SOUND, &CAvatarDlg::OnBnClickedOnline )
	ON_BN_CLICKED( IDC_ONLINE_DEFAULT, &CAvatarDlg::OnBnClickedOnline )
	ON_BN_CLICKED( IDC_ONLINE_CUSTOM, &CAvatarDlg::OnBnClickedOnline )
	ON_BN_CLICKED( IDC_OFFLINE_NO_SOUND, &CAvatarDlg::OnBnClickedOffline )
	ON_BN_CLICKED( IDC_OFFLINE_DEFAULT, &CAvatarDlg::OnBnClickedOffline )
	ON_BN_CLICKED( IDC_OFFLINE_CUSTOM, &CAvatarDlg::OnBnClickedOffline )
	ON_BN_CLICKED( IDC_PLAY_ONLINE, &CAvatarDlg::OnBnClickedPlayOnline )
	ON_BN_CLICKED( IDC_PLAY_OFFLINE, &CAvatarDlg::OnBnClickedPlayOffline )
	ON_BN_CLICKED( IDC_RESET, &CAvatarDlg::OnBnClickedReset )
END_MESSAGE_MAP()

// CAvatarDlg message handlers

BOOL CAvatarDlg::OnInitDialog()
{
	__super::OnInitDialog();

	CSingleLock pLock( &theApp.m_pSection, TRUE );

	if ( ! theApp.IsValid( m_pAvatar ) )
	{
		EndDialog( IDCANCEL );
		return FALSE;
	}

	Translate( GetSafeHwnd(), CAvatarDlg::IDD );

	SetWindowText( m_pAvatar->m_sDisplayName );

	CString sFilter = LoadString( IDS_SOUNDS_FILTER );

	{
		int nOnline = ( m_pAvatar->m_sOnlineSound.IsEmpty() ? IDC_ONLINE_DEFAULT :
			( m_pAvatar->m_sOnlineSound == NO_SOUND ? IDC_ONLINE_NO_SOUND : IDC_ONLINE_CUSTOM ) );
		CheckRadioButton( IDC_ONLINE_NO_SOUND, IDC_ONLINE_CUSTOM, nOnline );
		
		m_wndOnlineSound.EnableFileBrowseButton( _T( "wav" ), sFilter );
		m_wndOnlineSound.EnableWindow( nOnline == IDC_ONLINE_CUSTOM );
		if ( nOnline == IDC_ONLINE_CUSTOM ) m_wndOnlineSound.SetWindowText( m_pAvatar->m_sOnlineSound );

		m_wndPlayOnline.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_PLAY ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
		m_wndPlayOnline.EnableWindow( nOnline == IDC_ONLINE_CUSTOM );
	}

	{
		int nOffline = ( m_pAvatar->m_sOfflineSound.IsEmpty() ? IDC_OFFLINE_DEFAULT :
			( m_pAvatar->m_sOfflineSound == NO_SOUND ? IDC_OFFLINE_NO_SOUND : IDC_OFFLINE_CUSTOM ) );
		CheckRadioButton( IDC_OFFLINE_NO_SOUND, IDC_OFFLINE_CUSTOM, nOffline );
		
		m_wndOfflineSound.EnableFileBrowseButton( _T( "wav" ), sFilter );
		m_wndOfflineSound.EnableWindow( nOffline == IDC_OFFLINE_CUSTOM );
		if ( nOffline == IDC_OFFLINE_CUSTOM ) m_wndOfflineSound.SetWindowText( m_pAvatar->m_sOfflineSound );

		m_wndPlayOffline.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_PLAY ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
		m_wndPlayOffline.EnableWindow( nOffline == IDC_OFFLINE_CUSTOM );
	}

	m_bLoop = m_pAvatar->m_bLoopSound;

	UpdateData( FALSE );

	return TRUE;
}

void CAvatarDlg::OnOK()
{
	if ( ! UpdateData() )
		return;

	{
		CSingleLock pLock( &theApp.m_pSection, TRUE );

		if ( ! theApp.IsValid( m_pAvatar ) )
		{
			EndDialog( IDCANCEL );
			return;
		}

		{
			int nOnline = GetCheckedRadioButton( IDC_ONLINE_NO_SOUND, IDC_ONLINE_CUSTOM );
			CString sOnlineSound;
			switch ( nOnline )
			{
			case IDC_ONLINE_NO_SOUND:
				sOnlineSound = NO_SOUND;
				break;
			case IDC_ONLINE_DEFAULT:
				break;
			case IDC_ONLINE_CUSTOM:
				m_wndOnlineSound.GetWindowText( sOnlineSound );
				break;
			}
			m_pAvatar->m_sOnlineSound = sOnlineSound;
		}

		{
			int nOffline = GetCheckedRadioButton( IDC_OFFLINE_NO_SOUND, IDC_OFFLINE_CUSTOM );
			CString sOfflineSound;
			switch ( nOffline )
			{
			case IDC_OFFLINE_NO_SOUND:
				sOfflineSound = NO_SOUND;
				break;
			case IDC_OFFLINE_DEFAULT:
				break;
			case IDC_OFFLINE_CUSTOM:
				m_wndOfflineSound.GetWindowText( sOfflineSound );
				break;
			}
			m_pAvatar->m_sOfflineSound = sOfflineSound;
		}

		m_pAvatar->m_bLoopSound = m_bLoop;

		theApp.Refresh( TRUE, m_pAvatar );
	}

	__super::OnOK();
}

void CAvatarDlg::OnBnClickedOnline()
{
	UpdateData();
	
	int nOnline = GetCheckedRadioButton( IDC_ONLINE_NO_SOUND, IDC_ONLINE_CUSTOM );
	m_wndOnlineSound.EnableWindow( nOnline == IDC_ONLINE_CUSTOM );
	m_wndPlayOnline.EnableWindow( nOnline == IDC_ONLINE_CUSTOM );
}

void CAvatarDlg::OnBnClickedOffline()
{
	UpdateData();

	int nOffline = GetCheckedRadioButton( IDC_OFFLINE_NO_SOUND, IDC_OFFLINE_CUSTOM );
	m_wndOfflineSound.EnableWindow( nOffline == IDC_OFFLINE_CUSTOM );
	m_wndPlayOffline.EnableWindow( nOffline == IDC_OFFLINE_CUSTOM );
}

void CAvatarDlg::OnBnClickedPlayOnline()
{
	UpdateData();

	CString sOnlineSound;
	m_wndOnlineSound.GetWindowText( sOnlineSound );

	PlaySound( sOnlineSound, NULL, SND_FILENAME | SND_ASYNC );
}

void CAvatarDlg::OnBnClickedPlayOffline()
{
	UpdateData();

	CString sOfflineSound;
	m_wndOfflineSound.GetWindowText( sOfflineSound );

	PlaySound( sOfflineSound, NULL, SND_FILENAME | SND_ASYNC );
}

void CAvatarDlg::OnBnClickedReset()
{
	{
		CSingleLock pLock( &theApp.m_pSection, TRUE );

		if ( theApp.IsValid( m_pAvatar ) )
		{
			ZeroMemory( m_pAvatar->m_nTimeline, sizeof( m_pAvatar->m_nTimeline ) );
		}
	}

	AfxGetMainWnd()->GetDlgItem( IDC_USERS )->InvalidateRect( NULL );
	AfxGetMainWnd()->GetDlgItem( IDC_USERS )->UpdateWindow();

	m_wndTimeline.InvalidateRect( NULL );
	m_wndTimeline.UpdateWindow();
}
