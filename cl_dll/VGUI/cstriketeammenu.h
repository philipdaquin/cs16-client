#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/EditablePanel.h>

class CCounterStrikeViewport;

class CTeamMenu : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CTeamMenu, vgui2::EditablePanel);

public:
	CTeamMenu(vgui2::Panel *parent, const char *panelName);
	void SetViewport(CCounterStrikeViewport *pViewport) { m_pViewport = pViewport; }
	void SetSpectateVisible(bool bVisible);

	virtual void ApplySchemeSettings(vgui2::IScheme *scheme) override;
	virtual void Paint() override;
	virtual void OnCommand(const char *command) override;

private:
	CCounterStrikeViewport *m_pViewport;
};

#endif
