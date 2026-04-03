#include "VGUI/buymouseoverpanelbutton.h"

#include <string.h>
#include <vgui_controls/Controls.h>

using namespace vgui2;

MouseOverPanelButton::MouseOverPanelButton(Panel *parent, const char *panelName, const char *text, Panel *target, const char *command)
	: BaseClass(parent, panelName, text, target, command)
{
	m_iCost = 0;
	m_szCommand[0] = '\0';
}

int MouseOverPanelButton::GetCost() const
{
	return m_iCost;
}

const char *MouseOverPanelButton::GetCommandString() const
{
	return m_szCommand;
}

bool MouseOverPanelButton::IsCommandButton() const
{
	return m_szCommand[0] != '\0';
}

void MouseOverPanelButton::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	m_iCost = inResourceData->GetInt("cost", 0);

	const char *command = inResourceData->GetString("command", "");
	strncpy(m_szCommand, command, sizeof(m_szCommand) - 1);
	m_szCommand[sizeof(m_szCommand) - 1] = '\0';
}

static Panel *Create_MouseOverPanelButton()
{
	return new MouseOverPanelButton(NULL, NULL, "");
}

DECLARE_BUILD_FACTORY_CUSTOM_ALIAS(MouseOverPanelButton, MouseOverPanelButton, Create_MouseOverPanelButton);

