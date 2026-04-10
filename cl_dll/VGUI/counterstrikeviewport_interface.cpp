#include "counterstrikeviewport_interface.h"

#if !defined(VGUI2_STUB_MODE)

#include "counterstrikeviewport.h"
#include <vgui/ISurface.h>

typedef float vec_t;
typedef vec_t vec3_t[3];

#include "hud.h"
#include "vgui2_bootstrap.h"

static CCounterStrikeViewport *g_pCounterStrikeViewport = NULL;

void VGUI2_CreateViewport()
{
	if (g_pCounterStrikeViewport || !VGUI2_IsReady() || !g_pVGuiSurface)
	{
		gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport skipped viewport=%p ready=%d surface=%p\n",
			g_pCounterStrikeViewport, VGUI2_IsReady(), g_pVGuiSurface);
		return;
	}

	vgui2::VPANEL root = g_pVGuiSurface->GetEmbeddedPanel();
	if (!root)
	{
		gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport failed: embedded/root panel is null\n");
		return;
	}

	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport using embedded/root panel %u\n", (unsigned int)root);

	g_pCounterStrikeViewport = new CCounterStrikeViewport(root);
	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] Viewport created: %p\n", g_pCounterStrikeViewport);
}

void VGUI2_DestroyViewport()
{
	delete g_pCounterStrikeViewport;
	g_pCounterStrikeViewport = NULL;
}

bool VGUI2_HasViewport()
{
	return g_pCounterStrikeViewport != NULL;
}

void VGUI2_ShowTeamMenu()
{
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowTeamMenu();
}

void VGUI2_ShowClassMenu(int menuType)
{
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowClassMenu(menuType);
}

void VGUI2_ShowBuyMenu()
{
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowBuyMenu();
}

void VGUI2_ShowBuySubMenu(int category)
{
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowBuySubMenu((CCounterStrikeViewport::BuyMenuCategory_t)category);
}

void VGUI2_HideAllGameMenus()
{
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->HideAllGameMenus();
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

#endif
