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

#pragma once

#include "TrayIcon.h"
#include "CtrlsResize.h"
#include "NotifyDlg.h"
#include "UsersBox.h"

#define MIN_WIDTH		250
#define MIN_HEIGHT		250
#define BOX_GAP			4
#define BOX_INDICATOR	8
#define BOX_STATUS		16
#define BOX_ICON		(48 + 2)
#define RGB_ONLINE		RGB( 64, 240, 64 )		// Minimum color value must be 64
#define RGB_OFFLINE		RGB( 200, 200, 200 )	// Minimum color value must be 64
#define RGB_UNKNOWN		RGB( 240, 64, 64 )		// Minimum color value must be 64
#define TIMER			1


// CSLAPDlg dialog

class CSLAPDlg : public CDialog, ITrayIconListener
{
public:
	CSLAPDlg(CWnd* pParent = NULL);	// standard constructor

	enum { IDD = IDD_SLAP_DIALOG };
	
	BOOL OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	void OnUsersRButtonUp(UINT nFlags, CPoint point);

protected:
	HICON					m_hIcon;
	HICON					m_hUserIcon;
	HICON					m_hSoundIcon;
	HICON					m_hNoSoundIcon;
	HICON					m_hAlertIcon;
	CFont					m_fntBold;
	CFont					m_fntNormal;
	int						m_nTitleHeight;
	int						m_nTextHeight;
	CButton					m_wndTeleport;
	CButton					m_wndWeb;
	CButton					m_wndCopy;
	CButton					m_wndAvatarOptions;
	CButton					m_wndRefresh;
	CButton					m_wndOptions;
	CUsersBox				m_wndAvatars;
	CStatic					m_wndStatus;
	CString					m_sStatus;
	CCtrlResize				m_pResizer;
	CToolTipCtrl			m_pTips;
	CTrayIcon				m_pTray;
	CNotifyDlg*				m_dlgNotify;

	CList< CAvatar* >		m_pNotifications;		// Avatar notification queue
	CTime					m_tLastNotification;	// Time of last notification

	volatile CWinThread*	m_pThread;
	CEvent					m_eventThreadStop;

	CTime					m_tLastRefresh;
	DWORD					m_nUpdatePeriod;		// Update avatar list period (seconds), 30..3600 seconds, default: 60 seconds
	DWORD					m_nDeadTime;			// Dead time after status change (seconds), default: 300 seconds
	BOOL					m_bShowOnlineOnly;
	BOOL					m_bOnlineTray;
	BOOL					m_bOfflineTray;

	int						m_nYOffset;
	int						m_nXOffset;

	enum Work { Idle, Login, Update };
	CList< Work >			m_pWorkQueue;
	void AddWork(Work work);
	Work GetNextWork();

	DWORD WebRequest(CInternetSession* pInternet, const CString& sUrl, const CString& sReferer, CByteArray& aContent, CString& sLocation, LPCSTR szParams = NULL, BOOL bPOST = FALSE, BOOL bNoCache = TRUE);
	BOOL WebLogin(CInternetSession* pInternet);
	BOOL WebUpdate(CInternetSession* pInternet);
	BOOL WebGetImage(CInternetSession* pInternet);
	
	// Status
	void SetStatus(UINT nStatus);
	void SetStatus(LPCTSTR szStatus);
	CString GetStatus() const;
	
	void ReCreateList();										// Re-create avatar list
	int FindAvatar(CAvatar* pAvatar);							// Returns avatar index in the avatar list (LB_ERR - not found)
	void ShowNotify(CAvatar* pAvatar);
	void ShowNotifyDialog(LPCTSTR szTitle, LPCTSTR szText);

	static UINT __cdecl ThreadFn(LPVOID pParam);
	void Thread();
	void Start();
	void Stop();
	BOOL IsWorkEnabled() const;

	static void __cdecl OnShutdown();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnOK();

	virtual void OnTrayIconLButtonDblClk(CTrayIcon* pTrayIcon);
	virtual void OnTrayIconRButtonUp(CTrayIcon* pTrayIcon);

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedOptions();
	afx_msg void OnBnClickedRefresh();
	afx_msg LRESULT OnRefresh(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLbnDblclkUsers();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnLbnSelchangeUsers();
	afx_msg void OnBnClickedTeleport();
	afx_msg void OnBnClickedWebProfile();
	afx_msg void OnBnClickedCopy();
	afx_msg void OnBnClickedAvatarOptions();
	afx_msg void OnDestroy();
	afx_msg void OnExit();
	afx_msg void OnShow();
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);

	DECLARE_MESSAGE_MAP()
};
