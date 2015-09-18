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
#include "AvatarDlg.h"
#include "OptionsDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// CSLAPDlg dialog

CSLAPDlg::CSLAPDlg(CWnd* pParent /*=NULL*/)
	: CDialog			( CSLAPDlg::IDD, pParent )
	, m_hIcon			( theApp.LoadIcon( IDR_MAINFRAME ) )
	, m_pThread			( NULL )
	, m_eventThreadStop	( FALSE, TRUE )
	, m_dlgNotify		( NULL )
	, m_nUpdatePeriod	( max( UPDATE_PERIOD_MIN, min( UPDATE_PERIOD_MAX, theApp.GetProfileInt( SETTINGS, UPDATE_PERIOD, UPDATE_PERIOD_DEFAULT ) ) ) )
	, m_nDeadTime		( max( DEAD_TIME_MIN, min( DEAD_TIME_MAX, theApp.GetProfileInt( SETTINGS, DEAD_TIME, DEAD_TIME_DEFAULT ) ) ) )
	, m_bShowOnlineOnly	( theApp.GetProfileInt( SETTINGS, SHOW_ONLINE_ONLY, 0 ) != 0 )
	, m_bOnlineTray		( theApp.GetProfileInt( SETTINGS, ONLINE_TRAY, 1 ) != 0 )
	, m_bOfflineTray	( theApp.GetProfileInt( SETTINGS, OFFLINE_TRAY, 1 ) != 0 )
	, m_nYOffset		( 15 )
	, m_nXOffset		( 15 )
	, m_nMouseSelected	( LB_ERR )
{
}

void CSLAPDlg::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_USERS, m_wndAvatars);
	DDX_Control(pDX, IDC_STATUS, m_wndStatus);
	DDX_Control(pDX, IDC_TELEPORT, m_wndTeleport);
	DDX_Control(pDX, IDC_WEB, m_wndWeb);
	DDX_Control(pDX, IDC_COPY, m_wndCopy);
	DDX_Control(pDX, IDC_AVATAR_OPTIONS, m_wndAvatarOptions);
	DDX_Control(pDX, IDC_REFRESH, m_wndRefresh);
	DDX_Control(pDX, IDC_OPTIONS, m_wndOptions);
	DDX_Control(pDX, IDC_FILTER, m_wndFilter);
}

bool CSLAPDlg::Filter(const CAvatar* pAvatar) const
{
	return ( ! m_bShowOnlineOnly || ( pAvatar->m_bFriend && pAvatar->m_bOnline ) ) && pAvatar->m_bFiltered;
}

void CSLAPDlg::ReCreateList()
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	m_wndAvatars.ResetContent();

	for ( POSITION pos = theApp.GetAvatarIterator(); pos; )
	{
		const CAvatar* pAvatar = theApp.GetNextAvatar( pos );

		// Add new item
		if ( Filter( pAvatar ) )
		{
			int nIndex = m_wndAvatars.AddString( pAvatar->GetSortName() );
			m_wndAvatars.SetItemDataPtr( nIndex, (void*)pAvatar );
		}
	}

	UpdateInterface();
}

int CSLAPDlg::GetSelectedCount() const
{
	int nSelected = 0;
	const int nCount = GetCount();
	for ( int i = 0; i < nCount; ++i )
	{
		if ( IsSelected( i ) )
		{
			++ nSelected;
		}
	}
	return nSelected;
}

int CSLAPDlg::GetSelected() const
{
	const int nCount = m_wndAvatars.GetSelCount();
	if ( nCount < 1 )
		return LB_ERR;
	else if ( nCount == 1 || ! IsSelected( m_nMouseSelected ) )
		return m_wndAvatars.GetCurSel();
	else
		return m_nMouseSelected;
}

BOOL CSLAPDlg::PreTranslateMessage(MSG* pMsg)
{
	m_pTips.RelayEvent( pMsg );

	if ( pMsg->hwnd != m_wndAvatars.m_hWnd && pMsg->message == WM_MOUSEWHEEL )
	{
		m_wndAvatars.PostMessage( pMsg->message, pMsg->wParam, pMsg->lParam );
	}

	return __super::PreTranslateMessage( pMsg );
}

BEGIN_MESSAGE_MAP(CSLAPDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED( IDC_OPTIONS, &CSLAPDlg::OnBnClickedOptions )
	ON_BN_CLICKED( IDC_REFRESH, &CSLAPDlg::OnBnClickedRefresh )
	ON_MESSAGE( WM_APP_REFRESH, &CSLAPDlg::OnRefresh )
	ON_MESSAGE( WM_APP_NOTIFY, &CSLAPDlg::OnNotify )
	ON_WM_TIMER()
	ON_LBN_DBLCLK( IDC_USERS, &CSLAPDlg::OnLbnDblclkUsers )
	ON_LBN_SELCHANGE( IDC_USERS, &CSLAPDlg::OnLbnSelchangeUsers )
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_BN_CLICKED( IDC_TELEPORT, &CSLAPDlg::OnBnClickedTeleport )
	ON_BN_CLICKED( IDC_WEB, &CSLAPDlg::OnBnClickedWebProfile )
	ON_BN_CLICKED( IDC_COPY, &CSLAPDlg::OnBnClickedCopy )
	ON_BN_CLICKED( IDC_AVATAR_OPTIONS, &CSLAPDlg::OnBnClickedAvatarOptions )
	ON_WM_DESTROY()
	ON_WM_RBUTTONUP()
	ON_COMMAND( IDC_EXIT, &CSLAPDlg::OnExit )
	ON_COMMAND( IDC_SHOW, &CSLAPDlg::OnShow )
	ON_WM_WINDOWPOSCHANGING()
	ON_EN_CHANGE( IDC_FILTER, &CSLAPDlg::OnFilterChange)
	ON_COMMAND( IDC_DELETE, &CSLAPDlg::OnDelete )
END_MESSAGE_MAP()

// CSLAPDlg message handlers

BOOL CSLAPDlg::OnInitDialog()
{
	__super::OnInitDialog();

	SetWindowText( theApp.m_pszAppName );

	SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDR_MAINFRAME ), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_SHARED ), TRUE );		// Set big icon
	SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDR_MAINFRAME ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ), FALSE );	// Set small icon

	CString sFilterCue;
	VERIFY( sFilterCue.LoadString( IDC_FILTER ) );
	m_wndFilter.SetCueBanner( sFilterCue );
	m_wndFilter.SetLimitText( 64 );
	m_wndFilter.SetIcon( IDI_FILTER );

	m_pTray.SetIcon( m_hIcon, false );
	m_pTray.SetName( theApp.m_pszAppName );
	m_pTray.SetVisible( true );
	m_pTray.SetListener( this );

	m_pResizer.SetParentWnd( this );
	m_pResizer.AddControl( GetDlgItem( IDC_USERS   ), BIND_ALL );
	m_pResizer.AddControl( GetDlgItem( IDC_STATUS  ), BIND_BOTTOM | BIND_LEFT | BIND_RIGHT );
	m_pResizer.AddControl( &m_wndFilter, BIND_TOP | BIND_LEFT | BIND_RIGHT );
	m_pResizer.AddControl( &m_wndRefresh, BIND_TOP | BIND_RIGHT );
	m_pResizer.AddControl( &m_wndOptions, BIND_TOP | BIND_RIGHT );
	m_pResizer.FixControls();

	m_pTips.Create( this, TTS_ALWAYSTIP );
	m_pTips.Activate( TRUE );
	m_pTips.SetMaxTipWidth( 300 );
	m_pTips.AddTool( &m_wndTeleport, m_wndTeleport.GetDlgCtrlID() );
	m_pTips.AddTool( &m_wndWeb, m_wndWeb.GetDlgCtrlID() );
	m_pTips.AddTool( &m_wndCopy, m_wndCopy.GetDlgCtrlID() );
	m_pTips.AddTool( &m_wndAvatarOptions, m_wndAvatarOptions.GetDlgCtrlID() );
	m_pTips.AddTool( &m_wndRefresh, m_wndRefresh.GetDlgCtrlID() );
	m_pTips.AddTool( &m_wndOptions, m_wndOptions.GetDlgCtrlID() );
	m_pTips.AddTool( &m_wndFilter, m_wndFilter.GetDlgCtrlID() );

	m_wndTeleport.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_TELEPORT ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
	m_wndWeb.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_WEB ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
	m_wndCopy.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_COPY ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
	m_wndAvatarOptions.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_AVATAR_OPTIONS ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
	m_wndRefresh.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_REFRESH ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );
	m_wndOptions.SetIcon( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( IDI_OPTIONS ), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_SHARED ) );

	// Re-position window but make sure it fits to the virtual screen
	CRect rc;
	rc.left = theApp.GetProfileInt( SETTINGS, WINDOW_X, 0 );
	rc.top = theApp.GetProfileInt( SETTINGS, WINDOW_Y, 0 );
	rc.right = rc.left + theApp.GetProfileInt( SETTINGS, WINDOW_WIDTH, 0 ) - 1;
	rc.bottom = rc.top + theApp.GetProfileInt( SETTINGS, WINDOW_HEIGHT, 0 ) - 1;
	CRect rcScreen( 0, 0, GetSystemMetrics( SM_CXVIRTUALSCREEN ), GetSystemMetrics( SM_CYVIRTUALSCREEN ) );
	if ( ( rcScreen.PtInRect( CPoint( rc.left,  rc.top ) ) ||
		   rcScreen.PtInRect( CPoint( rc.right, rc.top ) ) ||
		   rcScreen.PtInRect( CPoint( rc.left,  rc.bottom ) ) ||
		   rcScreen.PtInRect( CPoint( rc.right, rc.bottom ) ) ) &&
		   rc.Width()  > MIN_WIDTH &&
		   rc.Height() > MIN_HEIGHT )
	{
		MoveWindow( rc, FALSE );
	}

	theApp.LoadAvatars();

	ReCreateList();

	CString sUsername, sPassword;
	if ( ! theApp.LoadPassword( sUsername, sPassword ) || sUsername.IsEmpty() || sPassword.IsEmpty() )
	{
		// No username or no password - ask user first
		SetStatus( IDS_NOPASSWORD );

		ShowWindow( SW_SHOWNORMAL );
		CenterWindow();

		OnBnClickedOptions();
	}
	else
	{
		if ( theApp.m_bTray )
			PostMessage( WM_SYSCOMMAND, SC_MINIMIZE );

		// Auto-login
		AddWork( Work::Update );
	}

	SetTimer( TIMER, 250, NULL );

	Start();

	// Setup WinSparkle update system
	win_sparkle_set_appcast_url( APPCAST_URL );
	win_sparkle_set_langid( GetUserDefaultLangID() );
	win_sparkle_set_shutdown_request_callback( &CSLAPDlg::OnShutdown );
	win_sparkle_init();
	win_sparkle_check_update_without_ui();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void __cdecl CSLAPDlg::OnShutdown()
{
	::PostMessage( theApp.m_pMainWnd->GetSafeHwnd(), WM_CLOSE, 0, 0 );
}

void CSLAPDlg::OnDestroy()
{
	CWaitCursor wc;

	if ( ! IsWorkEnabled() )
		// Already waiting for stop
		return;

	CString sClose;
	VERIFY( sClose.LoadString( IDS_CLOSING ) );
	m_pTray.SetName( theApp.sFullTitle + _T( "\r\n" ) + sClose );

	win_sparkle_cleanup();

	PlaySound( NULL, NULL, SND_ASYNC );

	ShowNotifyDialog( NULL, NULL );

	KillTimer( TIMER );

	Stop();

	CRect rc;
	GetWindowRect( &rc );
	theApp.WriteProfileInt( SETTINGS, WINDOW_X, rc.left );
	theApp.WriteProfileInt( SETTINGS, WINDOW_Y, rc.top );
	theApp.WriteProfileInt( SETTINGS, WINDOW_WIDTH, rc.Width() );
	theApp.WriteProfileInt( SETTINGS, WINDOW_HEIGHT, rc.Height() );
	
	m_pTray.SetVisible( false );

	CAvatar::Clear();

	__super::OnDestroy();
}

void CSLAPDlg::OnPaint()
{
	if ( IsIconic() )
	{
		CPaintDC dc( this );
		SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>( dc.GetSafeHdc() ), 0 );
		CRect rect;
		GetClientRect( &rect );
		dc.DrawIcon( ( rect.Width() - GetSystemMetrics( SM_CXICON ) + 1 ) / 2, ( rect.Height() - GetSystemMetrics( SM_CYICON ) + 1 ) / 2, m_hIcon );
	}
	else
	{
		__super::OnPaint();
	}
}

HCURSOR CSLAPDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>( m_hIcon );
}

void CSLAPDlg::OnOK()
{
	// ENTER
}

void CSLAPDlg::OnBnClickedOptions()
{
	COptionsDlg dlg( this );
	dlg.m_nUpdatePeriod = m_nUpdatePeriod;
	dlg.m_nDeadTime = m_nDeadTime;
	dlg.m_bShowOnlineOnly = m_bShowOnlineOnly;
	dlg.m_bOnlineTray = m_bOnlineTray;
	dlg.m_bOfflineTray = m_bOfflineTray;
	dlg.m_bDebugLog = theApp.m_bDebugLog;

	if ( dlg.DoModal() == IDOK )
	{
		m_nUpdatePeriod = dlg.m_nUpdatePeriod;
		m_nDeadTime = dlg.m_nDeadTime;
		m_bShowOnlineOnly = dlg.m_bShowOnlineOnly;
		m_bOnlineTray = dlg.m_bOnlineTray;
		m_bOfflineTray = dlg.m_bOfflineTray;
		theApp.m_bDebugLog = dlg.m_bDebugLog;

		theApp.WriteProfileInt( SETTINGS, UPDATE_PERIOD, m_nUpdatePeriod );
		theApp.WriteProfileInt( SETTINGS, DEAD_TIME, m_nDeadTime );
		theApp.WriteProfileInt( SETTINGS, SHOW_ONLINE_ONLY, m_bShowOnlineOnly ? 1 : 0 );
		theApp.WriteProfileInt( SETTINGS, ONLINE_TRAY, m_bOnlineTray ? 1 : 0 );
		theApp.WriteProfileInt( SETTINGS, OFFLINE_TRAY, m_bOfflineTray ? 1 : 0 );
		theApp.WriteProfileInt( SETTINGS, DEBUGLOG, theApp.m_bDebugLog ? 1 : 0 );

		theApp.Log();

		if ( dlg.m_bChanged )
		{
			ReCreateList();

			// Forced login
			AddWork( Work::Login );
		}
	}
}

int CSLAPDlg::FindAvatar(const CAvatar* pAvatar)
{
	const int nCount = GetCount();
	for ( int i = 0; i < nCount; ++i )
	{
		if ( Get( i ) == pAvatar )
			return i;
	}
	return LB_ERR;
}

LRESULT CSLAPDlg::OnRefresh(WPARAM wParam, LPARAM lParam)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	const CTime tNow = CTime::GetCurrentTime();

	const BOOL bSuccess = (BOOL)wParam;
	CAvatar* pAvatar = (CAvatar*)lParam;

	if ( pAvatar )
	{
		if ( theApp.IsValid( pAvatar ) )
		{
			pAvatar->Save();

			int nIndex = FindAvatar( pAvatar );
			if ( nIndex != LB_ERR )
			{
				CRect rc;
				if ( m_wndAvatars.GetItemRect( nIndex, &rc ) != LB_ERR && ! rc.IsRectEmpty() )
				{
					m_wndAvatars.InvalidateRect( &rc, FALSE );
					UpdateWindow();
				}
			}
		}
		return 0;
	}

	for ( POSITION pos = theApp.GetAvatarIterator(); pos; )
	{
		pAvatar = theApp.GetNextAvatar( pos );

		const BOOL bEnabled = ( tNow - pAvatar->m_tStatusChange ).GetTotalSeconds() > m_nDeadTime;

		if ( pAvatar->m_bDirty ||
			 pAvatar->m_bFriend != pAvatar->m_bNewFriend ||
			 ( pAvatar->m_bOnline != pAvatar->m_bNewOnline && bEnabled ) )
		{
			pAvatar->m_bFriend = pAvatar->m_bNewFriend;

			if ( bSuccess && pAvatar->m_bOnline != pAvatar->m_bNewOnline && bEnabled )
			{
				pAvatar->m_bOnline = pAvatar->m_bNewOnline;
				pAvatar->m_tStatusChange = tNow;

				ShowNotify( pAvatar );
			}

			// Remove old item
			int nIndex = FindAvatar( pAvatar );
			if ( nIndex != LB_ERR )
			{
				m_wndAvatars.DeleteString( nIndex );
				nIndex = LB_ERR;
			}

			// Add new item
			if ( Filter( pAvatar ) )
			{
				nIndex = m_wndAvatars.AddString( pAvatar->GetSortName() );
				m_wndAvatars.SetItemDataPtr( nIndex, (void*)pAvatar );
			}

			// Update item only
			CRect rc;
			if ( nIndex != LB_ERR && m_wndAvatars.GetItemRect( nIndex, &rc ) != LB_ERR && ! rc.IsRectEmpty() )
			{
				m_wndAvatars.InvalidateRect( &rc, FALSE );
			}
		}

		// Prepare for next run
		pAvatar->m_bDirty = FALSE;
		pAvatar->m_bNewFriend = FALSE;
		pAvatar->m_bNewOnline = FALSE;
	}

	UpdateInterface();

	// Report good result
	if ( bSuccess )
	{
		CString sStatus;
		sStatus.Format( IDS_RESULT, theApp.GetAvatarCount(), theApp.GetAvatarCount( TRUE ) );
		SetStatus( sStatus );
	}

	m_wndAvatars.InvalidateRect( NULL );
	m_wndAvatars.UpdateWindow();

	return 0;
}

void CSLAPDlg::OnBnClickedRefresh()
{
	AddWork( Work::Update );
}

void CSLAPDlg::OnTimer( UINT_PTR nIDEvent )
{
	__super::OnTimer( nIDEvent );

	if ( nIDEvent == TIMER )
	{
		CSingleLock pLock( &theApp.m_pSection, TRUE );

		const CTime tNow = CTime::GetCurrentTime();

		// Update status
		CString sOldStatus;
		m_wndStatus.GetWindowText( sOldStatus );
		if ( sOldStatus != m_sStatus )
		{
			m_wndStatus.SetWindowText( m_sStatus );
			theApp.Log( m_sStatus );

			m_pTray.SetName( theApp.sFullTitle + _T( "\r\n" ) + m_sStatus );
		}

		// Time to refresh avatar list?
		if ( (tNow - m_tLastRefresh).GetTotalSeconds() > m_nUpdatePeriod )
		{
			m_tLastRefresh = tNow;
			AddWork( Work::Update );
		}

		// Time to process next avatar notification?
		if ( m_pNotifications.GetCount() )
		{
			if ( (tNow - m_tLastNotification).GetTotalSeconds() > 2 )
			{
				m_tLastNotification = tNow;

				theApp.Notify( m_pNotifications.RemoveHead() );
			}
		}
	}
}

void CSLAPDlg::ShowNotify(const CAvatar* pAvatar)
{
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	if ( m_pNotifications.Find( pAvatar ) == NULL )
	{
		m_pNotifications.AddTail( pAvatar );
	}
}

void CSLAPDlg::ShowNotifyDialog(LPCTSTR szTitle, LPCTSTR szText)
{
	if ( m_dlgNotify )
	{
		m_dlgNotify->DestroyWindow();
		delete m_dlgNotify;
		m_dlgNotify = NULL;
	}

	if ( szTitle )
	{
		m_dlgNotify = new CNotifyDlg( szTitle, szText );
	}
}

LRESULT CSLAPDlg::OnNotify(WPARAM, LPARAM lParam)
{
	// Show notification popup and play sound
	
	CSingleLock pLock( &theApp.m_pSection, TRUE );

	const CAvatar* pAvatar = (const CAvatar*)lParam;

	if ( ! pAvatar )
	{
		// Stop notifications
		PlaySound( NULL, NULL, SND_ASYNC );

		ShowNotifyDialog( NULL, NULL );
	}
	else if ( theApp.IsValid( pAvatar ) )
	{
		CString sTitle;
		VERIFY( sTitle.LoadString( pAvatar->m_bOnline ? IDS_AVATAR_ONLINE : IDS_AVATAR_OFFLINE ) );

		CString sNotify;
		sNotify.Format( IDS_AVATAR_NOTIFY, (LPCTSTR)pAvatar->m_sDisplayName, (LPCTSTR)pAvatar->m_sRealName );

		if ( ! m_dlgNotify )
		{
			CString sSound = pAvatar->GetSound();
			if ( ! sSound.IsEmpty() )
				PlaySound( sSound, NULL, SND_FILENAME | SND_ASYNC | ( pAvatar->m_bLoopSound ? SND_LOOP : 0 ) );

			if ( pAvatar->m_bOnline && pAvatar->m_bLoopSound )
				ShowNotifyDialog( sTitle, sNotify );
		}

		if ( pAvatar->m_bOnline ? m_bOnlineTray : m_bOfflineTray )
			m_pTray.ShowBalloonTooltip( sTitle, sNotify, NIIF_USER );

		theApp.Log( _T( "Show notification \"" ) + sTitle + _T( "\" : " ) + sNotify );
	}

	return 0;
}

void CSLAPDlg::OnTrayIconLButtonDblClk(CTrayIcon* /*pTrayIcon*/)
{
	OnShow();
}

void CSLAPDlg::OnExit()
{
	OnCancel();
}

void CSLAPDlg::OnShow()
{
	if ( ! IsWindowVisible() )
	{
		ShowWindow( SW_SHOWNORMAL );
		SetForegroundWindow();
	}
	else
		ShowWindow( SW_HIDE );
}

void CSLAPDlg::OnTrayIconRButtonUp(CTrayIcon* /*pTrayIcon*/)
{
	CMenu pMenu;
	if ( pMenu.LoadMenu( IDR_MENU ) )
	{
		// Get second sub-menu - Avatar Menu
		if ( CMenu* pAvatarMenu = pMenu.GetSubMenu( 1 ) )
		{
			CPoint ptCursor;
			GetCursorPos( &ptCursor );

			pAvatarMenu->TrackPopupMenu( TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptCursor.x, ptCursor.y, this );
		}
		pMenu.DestroyMenu();
	}
}

void CSLAPDlg::OnLbnDblclkUsers()
{
	if ( GetKeyState( VK_SHIFT ) & 0x8000000 )
	{
		const int nSelected = m_wndAvatars.GetCurSel();
		if ( nSelected != LB_ERR )
		{
			ShowNotify( Get( nSelected ) );
		}
	}
	else if ( GetKeyState( VK_CONTROL ) & 0x8000000 )
	{
		OnBnClickedTeleport();
	}
	else if ( GetKeyState( VK_MENU ) & 0x8000000 )
	{
		OnBnClickedWebProfile();
	}
	else
	{
		OnBnClickedAvatarOptions();
	}
}

void CSLAPDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ( nID == SC_MINIMIZE )
	{
		m_pTray.SetVisible( true );
		ShowWindow( SW_HIDE );
		return;
	}

	__super::OnSysCommand(nID, lParam);
}

void CSLAPDlg::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize( nType, cx, cy );

	m_pResizer.OnSize();
}

void CSLAPDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	__super::OnGetMinMaxInfo( lpMMI );

	lpMMI->ptMinTrackSize.x = max( lpMMI->ptMinTrackSize.x, MIN_WIDTH );
	lpMMI->ptMinTrackSize.y = max( lpMMI->ptMinTrackSize.y, MIN_HEIGHT );
}

void CSLAPDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT pMIS)
{
	if ( nIDCtl == IDC_USERS )
	{
		pMIS->itemHeight = CAvatar::OnMeasureItem( this );
	}
	else
		__super::OnMeasureItem( nIDCtl, pMIS );
}

void CSLAPDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT pDIS)
{
	if ( nIDCtl == IDC_USERS && pDIS->itemData )
	{
		CSingleLock pLock( &theApp.m_pSection, TRUE );

		const CAvatar* pAvatar = (const CAvatar*)pDIS->itemData;
		if ( theApp.IsValid( pAvatar ) )
		{
			pAvatar->OnDrawItem( pDIS );
		}
	}
	else
		__super::OnDrawItem( nIDCtl, pDIS );
}

BOOL CSLAPDlg::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
{
	if ( nChar == VK_DELETE )
	{
		if ( GetFocus() == static_cast< CWnd*>( &m_wndAvatars ) )
		{
			OnDelete();
			return TRUE;
		}
	}
	else if ( nChar == VK_RETURN )
	{
		OnBnClickedAvatarOptions();
		return TRUE;
	}
	else if ( nChar == VK_F1 )
	{
		ShellExecute( NULL, NULL, _T( APP_URL ), NULL, NULL, SW_SHOWDEFAULT );
		return TRUE;
	}
	else if ( nChar == VK_F5 )
	{
		OnBnClickedRefresh();
		return TRUE;
	}
	else if ( nChar == 'P' && GetKeyState( VK_CONTROL ) & 0x8000000 )
	{
		OnBnClickedOptions();
		return TRUE;
	}
	else if ( ( nChar == 'C' || nChar == VK_INSERT ) && GetKeyState( VK_CONTROL ) & 0x8000000 )
	{
		OnBnClickedCopy();
		return TRUE;
	}
	else if ( nChar == 'A' && GetKeyState( VK_CONTROL ) & 0x8000000 )
	{
		const int nCount = GetCount();
		for ( int i = 0; i < nCount; ++ i )
		{
			m_wndAvatars.SetSel( i, TRUE );
		}
		return TRUE;
	}
	else if ( nChar == VK_ESCAPE )
	{
		const int nCount = GetCount();
		for ( int i = 0; i < nCount; ++ i )
		{
			m_wndAvatars.SetSel( i, FALSE );
		}
		return TRUE;
	}
	return FALSE;
}

void CSLAPDlg::OnUsersRButtonUp(UINT /*nFlags*/, CPoint point)
{
	BOOL bOutside = TRUE;
	m_nMouseSelected = m_wndAvatars.ItemFromPoint( point, bOutside );
	if ( m_nMouseSelected != LB_ERR && ! bOutside )
	{
		CMenu pMenu;
		if ( pMenu.LoadMenu( IDR_MENU ) )
		{
			CMenu* pAvatarMenu = NULL;

			{
				CSingleLock pLock( &theApp.m_pSection, TRUE );

				const CAvatar* pAvatar = Get( m_nMouseSelected );
				if ( theApp.IsValid( pAvatar ) )
				{
					// Get first sub-menu - Avatar Menu
					pAvatarMenu = pMenu.GetSubMenu( 0 );

					pAvatarMenu->EnableMenuItem( IDC_TELEPORT, MF_BYCOMMAND | ( pAvatar->m_sPlace.IsEmpty() ? MF_DISABLED : MF_ENABLED ) );
				}
			}

			CPoint ptCursor;
			GetCursorPos( &ptCursor );
			if ( pAvatarMenu )
				pAvatarMenu->TrackPopupMenu( TPM_TOPALIGN | TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptCursor.x, ptCursor.y, this );

			pMenu.DestroyMenu();
		}
	}
}

void CSLAPDlg::OnLbnSelchangeUsers()
{
	UpdateInterface();
}

void CSLAPDlg::UpdateInterface()
{
	BOOL bTeleport = FALSE, bSelected = FALSE;

	const int nCount = GetCount();
	for ( int i = 0; i < nCount; ++i )
	{
		if ( IsSelected( i ) )
		{
			CSingleLock pLock( &theApp.m_pSection, TRUE );

			const CAvatar* pAvatar = Get( i );
			if ( theApp.IsValid( pAvatar ) && ! pAvatar->m_sPlace.IsEmpty() )
				bTeleport = TRUE;

			bSelected = TRUE;
		}
	}

	m_wndTeleport.EnableWindow( bTeleport );
	m_wndWeb.EnableWindow( bSelected );
	m_wndCopy.EnableWindow( bSelected );
	m_wndAvatarOptions.EnableWindow( bSelected );
}

void CSLAPDlg::OnBnClickedTeleport()
{
	CWaitCursor wc;

	const int nSelected = GetSelected();
	if ( nSelected != LB_ERR )
	{
		CSingleLock pLock( &theApp.m_pSection, TRUE );

		const CAvatar* pAvatar = Get( nSelected );
		if ( theApp.IsValid( pAvatar ) && ! pAvatar->m_sPlace.IsEmpty() )
		{
			const CString sPlace = pAvatar->m_sPlace + _T( "/0" );

			theApp.Log( _T( "Teleporting to: " ) + sPlace );
			ShellExecute( NULL, NULL, sPlace, NULL, NULL, SW_SHOWDEFAULT );
		}
	}
}

void CSLAPDlg::OnBnClickedWebProfile()
{
	CWaitCursor wc;

	const int nCount = GetCount();
	for ( int i = 0; i < nCount; ++i )
	{
		if ( IsSelected( i ) )
		{
			CSingleLock pLock( &theApp.m_pSection, TRUE );

			const CAvatar* pAvatar = Get( i );
			if ( theApp.IsValid( pAvatar ) )
			{
				CString sRealName = pAvatar->m_sRealName;
				sRealName.MakeLower();
				sRealName.Replace( _T( ' ' ), _T( '.' ) );

				CString sProfile;
				VERIFY( sProfile.LoadString( IDS_PROFILE_URL ) );
				sProfile.Replace( _T( "{USERNAME}" ), sRealName );

				theApp.Log( _T( "Opening web-profile: " ) + sProfile );
				ShellExecute( NULL, NULL, sProfile, NULL, NULL, SW_SHOWDEFAULT );
			}
		}
	}
}

void CSLAPDlg::OnBnClickedCopy()
{
	CWaitCursor wc;
	CString sText;

	const int nCount = GetCount();
	for ( int i = 0; i < nCount; ++i )
	{
		if ( IsSelected( i ) )
		{
			CSingleLock pLock( &theApp.m_pSection, TRUE );

			const CAvatar* pAvatar = Get( i );
			if ( theApp.IsValid( pAvatar ) )
			{
				sText += pAvatar->m_sDisplayName + _T( " (" ) + CAvatar::MakePretty( pAvatar->m_sRealName ) + _T( ")" );
				if ( ! pAvatar->m_sPlace.IsEmpty() )
					sText += _T( " - " ) + pAvatar->m_sPlace + _T( "/0" );
				sText += _T( "\r\n" );
			}
		}
	}

	if ( ! sText.IsEmpty() && OpenClipboard() )
	{
		EmptyClipboard();

		if ( HGLOBAL hCopy = GlobalAlloc( GMEM_MOVEABLE, ( sText.GetLength() + 1 ) * sizeof( TCHAR ) ) )
		{
			if ( LPVOID lpCopy = GlobalLock( hCopy ) )
			{
				memcpy( lpCopy, (LPCTSTR)sText, ( sText.GetLength() + 1 ) * sizeof( TCHAR ) );

				GlobalUnlock( hCopy );

				SetClipboardData( CF_UNICODETEXT, hCopy );
			}
		}

		CloseClipboard();
	}
}

void CSLAPDlg::OnBnClickedAvatarOptions()
{
	CWaitCursor wc;

	const int nSelected = GetSelected();
	if ( nSelected != LB_ERR )
	{
		CAvatarDlg dlg( Get( nSelected ), this );
		dlg.DoModal();
	}
}

void CSLAPDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if ( ! lpwndpos->cx || ! lpwndpos->cy || ! ( GetKeyState( VK_SHIFT ) & 0x8000000 ) )
		return;

	CRect wndRect;
	GetWindowRect( &wndRect );

	// Screen resolution
	const int screenWidth = GetSystemMetrics( SM_CXSCREEN );
	const int screenHeight = GetSystemMetrics( SM_CYSCREEN );

	// Find the taskbar
	CWnd* pWnd = FindWindow( _T("Shell_TrayWnd"), _T("") );
	if ( ! pWnd )
		return;

	CRect trayRect;
	pWnd->GetWindowRect( &trayRect );

	const int wndWidth = wndRect.Width();
	const int wndHeight = wndRect.Height();

	int leftTaskbar = 0, rightTaskbar = 0, topTaskbar = 0, bottomTaskbar = 0;
	if ( trayRect.top <= 0 && trayRect.left <= 0 && trayRect.right >= screenWidth )
	{
		// top taskbar
		topTaskbar = trayRect.bottom - trayRect.top;
	}
	else if ( trayRect.top > 0 && trayRect.left <= 0 )
	{
		// bottom taskbar
		bottomTaskbar = trayRect.bottom - trayRect.top;
	}
	else if ( trayRect.top <= 0 && trayRect.left > 0 )
	{
		// right taskbar
		rightTaskbar = trayRect.right - trayRect.left;
	}
	else
	{
		// left taskbar
		leftTaskbar = trayRect.right - trayRect.left;
	}

	// Snap to screen border
	// Left border
	if ( lpwndpos->x >= -m_nXOffset + leftTaskbar && lpwndpos->x <= leftTaskbar + m_nXOffset )
	{
		lpwndpos->x = leftTaskbar;
	}

	// Top border
	if ( lpwndpos->y >= -m_nYOffset && lpwndpos->y <= topTaskbar + m_nYOffset )
	{
		lpwndpos->y = topTaskbar;
	}

	// Right border
	if ( lpwndpos->x + wndWidth <= screenWidth - rightTaskbar + m_nXOffset && lpwndpos->x + wndWidth >= screenWidth - rightTaskbar - m_nXOffset )
	{
		lpwndpos->x = screenWidth - rightTaskbar - wndWidth;
	}

	// Bottom border
	if ( lpwndpos->y + wndHeight <= screenHeight - bottomTaskbar + m_nYOffset && lpwndpos->y + wndHeight >= screenHeight - bottomTaskbar - m_nYOffset )
	{
		lpwndpos->y = screenHeight - bottomTaskbar - wndHeight;
	}
}

void CSLAPDlg::OnFilterChange()
{
	m_wndFilter.GetWindowText( m_sFilter );

	CSingleLock pLock( &theApp.m_pSection, TRUE );

	for ( POSITION pos = theApp.GetAvatarIterator(); pos; )
	{
		CAvatar* pAvatar = theApp.GetNextAvatar(pos);

		pAvatar->Filter( m_sFilter );
	}

	ReCreateList();
}

void CSLAPDlg::OnDelete()
{
	const int nSelected = GetSelectedCount();
	if ( nSelected > 0 )
	{
		CString sPrompt;
		sPrompt.Format( IDS_DELETE_AVATAR, nSelected );
		if ( AfxMessageBox( sPrompt, MB_ICONQUESTION | MB_YESNO ) == IDYES )
		{
			int nCount = GetCount();
			for ( int i = 0; i < nCount; ++i )
			{
				if ( IsSelected( i ) )
				{
					theApp.DeleteAvatar( Get( i ) );

					nCount = m_wndAvatars.DeleteString( i );
					i = -1;
				}
			}

			UpdateInterface();
		}
	}
}
