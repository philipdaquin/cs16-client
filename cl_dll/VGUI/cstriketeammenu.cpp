#include "cstriketeammenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"

CTeamMenu::CTeamMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	LoadControlSettings("Resource/UI/Teammenu.res");
}

void CTeamMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
}

void CTeamMenu::Paint()
{
	BaseClass::Paint();
}

void CTeamMenu::SetSpectateVisible(bool bVisible)
{
	vgui2::Panel *pSpectateButton = FindChildByName("specbutton");
	if (!pSpectateButton)
		return;

	pSpectateButton->SetVisible(bVisible);
	pSpectateButton->SetEnabled(bVisible);
}

void CTeamMenu::OnCommand(const char *command)
{
	if (!command || !command[0])
	{
		BaseClass::OnCommand(command);
		return;
	}

	if (!strnicmp(command, "jointeam", 8))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		char szCommand[64];
		snprintf(szCommand, sizeof(szCommand), "%s\n", command);
		VGUI2_RunClientCommand(szCommand);
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		VGUI2_RunClientCommand("cancelselect\n");
		return;
	}

	BaseClass::OnCommand(command);
}

#endif
