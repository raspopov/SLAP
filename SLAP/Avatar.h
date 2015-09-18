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


class CAvatar
{
public:
	CAvatar(const CString& sRealName);
	~CAvatar();

	CString GetSortName() const;
	CString GetSound() const;
	void Filter(const CString& sFilter);

	void OnDrawItem(LPDRAWITEMSTRUCT pDIS) const;
	void PaintTimeline(CDC* pDC, const CRect* pRect) const;
	//int GetHour(const CRect* pRect, CPoint pt) const;

	void Delete();					// Delete avatar data from the registry and disk
	BOOL Load();					// Load avatar data from the registry (including image from disk cache)
	BOOL Save() const;				// Save avatar data to the registry
	BOOL SetImage(HBITMAP hBitmap);	// Set new avatar image

	static BOOL IsValidUsername(LPCTSTR szUsername);
	static CString MakePretty(const CString& sName);
	static int OnMeasureItem(CWnd* pWnd);
	static void Clear();			// Clears all static resources

	CString		m_sRealName;		// Avatar real name (including "Resident" for new avatars)
	CString		m_sDisplayName;		// Avatar display name
	BOOL		m_bOnline;			// Is avatar online?
	CTime		m_tStatusChange;	// Time of last online/offline status change
	BOOL		m_bFriend;			// Is avatar a friend?
	CTime		m_tLastOnline;		// When avatar was online?
	CString		m_sPlace;			// Last known avatar place
	CString		m_sOnlineSound;		// Play this sound file when avatar goes online (empty - default sound)
	CString		m_sOfflineSound;	// Play this sound file when avatar goes offline
	BOOL		m_bLoopSound;		// Play sound until manually stopped
	CTime		m_tImage;			// When image was loaded (successful or not)
	DWORD		m_nTimeline[ 24 ];	// Avatar online timeline

	BOOL		m_bNewOnline;		// New online status
	BOOL		m_bNewFriend;		// New friend status
	BOOL		m_bDirty;			// Data was changed and list must be resorted
	BOOL		m_bFiltered;		// Avatar was filtered by name filter
	CImage		m_pImage;			// Avatar image (second life)

	// Static resources

	static HICON	m_hUserIcon;
	static HICON	m_hSoundIcon;
	static HICON	m_hNoSoundIcon;
	static HICON	m_hAlertIcon;
	static CFont	m_fntBold;
	static CFont	m_fntNormal;
	static CFont	m_fntTimeline;
	static int		m_nItemHeight;
	static int		m_nTitleHeight;
	static int		m_nTextHeight;
};
