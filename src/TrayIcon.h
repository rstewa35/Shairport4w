/*
 *
 *  TrayIcon.h
 *
 */

#pragma once


#define WM_TRAYICON		(WM_APP+0)

// Use a guid to uniquely identify our icon
class __declspec(uuid("0C1194D3-14C5-4E15-9F7B-CC046B69CD4C")) ShairPort4WTrayIcon;

template <class T>
class CTrayIconImpl
{
public:
	CTrayIconImpl()
	{
		m_bInit			= false;
		memset(&m_nid, 0, sizeof(m_nid));
	}
	~CTrayIconImpl()
	{
		RemoveTray();
	}

	bool InitTray(LPCTSTR lpszToolTip, HICON hIcon, UINT nID)
	{
		if (m_bInit)
			return true;

		T* pT = static_cast<T*>(this);
	
		// add the icon, setting the icon, tooltip, and callback message.
		// the icon will be identified with the GUID

		m_nid.cbSize			= sizeof(NOTIFYICONDATA);
		m_nid.hWnd				= pT->m_hWnd;
		m_nid.uID				= nID;
		m_nid.guidItem			= __uuidof(ShairPort4WTrayIcon);
		m_nid.hIcon				= hIcon;
		m_nid.uFlags			= NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_SHOWTIP | NIF_GUID;
		m_nid.uCallbackMessage	= WM_TRAYICON;
		_tcsncpy_s(m_nid.szTip, _countof(m_nid.szTip), lpszToolTip, _TRUNCATE);
		m_bInit					= Shell_NotifyIcon(NIM_ADD, (PNOTIFYICONDATA)&m_nid) ? true : false;

		if (m_bInit)
		{
			// NOTIFYICON_VERSION_4 is prefered
			m_nid.uVersion = NOTIFYICON_VERSION_4;
			m_bInit		   =  Shell_NotifyIcon(NIM_SETVERSION, &m_nid);	
		}

		return m_bInit;
	}
	void UpdateTrayIcon(HICON hIcon)
	{
		if (m_bInit)
		{
			T* pT = static_cast<T*>(this);

			if (pT->m_bTray)
			{
				if (hIcon == NULL)
					hIcon = pT->m_hIconSmall;

				m_nid.uFlags	= NIF_ICON;
				m_nid.hIcon		= hIcon;

				Shell_NotifyIcon(NIM_MODIFY, (PNOTIFYICONDATA)&m_nid);
			}
		}
	}
	void SetTooltipText(LPCTSTR pszTooltipText)
	{
		if (m_bInit && pszTooltipText)
		{
			m_nid.uFlags = NIF_TIP;
			_tcsncpy_s(m_nid.szTip, _countof(m_nid.szTip), pszTooltipText, _TRUNCATE);

			Shell_NotifyIcon(NIM_MODIFY, (PNOTIFYICONDATA)&m_nid);
		}
	}
	void SetInfoText(LPCTSTR pszInfoTitle, LPCTSTR pszInfoText, HICON hIcon = NULL)
	{
		if (m_bInit && pszInfoText)
		{
			m_nid.uFlags		= NIF_INFO;
			m_nid.dwInfoFlags	= NIIF_USER | (hIcon ? NIIF_LARGE_ICON : 0);
			m_nid.uTimeout		= 15000;

			if (pszInfoTitle)
				_tcsncpy_s(m_nid.szInfoTitle, _countof(m_nid.szInfoTitle), pszInfoTitle, _TRUNCATE);
			_tcsncpy_s(m_nid.szInfo, _countof(m_nid.szInfo), pszInfoText, _TRUNCATE);

			m_nid.hBalloonIcon = hIcon;

			Shell_NotifyIcon(NIM_MODIFY, (PNOTIFYICONDATA)&m_nid);
		}
	}
	void RemoveTray()
	{
		if (m_bInit)
		{
			NOTIFYICONDATA nid	= { 0 };
			nid.cbSize			= sizeof(nid);
			nid.uFlags			= NIF_GUID;
			nid.guidItem		= __uuidof(ShairPort4WTrayIcon);
			Shell_NotifyIcon(NIM_DELETE, (PNOTIFYICONDATA)&nid);
			m_bInit = false;
		}
	}

	BEGIN_MSG_MAP(CTrayIconImpl<T>)
		MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
	END_MSG_MAP()

	LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		if (wParam != m_nid.uID)
			return 0;

		T* pT = static_cast<T*>(this);

		if (LOWORD(lParam) == WM_RBUTTONUP)
		{
			CMenu Menu;

			if (!Menu.LoadMenu(m_nid.uID))
				return 0;

			CMenuHandle Popup(Menu.GetSubMenu(0));

			CPoint pos;
			GetCursorPos(&pos);

			SetForegroundWindow(pT->m_hWnd);

			Popup.SetMenuDefaultItem(0, TRUE);

			Popup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pT->m_hWnd);
			pT->PostMessage(WM_NULL);
			Menu.DestroyMenu();
		}
		else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
		{
			::ShowWindow(pT->m_hWnd, SW_SHOWNORMAL);
			::SetForegroundWindow(pT->m_hWnd);
		}
		return 0;
	}

protected:
	bool				m_bInit;
	NOTIFYICONDATA		m_nid;
};
