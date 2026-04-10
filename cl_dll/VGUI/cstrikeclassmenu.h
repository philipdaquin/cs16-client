#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/EditablePanel.h>

class CCounterStrikeViewport;

class CClassMenu : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CClassMenu, vgui2::EditablePanel);

public:
	CClassMenu(vgui2::Panel *parent, const char *panelName);
	void SetViewport(CCounterStrikeViewport *pViewport) { m_pViewport = pViewport; }
	void SetMenuType(int menuType);

	virtual void ApplySchemeSettings(vgui2::HScheme scheme) override;
	virtual void Paint() override;
	virtual void OnCommand(const char *command) override;

private:
	void ReloadControlSettings();

	CCounterStrikeViewport *m_pViewport;
	int m_iMenuType;
};

#endif
