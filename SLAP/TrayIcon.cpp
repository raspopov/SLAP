// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * Copyright (c) Istvan Pasztor
 * This source has been published on www.codeproject.com under the CPOL license.
 */

#include "stdafx.h"
#include "TrayIcon.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


#define TRAY_WINDOW_MESSAGE (WM_APP + 100)

typedef CMap< UINT, UINT, CTrayIcon*, CTrayIcon* const & > CIdToTrayIconMap;

static CIdToTrayIconMap& GetIdToTrayIconMap()
{
	static CIdToTrayIconMap pMap;
	return (CIdToTrayIconMap&)pMap;
}

static UINT GetNextTrayIconId()
{
	static UINT next_id = 1;
	return next_id++;
}

static const UINT g_WndMsgTaskbarCreated = RegisterWindowMessage( TEXT("TaskbarCreated") );

LRESULT CALLBACK CTrayIcon::MessageProcessorWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == TRAY_WINDOW_MESSAGE)
	{
		CTrayIcon* pIcon;
		if ( GetIdToTrayIconMap().Lookup( (UINT)wParam, pIcon ) )
			pIcon->OnMessage((UINT)lParam);
		return 0;
	}
	else if (uMsg == g_WndMsgTaskbarCreated)
	{
		CIdToTrayIconMap &id_to_tray = GetIdToTrayIconMap();
		for ( POSITION pos = id_to_tray.GetStartPosition(); pos; )
		{
			UINT id;
			CTrayIcon* pIcon;
			id_to_tray.GetNextAssoc( pos, id, pIcon );
			pIcon->OnTaskbarCreated();
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND CTrayIcon::GetMessageProcessorHWND()
{
	static HWND hWnd = NULL;
	if ( ! hWnd )
	{
		static LPCTSTR TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASSNAME = _T("TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASS");
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
		WNDCLASSEX wc = {};
		wc.cbSize = sizeof(wc);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hInstance = hInstance;
		wc.lpfnWndProc = MessageProcessorWndProc;
		wc.lpszClassName = TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASSNAME;
		if (!RegisterClassEx(&wc))
			return NULL;

		hWnd = CreateWindowEx(
			0,
			TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASSNAME,
			_T("TRAY_ICON_MESSAGE_PROCESSOR_WND"),
			WS_POPUP,
			0, 0, 0, 0,
			NULL,
			NULL,
			hInstance,
			NULL
			);
	}
	return hWnd;
}

CTrayIcon::CTrayIcon(LPCTSTR name, bool visible, HICON hIcon, bool destroy_icon_in_destructor)
	: m_Id( GetNextTrayIconId() )
	, m_Name( name )
	, m_hIcon( hIcon )
	, m_Visible( false )
	, m_DestroyIconInDestructor( destroy_icon_in_destructor )
	, m_pOnMessageFunc( NULL )
	, m_pListener( NULL )
{
	GetIdToTrayIconMap().SetAt( m_Id, this );
	SetVisible(visible);
}

CTrayIcon::~CTrayIcon()
{
	SetVisible(false);
	SetIcon(NULL, m_DestroyIconInDestructor);
	GetIdToTrayIconMap().RemoveKey( m_Id );
}

HICON CTrayIcon::InternalGetIcon() const
{
	return m_hIcon ? m_hIcon : ::LoadIcon( NULL, IDI_APPLICATION );
}

bool CTrayIcon::AddIcon()
{
	NOTIFYICONDATA data;
	FillNotifyIconData(data);
	data.uFlags |= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	data.uCallbackMessage = TRAY_WINDOW_MESSAGE;
	data.hIcon = InternalGetIcon();

	size_t tip_len = max( _countof( data.szTip ) - 1, m_Name.GetLength() );
	_tcsncpy_s( data.szTip, m_Name, tip_len );
	data.szTip[ tip_len ] = 0;

	return FALSE != Shell_NotifyIcon(NIM_ADD, &data);
}

bool CTrayIcon::RemoveIcon()
{
	NOTIFYICONDATA data;
	FillNotifyIconData(data);
	return FALSE != Shell_NotifyIcon(NIM_DELETE, &data);
}

void CTrayIcon::OnTaskbarCreated()
{
	if (m_Visible)
		AddIcon();
}

void CTrayIcon::SetName(const CString& name)
{
	m_Name = name;
	if (m_Visible)
	{
		NOTIFYICONDATA data;
		FillNotifyIconData(data);
		data.uFlags |= NIF_TIP;

		size_t tip_len = max( _countof( data.szTip ) - 1, name.GetLength() );
		_tcsncpy_s( data.szTip, name, tip_len );
		data.szTip[ tip_len ] = 0;

		Shell_NotifyIcon(NIM_MODIFY, &data);
	}
}

bool CTrayIcon::SetVisible(bool visible)
{
	m_Visible = visible;

	RemoveIcon();

	if ( m_Visible )
		return AddIcon();
	else
		return true;
}

void CTrayIcon::SetIcon(HICON hNewIcon, bool destroy_current_icon)
{
	if (m_hIcon == hNewIcon)
		return;
	if (destroy_current_icon && m_hIcon)
		DestroyIcon(m_hIcon);
	m_hIcon = hNewIcon;

	if (m_Visible)
	{
		NOTIFYICONDATA data;
		FillNotifyIconData(data);
		data.uFlags |= NIF_ICON;
		data.hIcon = InternalGetIcon();
		Shell_NotifyIcon(NIM_MODIFY, &data);
	}
}

bool CTrayIcon::ShowBalloonTooltip(LPCTSTR title, LPCTSTR msg, DWORD icon)
{
	if ( ! m_Visible )
		return false;

	NOTIFYICONDATA data;
	FillNotifyIconData(data);
	data.uFlags |= NIF_INFO;
	data.dwInfoFlags = icon;
	data.uTimeout = 10000;	// deprecated as of Windows Vista, it has a min(10000) and max(30000) value on previous Windows versions.
	data.uVersion = NOTIFYICON_VERSION;

	size_t title_len = max( _countof( data.szInfoTitle ) - 1, _tcslen( title ) );
	_tcsncpy_s( data.szInfoTitle, title, title_len );
	data.szInfoTitle[ title_len ] = 0;

	size_t msg_len = max( _countof( data.szInfo ) - 1, _tcslen( msg ) );
	_tcsncpy_s( data.szInfo, msg, msg_len );
	data.szInfo[ msg_len ] = 0;

	return FALSE != Shell_NotifyIcon(NIM_MODIFY, &data);
}

void CTrayIcon::OnMessage(UINT uMsg)
{
	if (m_pOnMessageFunc)
		m_pOnMessageFunc(this, uMsg);
	if (m_pListener)
		m_pListener->OnTrayIconMessage(this, uMsg);
}

void CTrayIcon::FillNotifyIconData(NOTIFYICONDATA& data)
{
	memset( &data, 0, sizeof( data ) );
	data.cbSize = NOTIFYICONDATA_V2_SIZE;	// Windows XP
	data.hWnd = GetMessageProcessorHWND();
	data.uID = m_Id;
}
