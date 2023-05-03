#pragma once

class CDialogInputBox : public CDialogImpl<CDialogInputBox>
{
public:
	CDialogInputBox();

	BEGIN_MSG_MAP_EX(CDialogInputBox)
		MSG_WM_INITDIALOG(OnInitDialog)
		COMMAND_ID_HANDLER_EX(IDC_BTN_CLEAR, OnClear)
		COMMAND_ID_HANDLER_EX(IDC_BTN_PRESETS, OnPresets)
		COMMAND_RANGE_HANDLER_EX(IDOK, IDCANCEL, OnCloseCmd)
		COMMAND_RANGE_HANDLER_EX(IDC_COMBO_ADDED, IDC_COMBO_PLAYCOUNT, OnUpdate)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_ADDED, m_history_added)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_FIRST_PLAYED, m_history_first_played)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_LAST_PLAYED, m_history_last_played)
		DROPDOWN_HISTORY_HANDLER(IDC_COMBO_PLAYCOUNT, m_history_playcount)
	END_MSG_MAP()

	enum { IDD = IDD_DIALOG_INPUTBOX };

	pfc::string8 m_added, m_first_played, m_last_played, m_playcount;

private:
	static bool CheckComponent(const pfc::string& name);
	static bool CheckString(const pfc::string8& str);

	BOOL OnInitDialog(CWindow, LPARAM);
	void OnClear(uint32_t, int, CWindow);
	void OnCloseCmd(uint32_t, int nID, CWindow);
	void OnPresets(uint32_t, int, CWindow);
	void OnUpdate(uint32_t, int, CWindow);

	CButton m_button_ok, m_button_presets, m_button_clear;
	CWindow m_window_added, m_window_first_played, m_window_last_played, m_window_playcount;
	cfg_dropdown_history m_history_added, m_history_first_played, m_history_last_played, m_history_playcount;
	fb2k::CCoreDarkModeHooks m_hooks;
};
