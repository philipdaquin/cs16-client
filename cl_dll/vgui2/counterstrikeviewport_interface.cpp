#include "counterstrikeviewport_interface.h"

#include "CHudViewPort.h"
#include "CBaseViewport.h"
#include "hud.h"

bool VGUI2_HasViewport()
{
	return g_pViewport != nullptr;
}

void *VGUI2_GetViewportPtr()
{
	return g_pViewport;
}

void VGUI2_CreateViewport()
{
}

void VGUI2_DestroyViewport()
{
}

void VGUI2_ShowTeamMenu()
{
	if (g_pViewport)
		static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(MENU_TEAM);
}

void VGUI2_ShowClassMenu(int menuType)
{
	if (g_pViewport)
		static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(menuType);
}

void VGUI2_ShowBuyMenu()
{
	if (g_pViewport)
		static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(MENU_BUY);
}

void VGUI2_ShowBuySubMenu(int category)
{
	if (!g_pViewport)
		return;

	static const int kMenuTypes[] =
	{
		MENU_BUY_PISTOL,
		MENU_BUY_SHOTGUN,
		MENU_BUY_SUBMACHINEGUN,
		MENU_BUY_RIFLE,
		MENU_BUY_MACHINEGUN,
		MENU_BUY_ITEM
	};

	if (category < 0 || category >= static_cast<int>(sizeof(kMenuTypes) / sizeof(kMenuTypes[0])))
		return;

	static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(kMenuTypes[category]);
}

void VGUI2_HideAllGameMenus()
{
	if (g_pViewport)
		g_pViewport->HideAllVGUIMenu();
}

bool VGUI2_ShouldCaptureInput()
{
	return g_pViewport && g_pViewport->GetActivePanel() != nullptr;
}

int VGUI2_GetLocalPlayerTeam()
{
	return g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber;
}

void VGUI2_RunClientCommand(const char *command)
{
	if (!command || !command[0])
		return;

	gEngfuncs.pfnClientCmd((char *)command);
}
