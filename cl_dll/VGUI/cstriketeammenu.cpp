#include "cstriketeammenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"

static const bool kTeamMenuEmptyDebug = true;

CTeamMenu::CTeamMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
{
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu ENTRY this=%p parent=%p name='%s'\n",
		this, parent, panelName ? panelName : "<null>");

	printf("BEFORE SetProportional\n");
	SetProportional(false);
	printf("AFTER SetProportional\n");

	SetVisible(false);
	printf("AFTER SetVisible\n");

	SetPaintBackgroundEnabled(true);
	printf("AFTER SetPaintBackgroundEnabled\n");

	printf("BEFORE kTeamMenuEmptyDebug check\n");

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu before LoadControlSettings empty=%d\n",
		kTeamMenuEmptyDebug ? 1 : 0);

	if (!kTeamMenuEmptyDebug)
	{
		LoadControlSettings("Resource/UI/Teammenu.res");
		printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu after LoadControlSettings panel=%p childSpec=%p\n",
			this, FindChildByName("specbutton"));
	}
	else
	{
		printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu SKIP LoadControlSettings due to kTeamMenuEmptyDebug=1\n");
	}

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu EXIT this=%p\n", this);
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
