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
#include "CtrlsResize.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Construction/Destruction
CCtrlResize::CControlInfo::CControlInfo () :
	controlID(0), bindtype (BIND_UNKNOWN), rectInitial(), m_pControlWnd(NULL)
{
}

CCtrlResize::CControlInfo::CControlInfo (int _controlID, int _bindtype, const CRect& _rectInitial, CWnd* _pWnd)
{
	controlID = _controlID;
	bindtype = _bindtype;
	rectInitial = _rectInitial;
	m_pControlWnd = _pWnd;
}

CCtrlResize::CCtrlResize() :
	m_pWnd (NULL)
{
}

CCtrlResize::~CCtrlResize()
{
	for (int i = 0; i < m_aCtrls.GetSize(); i++)
		delete m_aCtrls.GetAt(i);
}

int CCtrlResize::AddControl (int _controlID, int _bindtype, const CRect &_rectInitial)
{
	m_aCtrls.Add (new CControlInfo (_controlID, _bindtype, _rectInitial, 0));
	return 0;
}
int CCtrlResize::AddControl (CWnd* _pWnd, int _bindtype, const CRect &_rectInitial)
{
	m_aCtrls.Add (new CControlInfo (0, _bindtype, _rectInitial, _pWnd));
	return 0;
}

int CCtrlResize::FixControls()
{
	if (!m_pWnd)
		return 1;

	m_pWnd->GetClientRect(&m_rectInitialParent);
	m_pWnd->ScreenToClient(&m_rectInitialParent);
	
	for (int i = 0; i < m_aCtrls.GetSize(); i++) {
		CControlInfo* pInfo = m_aCtrls.GetAt(i);
		CWnd* pControlWnd = pInfo->m_pControlWnd ? pInfo->m_pControlWnd :
			m_pWnd->GetDlgItem (pInfo->controlID);
		if (pControlWnd) {
			pControlWnd->GetWindowRect (&pInfo->rectInitial);
			m_pWnd->ScreenToClient (&pInfo->rectInitial);
		}
	}
	return 0;
}

void CCtrlResize::SetParentWnd(CWnd *pWnd)
{
	m_pWnd = pWnd;
}

void CCtrlResize::OnSize()
{
	if ( ! m_pWnd || ! ::IsWindow( m_pWnd->GetSafeHwnd() ) )
		return;

	CRect rr, rectWnd;
	m_pWnd->GetClientRect(&rectWnd);
	
	for (int i = 0; i < m_aCtrls.GetSize(); i++) {
		CControlInfo* pInfo = m_aCtrls.GetAt(i);
		if (pInfo) {
			CWnd* pControlWnd = pInfo->m_pControlWnd ? pInfo->m_pControlWnd :
				m_pWnd->GetDlgItem (pInfo->controlID);
			if (pControlWnd) {

				rr = pInfo->rectInitial;
				if (pInfo->bindtype & BIND_RIGHT) 
					rr.right = rectWnd.right -
						(m_rectInitialParent.Width() - pInfo->rectInitial.right);
				if (pInfo->bindtype & BIND_BOTTOM) 
					rr.bottom = rectWnd.bottom -
						(m_rectInitialParent.Height() - pInfo->rectInitial.bottom);
				if (pInfo->bindtype & BIND_TOP);
				else
					rr.top = rr.bottom - pInfo->rectInitial.Height();
				if (pInfo->bindtype & BIND_LEFT);
				else
					rr.left = rr.right - pInfo->rectInitial.Width();
				
				pControlWnd->MoveWindow(&rr);
				pControlWnd->Invalidate(FALSE);
			}
		}
	}
}
