#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/WizardSubPanel.h>
#include <utlvector.h>
#include "VGUI/counterstrikeviewport.h"

class MouseOverPanelButton;

class CBuySubMenu : public vgui2::WizardSubPanel
{
	DECLARE_CLASS_SIMPLE(CBuySubMenu, vgui2::WizardSubPanel);

public:
	CBuySubMenu(vgui2::Panel *parent, const char *panelName);
	void SetViewport(CCounterStrikeViewport *pViewport) { m_pViewport = pViewport; }
	void SetCategory(CCounterStrikeViewport::BuyMenuCategory_t category, bool isCT);
	void EnsureControlSettingsLoaded();
	void DeleteSubPanels();
	void ShowPanel(bool bShow);

	virtual void ApplySchemeSettings(vgui2::IScheme *scheme) override;
	virtual void Paint() override;
	virtual void SetVisible(bool state) override;
	virtual vgui2::WizardSubPanel *GetNextSubPanel() override;
	virtual vgui2::Panel *CreateControlByName(const char *controlName) override;
	virtual void OnCommand(const char *command) override;

private:
	CBuySubMenu *CreateNewSubMenu();
	MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel);
	const char *GetResourceName() const;

	CCounterStrikeViewport *m_pViewport;
	CCounterStrikeViewport::BuyMenuCategory_t m_Category;
	bool m_bIsCT;
	bool m_bControlSettingsLoaded;
	vgui2::EditablePanel *m_pPanel;
	MouseOverPanelButton *m_pFirstButton;
	vgui2::WizardSubPanel *m_NextPanel;
	struct SubMenuEntry_t
	{
		char filename[_MAX_PATH];
		CBuySubMenu *panel;
	};
	CUtlVector<SubMenuEntry_t> m_SubMenus;
};

#endif
