#include "stdafx.hpp"
#include "DialogEdit.hpp"

CDialogEdit::CDialogEdit()
	: m_history_added(guids::history_added)
	, m_history_first_played(guids::history_first_played)
	, m_history_last_played(guids::history_last_played)
	, m_history_playcount(guids::history_playcount)
	, m_history_rating(guids::history_rating) {}

#pragma region static
bool CDialogEdit::CheckComponent(const pfc::string& name)
{
	for (auto ptr : componentversion::enumerate())
	{
		pfc::string8 str;
		ptr->get_file_name(str);
		if (stricmp_utf8(str, name) == 0)
		{
			return true;
		}
	}

	return false;
}

bool CDialogEdit::GetMenuFlag(const pfc::string8& name)
{
	return CheckComponent(name) ? MF_STRING : MF_GRAYED;
}

bool CDialogEdit::IsTF(std::string_view str)
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
	m_window_rating = GetDlgItem(IDC_COMBO_RATING);

	m_history_added.on_init(m_window_added, "");
	m_history_first_played.on_init(m_window_first_played, "");
	m_history_last_played.on_init(m_window_last_played, "");
	m_history_playcount.on_init(m_window_playcount, "");
	m_history_rating.on_init(m_window_rating, "");

	// not via on_init, we don't want these values saved
	pfc::setWindowText(m_window_added, m_added);
	pfc::setWindowText(m_window_first_played, m_first_played);
	pfc::setWindowText(m_window_last_played, m_last_played);
	pfc::setWindowText(m_window_playcount, m_playcount);
	pfc::setWindowText(m_window_rating, m_rating);

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
	m_window_rating.SetWindowTextW(L"");
}

void CDialogEdit::OnCloseCmd(uint32_t, int nID, CWindow)
{
	if (nID == IDOK)
	{
		m_added = pfc::getWindowText(m_window_added);
		m_first_played = pfc::getWindowText(m_window_first_played);
		m_last_played = pfc::getWindowText(m_window_last_played);
		m_playcount = pfc::getWindowText(m_window_playcount);
		m_rating = pfc::getWindowText(m_window_rating);

		if (IsTF(m_added)) m_history_added.add_item(m_added);
		if (IsTF(m_first_played)) m_history_first_played.add_item(m_first_played);
		if (IsTF(m_last_played)) m_history_last_played.add_item(m_last_played);
		if (IsTF(m_playcount)) m_history_playcount.add_item(m_playcount);
		if (IsTF(m_rating)) m_history_playcount.add_item(m_rating);
	}

	EndDialog(nID);
}

void CDialogEdit::OnPresets(uint32_t, int, CWindow)
{
	HMENU menu = CreatePopupMenu();
	AppendMenuW(menu, GetMenuFlag("foo_playcount"), ID_MENU_FOO_PLAYCOUNT, L"foo_playcount");
	AppendMenuW(menu, GetMenuFlag("foo_lastfm_playcount_sync"), ID_MENU_FOO_LASTFM_PLAYCOUNT_SYNC, L"foo_lastfm_playcount_sync");

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
		m_window_rating.SetWindowTextW(L"[%rating%]");
		break;
	case ID_MENU_FOO_LASTFM_PLAYCOUNT_SYNC:
		m_window_added.SetWindowTextW(L"");
		m_window_first_played.SetWindowTextW(L"[%lfm_first_played%]");
		m_window_last_played.SetWindowTextW(L"[%lfm_last_played%]");
		m_window_playcount.SetWindowTextW(L"[%lfm_playcount%]");
		m_window_rating.SetWindowTextW(L"");
		break;
	}
}
