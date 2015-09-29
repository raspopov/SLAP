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
#include "OptionsDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


// COptionsDlg dialog

COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CDialog				( COptionsDlg::IDD, pParent )
	, m_bDebugLog			( FALSE )
	, m_bShowOnlineOnly		( FALSE )
	, m_bShowOnlineOnlyOld	( FALSE )
	, m_bOnlineTray			( FALSE )
	, m_bOfflineTray		( FALSE )
	, m_bAutostart			( FALSE )
	, m_nUpdatePeriod		( UPDATE_PERIOD_DEFAULT )
	, m_nDeadTime			( DEAD_TIME_DEFAULT )
	, m_bChanged			( FALSE )
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange( pDX );

	DDX_Text( pDX, IDC_USERNAME, m_sUsername );
	DDX_Text( pDX, IDC_PASSWORD, m_sPassword );
	DDX_Check( pDX, IDC_DEBUGLOG, m_bDebugLog );
	DDX_Control( pDX, IDC_SOUND_ONLINE, m_wndOnlineSound );
	DDX_Control( pDX, IDC_SOUND_OFFLINE, m_wndOfflineSound );
	DDX_Control( pDX, IDC_PLAY_ONLINE, m_wndPlayOnline );
	DDX_Control( pDX, IDC_PLAY_OFFLINE, m_wndPlayOffline );
	DDX_Check( pDX, IDC_ONLINE_ONLY, m_bShowOnlineOnly );
	DDX_Check( pDX, IDC_ONLINE_TRAY, m_bOnlineTray );
	DDX_Check( pDX, IDC_OFFLINE_TRAY, m_bOfflineTray );
	DDX_Control( pDX, IDC_TITLE, m_wndTitle );
	DDX_Control( pDX, IDC_UPDATE, m_wndUpdate );
	DDX_Check( pDX, IDC_AUTOSTART, m_bAutostart );
	DDX_Text( pDX, IDC_UPDATE_PERIOD, m_nUpdatePeriod );
	DDV_MinMaxInt( pDX, m_nUpdatePeriod, UPDATE_PERIOD_MIN, UPDATE_PERIOD_MAX );
	DDX_Text( pDX, IDC_DEAD_TIME, m_nDeadTime );
	DDV_MinMaxInt( pDX, m_nDeadTime, DEAD_TIME_MIN, DEAD_TIME_MAX );
	DDX_Control( pDX, IDC_UPDATE_PERIOD_SPIN, m_wndUpdatePeriodSpin );
	DDX_Control( pDX, IDC_DEAD_TIME_SPIN, m_wndDeadTimeSpin );
}

BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
	ON_BN_CLICKED( IDC_ONLINE_NO_SOUND, &COptionsDlg::OnBnClickedOnline )
	ON_BN_CLICKED( IDC_ONLINE_CUSTOM, &COptionsDlg::OnBnClickedOnline )
	ON_BN_CLICKED( IDC_OFFLINE_NO_SOUND, &COptionsDlg::OnBnClickedOffline )
	ON_BN_CLICKED( IDC_OFFLINE_CUSTOM, &COptionsDlg::OnBnClickedOffline )
	ON_BN_CLICKED( IDC_PLAY_ONLINE, &COptionsDlg::OnBnClickedPlayOnline )
	ON_BN_CLICKED( IDC_PLAY_OFFLINE, &COptionsDlg::OnBnClickedPlayOffline )
	ON_NOTIFY( NM_CLICK, IDC_TITLE, &COptionsDlg::OnNMClickTitle )
	ON_NOTIFY( NM_RETURN, IDC_TITLE, &COptionsDlg::OnNMClickTitle )
	ON_NOTIFY( NM_CLICK, IDC_UPDATE, &COptionsDlg::OnNMClickUpdate )
	ON_NOTIFY( NM_RETURN, IDC_UPDATE, &COptionsDlg::OnNMClickUpdate )
END_MESSAGE_MAP()

BOOL COptionsDlg::OnInitDialog()
{
	__super::OnInitDialog();

	m_bShowOnlineOnlyOld = m_bShowOnlineOnly;

	m_wndUpdatePeriodSpin.SetRange32( UPDATE_PERIOD_MIN, UPDATE_PERIOD_MAX );
	m_wndDeadTimeSpin.SetRange32( DEAD_TIME_MIN, DEAD_TIME_MAX );

	const int nClose = theApp.sCopyright.Find( _T( '>' ) );
	const int nOpen = theApp.sCopyright.Find( _T( '<' ) );
	const CString sMail = theApp.sCopyright.Mid( nOpen + 1, nClose - nOpen - 1 );
	m_wndTitle.SetWindowText( CString( _T("<a href=\"" ) ) + szAppURL + _T( "\">" ) +
		theApp.sFullTitle + _T("</a> ") + theApp.sVersion + _T( "\n" ) +
		theApp.sCopyright.Left( nOpen + 1 ) + _T("<a href=\"mailto:" ) + sMail + _T( "?Subject=" ) +
		theApp.sFullTitle + _T(" ") + theApp.sVersion + _T( "\">" ) + sMail + _T( "</a>" ) + theApp.sCopyright.Mid( nClose ) );

	theApp.LoadPassword( m_sUsername, m_sPassword );

	CString sFilter;
	VERIFY( sFilter.LoadString( IDS_SOUNDS_FILTER ) );

	{
		CString sOnlineSound = theApp.GetProfileString( SETTINGS, SOUND_ONLINE );
		CheckRadioButton( IDC_ONLINE_NO_SOUND, IDC_ONLINE_CUSTOM, ( sOnlineSound.IsEmpty() ? IDC_ONLINE_NO_SOUND : IDC_ONLINE_CUSTOM ) );

		m_wndOnlineSound.EnableFileBrowseButton( _T( "wav" ), sFilter );
		m_wndOnlineSound.EnableWindow( ! sOnlineSound.IsEmpty() );
		m_wndOnlineSound.SetWindowText( sOnlineSound );

		m_wndPlayOnline.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_PLAY ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
		m_wndPlayOnline.EnableWindow( ! sOnlineSound.IsEmpty() );
	}

	{
		CString sOfflineSound = theApp.GetProfileString( SETTINGS, SOUND_OFFLINE );
		CheckRadioButton( IDC_OFFLINE_NO_SOUND, IDC_OFFLINE_CUSTOM, ( sOfflineSound.IsEmpty() ? IDC_OFFLINE_NO_SOUND : IDC_OFFLINE_CUSTOM ) );

		m_wndOfflineSound.EnableFileBrowseButton( _T( "wav" ), sFilter );
		m_wndOfflineSound.EnableWindow( ! sOfflineSound.IsEmpty() );
		m_wndOfflineSound.SetWindowText( sOfflineSound );

		m_wndPlayOffline.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_PLAY ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
		m_wndPlayOffline.EnableWindow( ! sOfflineSound.IsEmpty() );
	}

	const CString sStartupLink = theApp.sStartup + _T("\\") + theApp.m_pszAppName + _T(".lnk");
	m_bAutostart = ( GetFileAttributes( sStartupLink ) != INVALID_FILE_ATTRIBUTES );

	UpdateData( FALSE );

	return TRUE;
}

void COptionsDlg::OnOK()
{
	if ( ! UpdateData() )
		return;

	m_bChanged = FALSE;

	// Save changed username and/or password only
	CString sUsername, sPassword;
	theApp.LoadPassword( sUsername, sPassword );
	if ( m_sUsername != sUsername || m_sPassword != sPassword )
	{
		m_bChanged = TRUE;
		theApp.SavePassword( m_sUsername, m_sPassword );
	}

	// Clean-up
	SecureZeroMemory( sUsername.GetBuffer(), sUsername.GetLength() * sizeof( TCHAR ) );
	sUsername.ReleaseBuffer();
	SecureZeroMemory( sPassword.GetBuffer(), sPassword.GetLength() * sizeof( TCHAR ) );
	sPassword.ReleaseBuffer();
	SecureZeroMemory( m_sUsername.GetBuffer(), m_sUsername.GetLength() * sizeof( TCHAR ) );
	m_sUsername.ReleaseBuffer();
	SecureZeroMemory( m_sPassword.GetBuffer(), m_sPassword.GetLength() * sizeof( TCHAR ) );
	m_sPassword.ReleaseBuffer();

	{
		int nOnline = GetCheckedRadioButton( IDC_ONLINE_NO_SOUND, IDC_ONLINE_CUSTOM );
		CString sOnlineSound;
		switch ( nOnline )
		{
		case IDC_ONLINE_NO_SOUND:
			break;
		case IDC_ONLINE_CUSTOM:
			m_wndOnlineSound.GetWindowText( sOnlineSound );
			break;
		}
		theApp.WriteProfileString( SETTINGS, SOUND_ONLINE, sOnlineSound );
	}

	{
		int nOffline = GetCheckedRadioButton( IDC_OFFLINE_NO_SOUND, IDC_OFFLINE_CUSTOM );
		CString sOfflineSound;
		switch ( nOffline )
		{
		case IDC_OFFLINE_NO_SOUND:
			break;
		case IDC_OFFLINE_CUSTOM:
			m_wndOfflineSound.GetWindowText( sOfflineSound );
			break;
		}
		theApp.WriteProfileString( SETTINGS, SOUND_OFFLINE, sOfflineSound );
	}

	if ( m_bShowOnlineOnlyOld != m_bShowOnlineOnly )
	{
		m_bChanged = TRUE;
	}

	const CString sStartupLink = theApp.sStartup + _T( "\\" ) + theApp.m_pszAppName + _T( ".lnk" );
	if ( m_bAutostart )
	{
		CComPtr< IShellLink > pLink;
		if ( SUCCEEDED( pLink.CoCreateInstance( CLSID_ShellLink ) ) )
		{
			pLink->SetPath( theApp.sModuleFileName );
			pLink->SetDescription( theApp.sFullTitle );
			pLink->SetArguments( _T("-tray") );
			CComQIPtr< IPersistFile > pLinkFile( pLink );
			if ( pLinkFile )
				pLinkFile->Save( sStartupLink, TRUE );
		}
	}
	else
		DeleteFile( sStartupLink );

	__super::OnOK();
}

void COptionsDlg::OnCancel()
{
	SecureZeroMemory( m_sUsername.GetBuffer(), m_sUsername.GetLength() * sizeof( TCHAR ) );
	m_sUsername.ReleaseBuffer();
	SecureZeroMemory( m_sPassword.GetBuffer(), m_sPassword.GetLength() * sizeof( TCHAR ) );
	m_sPassword.ReleaseBuffer();

	__super::OnCancel();
}

void COptionsDlg::OnBnClickedOnline()
{
	UpdateData();

	int nOnline = GetCheckedRadioButton( IDC_ONLINE_NO_SOUND, IDC_ONLINE_CUSTOM );
	m_wndOnlineSound.EnableWindow( nOnline == IDC_ONLINE_CUSTOM );
	m_wndPlayOnline.EnableWindow( nOnline == IDC_ONLINE_CUSTOM );
}

void COptionsDlg::OnBnClickedOffline()
{
	UpdateData();

	int nOffline = GetCheckedRadioButton( IDC_OFFLINE_NO_SOUND, IDC_OFFLINE_CUSTOM );
	m_wndOfflineSound.EnableWindow( nOffline == IDC_OFFLINE_CUSTOM );
	m_wndPlayOffline.EnableWindow( nOffline == IDC_OFFLINE_CUSTOM );
}

void COptionsDlg::OnBnClickedPlayOnline()
{
	UpdateData();

	CString sOnlineSound;
	m_wndOnlineSound.GetWindowText( sOnlineSound );

	PlaySound( sOnlineSound, NULL, SND_FILENAME | SND_ASYNC );
}

void COptionsDlg::OnBnClickedPlayOffline()
{
	UpdateData();

	CString sOfflineSound;
	m_wndOfflineSound.GetWindowText( sOfflineSound );

	PlaySound( sOfflineSound, NULL, SND_FILENAME | SND_ASYNC );
}

void COptionsDlg::OnNMClickTitle(NMHDR *pNMHDR, LRESULT *pResult)
{
	PNMLINK pNMLink = (PNMLINK)pNMHDR;

	ShellExecute( NULL,NULL, pNMLink->item.szUrl, NULL, NULL, SW_SHOWDEFAULT );

	*pResult = 0;
}

void COptionsDlg::OnNMClickUpdate(NMHDR * /*pNMHDR*/, LRESULT *pResult)
{
	//PNMLINK pNMLink = (PNMLINK)pNMHDR;

	win_sparkle_check_update_with_ui();

	__super::OnCancel();

	*pResult = 0;
}
