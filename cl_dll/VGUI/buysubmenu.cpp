#include "VGUI/buysubmenu.h"
#include "VGUI/buymenu.h"
#include "VGUI/buymouseoverpanelbutton.h"

#include <string.h>

using namespace vgui2;

CBuySubMenu::CBuySubMenu(Panel *parent, const char *panelName, CBuyMenu *pOwner) : BaseClass(parent, panelName)
{
	m_pOwner = pOwner;
	m_szResourceFile[0] = '\0';

	SetVisible(false);
	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
}

CBuySubMenu::~CBuySubMenu()
{
}

bool CBuySubMenu::LoadFromResource(const char *resourceFile)
{
	if (!resourceFile || !resourceFile[0])
		return false;

	strncpy(m_szResourceFile, resourceFile, sizeof(m_szResourceFile) - 1);
	m_szResourceFile[sizeof(m_szResourceFile) - 1] = '\0';

	LoadControlSettings(resourceFile);
	m_Buttons.RemoveAll();
	CollectButtons(this);
	return true;
}

void CBuySubMenu::UpdateAffordability(int money)
{
	for (int i = 0; i < m_Buttons.Count(); ++i)
	{
		MouseOverPanelButton *button = m_Buttons[i];
		if (!button)
			continue;

		if (button->GetCost() > 0)
			button->SetEnabled(money >= button->GetCost());
	}
}

const char *CBuySubMenu::GetResourceFile() const
{
	return m_szResourceFile;
}

bool CBuySubMenu::HasCostedButtons() const
{
	for (int i = 0; i < m_Buttons.Count(); ++i)
	{
		if (m_Buttons[i] && m_Buttons[i]->GetCost() > 0)
			return true;
	}

	return false;
}

void CBuySubMenu::OnCommand(const char *command)
{
	if (m_pOwner)
		m_pOwner->OnCommand(command);
}

void CBuySubMenu::CollectButtons(Panel *panel)
{
	if (!panel)
		return;

	MouseOverPanelButton *hoverButton = dynamic_cast<MouseOverPanelButton *>(panel);
	if (hoverButton)
		m_Buttons.AddToTail(hoverButton);

	for (int i = 0; i < panel->GetChildCount(); ++i)
		CollectButtons(panel->GetChild(i));
}

