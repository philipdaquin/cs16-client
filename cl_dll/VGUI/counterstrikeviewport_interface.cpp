#include "counterstrikeviewport_interface.h"

#if !defined(VGUI2_STUB_MODE)

#include "counterstrikeviewport.h"
#include <vgui/ISurface.h>
#include <tier2/tier2.h>

typedef float vec_t;
typedef vec_t vec3_t[3];

#include "hud.h"
#include "vgui2_bootstrap.h"

static CCounterStrikeViewport *g_pCounterStrikeViewport = NULL;


extern vgui2::IVGui *g_pVGui;
extern vgui2::IPanel *g_pVGuiPanel;

void VGUI2_CreateViewport()
{
	gEngfuncs.Con_Printf("PRE-VIEWPORT g_pVGui=%p g_pVGuiPanel=%p\n", g_pVGui, g_pVGuiPanel);

	if (!g_pVGui || !g_pVGuiPanel)
	{
		gEngfuncs.Con_Printf("PRE-VIEWPORT ABORTING - null interfaces!\n");
		return;
	}

	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport ENTRY ready=%d surface=%p viewport=%p\n",
		VGUI2_IsReady() ? 1 : 0, g_pVGuiSurface, g_pCounterStrikeViewport);

	if (g_pCounterStrikeViewport || !VGUI2_IsReady() || !g_pVGuiSurface)
	{
		gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport skipped viewport=%p ready=%d surface=%p\n",
			g_pCounterStrikeViewport, VGUI2_IsReady(), g_pVGuiSurface);
		return;
	}

	vgui2::VPANEL root = g_pVGuiSurface->GetEmbeddedPanel();
	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport root=%u\n", (unsigned int)root);
	if (!root)
	{
		gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport failed: embedded/root panel is null\n");
		return;
	}

	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport using embedded/root panel %u\n", (unsigned int)root);
	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport about to execute new CCounterStrikeViewport(root=%u)\n",
		(unsigned int)root);
	gEngfuncs.Con_Printf("PRE-VIEWPORT g_pVGui=%p g_pVGuiPanel=%p g_pVGuiSurface=%p root=%u\n",
		g_pVGui, g_pVGuiPanel, g_pVGuiSurface, (unsigned int)root);

	g_pCounterStrikeViewport = new CCounterStrikeViewport(root);
	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] CreateViewport assignment result viewport=%p\n", g_pCounterStrikeViewport);
	gEngfuncs.Con_Printf("[VGUI2-VIEWPORT] Viewport created: %p class=CCounterStrikeViewport kind=vgui2::EditablePanel root=%u\n",
		g_pCounterStrikeViewport, (unsigned int)root);
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

void *VGUI2_GetViewportPtr()
{
	return g_pCounterStrikeViewport;
}

void VGUI2_ShowTeamMenu()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_ShowTeamMenu viewport=%p target=CCounterStrikeViewport::ShowTeamMenu\n", g_pCounterStrikeViewport);
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowTeamMenu();
}

void VGUI2_ShowClassMenu(int menuType)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_ShowClassMenu viewport=%p target=CCounterStrikeViewport::ShowClassMenu type=%d\n",
		g_pCounterStrikeViewport, menuType);
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowClassMenu(menuType);
}

void VGUI2_ShowBuyMenu()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_ShowBuyMenu viewport=%p target=CCounterStrikeViewport::ShowBuyMenu\n",
		g_pCounterStrikeViewport);
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowBuyMenu();
}

void VGUI2_ShowBuySubMenu(int category)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_ShowBuySubMenu viewport=%p target=CCounterStrikeViewport::ShowBuySubMenu category=%d\n",
		g_pCounterStrikeViewport, category);
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->ShowBuySubMenu((CCounterStrikeViewport::BuyMenuCategory_t)category);
}

void VGUI2_HideAllGameMenus()
{
	if (g_pCounterStrikeViewport)
		g_pCounterStrikeViewport->HideAllGameMenus();
}

bool VGUI2_ShouldCaptureInput()
{
	return g_pCounterStrikeViewport && g_pCounterStrikeViewport->IsVisible();
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
