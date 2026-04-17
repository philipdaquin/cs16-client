#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/WizardPanel.h>

class CCounterStrikeViewport;
class CBuySubMenu;

class CBuyMenu : public vgui2::WizardPanel
{
	DECLARE_CLASS_SIMPLE(CBuyMenu, vgui2::WizardPanel);

public:
	CBuyMenu(vgui2::Panel *parent, const char *panelName);
	void SetViewport(CCounterStrikeViewport *pViewport) { m_pViewport = pViewport; }
	void EnsureControlSettingsLoaded();
	void ReloadControlSettings();
	void ShowPanel(bool bShow);
	void Update();
	void OnClose();

	virtual void ApplySchemeSettings(vgui2::IScheme *scheme) override;
	virtual void Paint() override;
	virtual void OnCommand(const char *command) override;

private:
	CCounterStrikeViewport *m_pViewport;
	CBuySubMenu *m_pMainMenu;
	bool m_bControlSettingsLoaded;
};

#endif
