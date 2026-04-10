#include "counterstrikeviewport_interface.h"

#if !defined(VGUI2_STUB_MODE)

#include "hud.h"
#include "vgui2_bootstrap.h"

static CCounterStrikeViewport *g_pCounterStrikeViewport = NULL;
static const char *kViewportLogPrefix = "[VGUI2-VIEWPORT] ";

void VGUI2_CreateViewport()
{
	if (g_pCounterStrikeViewport || !VGUI2_IsReady() || !g_pVGuiSurface)
	{
		gEngfuncs.Con_Printf(kViewportLogPrefix "CreateViewport skipped viewport=%p ready=%d surface=%p\n",
			g_pCounterStrikeViewport, VGUI2_IsReady(), g_pVGuiSurface);
		return;
	}

	vgui2::VPANEL root = g_pVGuiSurface->GetEmbeddedPanel();
	if (!root)
	{
		gEngfuncs.Con_Printf(kViewportLogPrefix "CreateViewport failed: embedded/root panel is null\n");
		return;
	}

	gEngfuncs.Con_Printf(kViewportLogPrefix "CreateViewport using embedded/root panel %u\n", (unsigned int)root);

	g_pCounterStrikeViewport = new CCounterStrikeViewport(root);
	gEngfuncs.Con_Printf(kViewportLogPrefix "Viewport created: %p\n", g_pCounterStrikeViewport);
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

#endif
