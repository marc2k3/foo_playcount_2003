#include "stdafx.hpp"
#include "DialogEdit.hpp"

CDialogEdit::CDialogEdit()
	: m_history_added(guids::history_added)
	, m_history_first_played(guids::history_first_played)
	, m_history_last_played(guids::history_last_played)
	, m_history_playcount(guids::history_playcount) {}

#pragma region static
bool CDialogEdit::CheckComponent(const pfc::string& name)
{
	for (auto ptr : componentversion::enumerate())
	{
		string8 str;
		ptr->get_file_name(str);
		if (stricmp_utf8(str, name) == 0)
		{
			return true;
		}
	}

	return false;
}

bool CDialogEdit::CheckString(const string8& str)
{
	return str.contains("%") || str.contains("$");
}
#pragma endregion

BOOL CDialogEdit::OnInitDialog(CWindow, LPARAM)
{
	m_button_clear = GetDlgItem(IDC_BTN_CLEAR);
	m_button_presets = GetDlgItem(IDC_BTN_PRESETS);

	m_window_added = GetDlgItem(IDC_COMBO_ADDED);
	m_window_first_played = GetDlgItem(IDC_COMBO_FIRST_PLAYED);
	m_window_last_played = GetDlgItem(IDC_COMBO_LAST_PLAYED);
	m_window_playcount = GetDlgItem(IDC_COMBO_PLAYCOUNT);

	m_history_added.on_init(m_window_added, "");
	m_history_first_played.on_init(m_window_first_played, "");
	m_history_last_played.on_init(m_window_last_played, "");
	m_history_playcount.on_init(m_window_playcount, "");

	// not via on_init, we don't want these values saved
	pfc::setWindowText(m_window_added, m_added);
	pfc::setWindowText(m_window_first_played, m_first_played);
	pfc::setWindowText(m_window_last_played, m_last_played);
	pfc::setWindowText(m_window_playcount, m_playcount);

	if (!Component::simple_mode)
	{
		m_window_first_played.EnableWindow(FALSE);
		m_window_last_played.EnableWindow(FALSE);
		m_window_playcount.EnableWindow(FALSE);

		m_button_presets.EnableWindow(FALSE);
		m_button_clear.EnableWindow(FALSE);
	}

	m_hooks.AddDialogWithControls(*this);
	CenterWindow();
	return TRUE;
}

void CDialogEdit::OnClear(uint32_t, int, CWindow)
{
	m_window_added.SetWindowTextW(L"");
	m_window_first_played.SetWindowTextW(L"");
	m_window_last_played.SetWindowTextW(L"");
	m_window_playcount.SetWindowTextW(L"");
}

void CDialogEdit::OnCloseCmd(uint32_t, int nID, CWindow)
{
	if (nID == IDOK)
	{
		m_added = pfc::getWindowText(m_window_added);
		m_first_played = pfc::getWindowText(m_window_first_played);
		m_last_played = pfc::getWindowText(m_window_last_played);
		m_playcount = pfc::getWindowText(m_window_playcount);

		if (CheckString(m_added)) m_history_added.add_item(m_added);
		if (CheckString(m_first_played)) m_history_first_played.add_item(m_first_played);
		if (CheckString(m_last_played)) m_history_last_played.add_item(m_last_played);
		if (CheckString(m_playcount)) m_history_playcount.add_item(m_playcount);
	}

	EndDialog(nID);
}

void CDialogEdit::OnPresets(uint32_t, int, CWindow)
{
	const bool foo_playcount = CheckComponent("foo_playcount");
	const bool foo_lastfm_playcount_sync = CheckComponent("foo_lastfm_playcount_sync");

	HMENU menu = CreatePopupMenu();
	AppendMenuW(menu, foo_playcount ? MF_STRING : MF_GRAYED, ID_MENU_FOO_PLAYCOUNT, L"foo_playcount");
	AppendMenuW(menu, foo_lastfm_playcount_sync ? MF_STRING : MF_GRAYED, ID_MENU_FOO_LASTFM_PLAYCOUNT_SYNC, L"foo_lastfm_playcount_sync");

	CRect rect;
	GetDlgItem(IDC_BTN_PRESETS).GetWindowRect(&rect);
	const int id = TrackPopupMenuEx(menu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rect.left, rect.bottom, m_hWnd, nullptr);
	DestroyMenu(menu);

	switch (id)
	{
	case ID_MENU_FOO_PLAYCOUNT:
		m_window_added.SetWindowTextW(L"[%added%]");
		m_window_first_played.SetWindowTextW(L"[%first_played%]");
		m_window_last_played.SetWindowTextW(L"[%last_played%]");
		m_window_playcount.SetWindowTextW(L"[%play_count%]");
		break;
	case ID_MENU_FOO_LASTFM_PLAYCOUNT_SYNC:
		m_window_added.SetWindowTextW(L"");
		m_window_first_played.SetWindowTextW(L"[%lfm_first_played%]");
		m_window_last_played.SetWindowTextW(L"[%lfm_last_played%]");
		m_window_playcount.SetWindowTextW(L"[%lfm_playcount%]");
		break;
	}
}
