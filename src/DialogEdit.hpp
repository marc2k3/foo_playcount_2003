#pragma once

class CDialogEdit : public CDialogImpl<CDialogEdit>
{
public:
	CDialogEdit();

	BEGIN_MSG_MAP_EX(CDialogEdit)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BTN_CLEAR, OnClear)
		COMMAND_ID_HANDLER_EX(IDC_BTN_PRESETS, OnPresets)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_ADDED, m_history_added)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_FIRST_PLAYED, m_history_first_played)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_LAST_PLAYED, m_history_last_played)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_PLAYCOUNT, m_history_playcount)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_RATING, m_history_rating)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_EDIT };

	pfc::string8 m_added, m_first_played, m_last_played, m_playcount, m_rating;

private:
	static bool CheckComponent(const pfc::string& name);
	static bool GetMenuFlag(const pfc::string8& name);
	static bool IsTF(std::string_view str);

	BOOL OnInitDialog(CWindow, LPARAM);
	void OnClear(uint32_t, int, CWindow);
	void OnCloseCmd(uint32_t, int nID, CWindow);
	void OnPresets(uint32_t, int, CWindow);

	CButton m_button_presets, m_button_clear;
	CWindow m_window_added, m_window_first_played, m_window_last_played, m_window_playcount, m_window_rating;
	cfg_dropdown_history m_history_added, m_history_first_played, m_history_last_played, m_history_playcount, m_history_rating;
	fb2k::CCoreDarkModeHooks m_hooks;
};
