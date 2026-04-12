#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/EditablePanel.h>
#include "VGUI/counterstrikeviewport.h"

class CBuySubMenu : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CBuySubMenu, vgui2::EditablePanel);

public:
	CBuySubMenu(vgui2::Panel *parent, const char *panelName);
	void SetViewport(CCounterStrikeViewport *pViewport) { m_pViewport = pViewport; }
	void SetCategory(CCounterStrikeViewport::BuyMenuCategory_t category, bool isCT);
	void EnsureControlSettingsLoaded();

	virtual void ApplySchemeSettings(vgui2::IScheme *scheme) override;
	virtual void Paint() override;
	virtual void OnCommand(const char *command) override;

private:
	const char *GetResourceName() const;

	CCounterStrikeViewport *m_pViewport;
	CCounterStrikeViewport::BuyMenuCategory_t m_Category;
	bool m_bIsCT;
	bool m_bControlSettingsLoaded;
};

#endif
