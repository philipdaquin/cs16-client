#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/EditablePanel.h>

class CCounterStrikeViewport;

class CBuyMenu : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CBuyMenu, vgui2::EditablePanel);

public:
	CBuyMenu(vgui2::Panel *parent, const char *panelName);
	void SetViewport(CCounterStrikeViewport *pViewport) { m_pViewport = pViewport; }

	virtual void ApplySchemeSettings(vgui2::HScheme scheme) override;
	virtual void Paint() override;
	virtual void OnCommand(const char *command) override;

private:
	CCounterStrikeViewport *m_pViewport;
};

#endif