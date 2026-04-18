#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/WizardSubPanel.h>
#include "../../SourceSDK/public/tier1/utlvector.h"
#include "buymouseoverpanelbutton.h"
#include "VGUI/counterstrikeviewport.h"

class CBuySubMenu : public vgui2::WizardSubPanel
{
	DECLARE_CLASS_SIMPLE(CBuySubMenu, vgui2::WizardSubPanel);

public:
	CBuySubMenu(vgui2::Panel *parent, const char *panelName);
	~CBuySubMenu();
	void SetViewport(CCounterStrikeViewport *pViewport) { m_pViewport = pViewport; }
	void SetCategory(CCounterStrikeViewport::BuyMenuCategory_t category, bool isCT);
	void EnsureControlSettingsLoaded();
	void ShowPanel(bool bShow);

	virtual void ApplySchemeSettings(vgui2::IScheme *scheme) override;
	virtual void Paint() override;
	virtual void OnCommand(const char *command) override;
	virtual void SetVisible(bool state) override;
	virtual void DeleteSubPanels();
	virtual vgui2::WizardSubPanel *GetNextSubPanel() override;

private:
	const char *GetResourceName() const;
	CBuySubMenu *CreateNewSubMenu();
	vgui2::Panel *CreateControlByName(const char *controlName) override;
	vgui2::MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel);

	typedef struct
	{
		char filename[_MAX_PATH];
		CBuySubMenu *panel;
	} SubMenuEntry_t;

	CCounterStrikeViewport *m_pViewport;
	vgui2::EditablePanel *m_pPanel;
	vgui2::MouseOverPanelButton *m_pFirstButton;
	CUtlVector<SubMenuEntry_t> m_SubMenus;
	vgui2::WizardSubPanel *m_NextPanel;
	CCounterStrikeViewport::BuyMenuCategory_t m_Category;
	bool m_bIsCT;
	bool m_bControlSettingsLoaded;
};

#endif
