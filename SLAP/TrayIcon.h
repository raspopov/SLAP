/*
 * Copyright (c) Istvan Pasztor
 * This source has been published on www.codeproject.com under the CPOL license.
 */

#pragma once

struct ITrayIconListener;

// You can use this class either by inheriting from it and overriding the OnMessage() method,
// or by instantiating this class directly and setting its listener object or function.

class CTrayIcon
{
public:
	CTrayIcon(LPCTSTR name= _T(""), bool visible = false, HICON hIcon = NULL, bool destroy_icon_in_destructor = false);
	virtual ~CTrayIcon();

	virtual void SetName(const CString& name);
	const CString& GetName() const { return m_Name; }

	virtual bool SetVisible(bool visible);
	bool IsVisible() const { return m_Visible; }

	// The destructor may destroy the specified hIcon. If you want to avoid that, call
	// SetIcon(NULL, false) or SetDestroyIconInDestructor(false).
	virtual void SetIcon(HICON hNewIcon, bool destroy_current_icon=true);
	HICON GetIcon() const { return m_hIcon; }

	void SetDestroyIconInDestructor(bool b)	{ m_DestroyIconInDestructor = b; }
	bool GetDestroyIconInDestructor() const { return m_DestroyIconInDestructor; }

	// ShowBalloonTooltip() works only on win2k and later
	bool ShowBalloonTooltip(LPCTSTR title, LPCTSTR msg, DWORD icon);

	typedef void (*POnMessageFunc)(CTrayIcon* pTrayIcon, UINT uMsg);
	void SetListener(POnMessageFunc pOnMessageFunc) { m_pOnMessageFunc = pOnMessageFunc; }
	void SetListener(ITrayIconListener *pListener) { m_pListener = pListener; }

protected:
	// uMsg can be one of the following window messages:
	// - WM_MOUSEMOVE
	// - WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK
	// - WM_RBUTTONDOWN, WM_RBUTTONUP, WM_RBUTTONDBLCLK
	// - WM_MBUTTONDOWN, WM_MBUTTONUP, WM_MBUTTONDBLCLK
	// WinXP and later:
	// - NIN_BALLOONXXX messages (eg.: NIN_BALLOONUSERCLICK)
	//
	// Use GetCursorPos() if you need the location of the cursor.
	// The default implementation calls the listener.
	virtual void OnMessage(UINT uMsg);

private:
	void FillNotifyIconData(NOTIFYICONDATA& data);
	// Never returns NULL! If GetIcon()==NULL, then this returns a system icon
	HICON InternalGetIcon() const;
	bool AddIcon();
	bool RemoveIcon();
	void OnTaskbarCreated();

private:
	UINT				m_Id;
	CString				m_Name;
	bool				m_Visible;
	HICON				m_hIcon;
	bool				m_DestroyIconInDestructor;
	POnMessageFunc		m_pOnMessageFunc;
	ITrayIconListener*	m_pListener;

	static LRESULT CALLBACK MessageProcessorWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static HWND GetMessageProcessorHWND();
};


struct ITrayIconListener
{
	virtual void OnTrayIconMouseMove(CTrayIcon* /*pTrayIcon*/) {}

	virtual void OnTrayIconLButtonDown(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconLButtonUp(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconLButtonDblClk(CTrayIcon* /*pTrayIcon*/) {}

	virtual void OnTrayIconRButtonDown(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconRButtonUp(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconRButtonDblClk(CTrayIcon* /*pTrayIcon*/) {}

	virtual void OnTrayIconMButtonDown(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconMButtonUp(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconMButtonDblClk(CTrayIcon* /*pTrayIcon*/) {}

	// WinXP and later
	virtual void OnTrayIconSelect(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconBalloonShow(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconBalloonHide(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconBalloonTimeout(CTrayIcon* /*pTrayIcon*/) {}
	virtual void OnTrayIconBalloonUserClick(CTrayIcon* /*pTrayIcon*/) {}

	// Use GetCursorPos() if you need the location of the cursor.
	virtual void OnTrayIconMessage(CTrayIcon* pTrayIcon, UINT uMsg)
	{
		switch (uMsg)
		{
		case WM_MOUSEMOVE: OnTrayIconMouseMove(pTrayIcon); break;
		case WM_LBUTTONDOWN: OnTrayIconLButtonDown(pTrayIcon); break;
		case WM_LBUTTONUP: OnTrayIconLButtonUp(pTrayIcon); break;
		case WM_LBUTTONDBLCLK: OnTrayIconLButtonDblClk(pTrayIcon); break;
		case WM_RBUTTONDOWN: OnTrayIconRButtonDown(pTrayIcon); break;
		case WM_RBUTTONUP: OnTrayIconRButtonUp(pTrayIcon); break;
		case WM_RBUTTONDBLCLK: OnTrayIconRButtonDblClk(pTrayIcon); break;
		case WM_MBUTTONDOWN: OnTrayIconMButtonDown(pTrayIcon); break;
		case WM_MBUTTONUP: OnTrayIconMButtonUp(pTrayIcon); break;
		case WM_MBUTTONDBLCLK: OnTrayIconMButtonDblClk(pTrayIcon); break;

#ifdef NIN_SELECT
		case NIN_SELECT: OnTrayIconSelect(pTrayIcon); break;
		case NIN_BALLOONSHOW: OnTrayIconBalloonShow(pTrayIcon); break;
		case NIN_BALLOONHIDE: OnTrayIconBalloonHide(pTrayIcon); break;
		case NIN_BALLOONTIMEOUT: OnTrayIconBalloonTimeout(pTrayIcon); break;
		case NIN_BALLOONUSERCLICK: OnTrayIconBalloonUserClick(pTrayIcon); break;
#endif
		}
	}
};
