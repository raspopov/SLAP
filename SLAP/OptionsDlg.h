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

// COptionsDlg dialog

class COptionsDlg : public CDialog
{
public:
	COptionsDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_OPTIONS };
	
	BOOL				m_bChanged;
	int					m_nUpdatePeriod;
	int					m_nDeadTime;
	BOOL				m_bShowOnlineOnly;
	BOOL				m_bOnlineTray;
	BOOL				m_bOfflineTray;
	BOOL				m_bDebugLog;

protected:
	CString				m_sUsername;
	CString				m_sPassword;
	CMFCEditBrowseCtrl	m_wndOnlineSound;
	CMFCEditBrowseCtrl	m_wndOfflineSound;
	CButton				m_wndPlayOnline;
	CButton				m_wndPlayOffline;
	CLinkCtrl			m_wndTitle;
	CLinkCtrl			m_wndUpdate;
	BOOL				m_bAutostart;
	CSpinButtonCtrl		m_wndUpdatePeriodSpin;
	CSpinButtonCtrl		m_wndDeadTimeSpin;
	BOOL				m_bShowOnlineOnlyOld;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnBnClickedOnline();
	afx_msg void OnBnClickedOffline();
	afx_msg void OnBnClickedPlayOnline();
	afx_msg void OnBnClickedPlayOffline();
	afx_msg void OnNMClickTitle(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickUpdate(NMHDR *pNMHDR, LRESULT *pResult);

	DECLARE_MESSAGE_MAP()
};
