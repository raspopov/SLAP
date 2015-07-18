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

const int BIND_TOP =	0x01;
const int BIND_LEFT =	0x02;
const int BIND_RIGHT =	0x04;
const int BIND_BOTTOM =	0x08;
const int BIND_ALL =	0x0F;
const int BIND_UNKNOWN = 0x00;

#pragma warning (push)
#pragma warning (disable : 4239)

class CCtrlResize
{
public:
	CCtrlResize();
	~CCtrlResize();
	void OnSize ();
	void SetParentWnd (CWnd *pWnd);
	int FixControls ();
	int AddControl (int _controlID, int _bindtype, const CRect& _rectInitial = CRect());
	int AddControl (CWnd *_pControl, int _bindtype, const CRect& _rectInitial = CRect());

	class CControlInfo  
	{
	public:
		CControlInfo ();
		CControlInfo (int _controlID, int _bindtype, const CRect& _rectInitial, CWnd* _m_pControlWnd = NULL);
		int controlID;
		int bindtype;
		CRect rectInitial;
		CWnd* m_pControlWnd;
	};

private:
	CWnd* m_pWnd;
	CArray <CControlInfo*, CControlInfo*> m_aCtrls;
	CRect m_rectInitialParent;
};

#pragma warning (pop)
