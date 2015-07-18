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

// CAvatarDlg dialog

class CAvatarDlg : public CDialog
{
public:
	CAvatarDlg(CAvatar* pAVatar, CWnd* pParent = NULL);

	enum { IDD = IDD_AVATAR };

protected:
	CAvatar*			m_pAvatar;
	CMFCEditBrowseCtrl	m_wndOnlineSound;
	CMFCEditBrowseCtrl	m_wndOfflineSound;
	CButton				m_wndPlayOnline;
	CButton				m_wndPlayOffline;
	BOOL				m_bLoop;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBnClickedOnline();
	afx_msg void OnBnClickedOffline();
	afx_msg void OnBnClickedPlayOnline();
	afx_msg void OnBnClickedPlayOffline();

	DECLARE_MESSAGE_MAP()
};
