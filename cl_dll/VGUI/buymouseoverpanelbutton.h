#pragma once

#include <KeyValues.h>
#include <vgui_controls/Button.h>

class MouseOverPanelButton : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(MouseOverPanelButton, vgui2::Button);

public:
	MouseOverPanelButton(vgui2::Panel *parent, const char *panelName, const char *text, vgui2::Panel *target = NULL, const char *command = NULL);

	int GetCost() const;
	const char *GetCommandString() const;
	bool IsCommandButton() const;

	virtual void ApplySettings(KeyValues *inResourceData);

private:
	int m_iCost;
	char m_szCommand[256];
};
