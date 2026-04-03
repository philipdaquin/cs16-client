#include "VGUI/counterstrikeviewport.h"
#include "VGUI/vgui_legacy_api.h"
#include "VGUI/vgui_int.h"
#include "cdll_dll.h"

#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

void VGui_EnsureInit();

CCounterStrikeViewport *g_pViewport = NULL;

static bool IsMigratedMenuType(int menuType)
{
	switch (menuType)
	{
	case MENU_TEAM:
	case MENU_CLASS_T:
	case MENU_CLASS_CT:
	case MENU_BUY:
	case MENU_BUY_PISTOL:
	case MENU_BUY_SHOTGUN:
	case MENU_BUY_RIFLE:
	case MENU_BUY_SUBMACHINEGUN:
	case MENU_BUY_MACHINEGUN:
	case MENU_BUY_ITEM:
		return true;
	default:
		return false;
	}
}

void VGui_Startup()
{
	if (g_pViewport)
	{
		g_pViewport->SetSize(VGuiLegacy_GetScreenWidth(), VGuiLegacy_GetScreenHeight());
		return;
	}

	VGui_EnsureInit();

	if (!g_pVGuiSurface || !g_pVGuiPanel || !g_pVGuiInput)
	{
		VGuiLegacy_ConsolePrint("[VGUI2] Missing surface/panel/input interfaces in VGui_Startup\n");
		return;
	}

	g_pViewport = new CCounterStrikeViewport();
	g_pViewport->SetParent(g_pVGuiSurface->GetEmbeddedPanel());
	g_pViewport->SetVisible(true);
	g_pViewport->Start();
}

void VGui_Shutdown()
{
	if (!g_pViewport)
		return;

	g_pViewport->SetVisible(false);
	g_pViewport->MarkForDeletion();
	g_pViewport = NULL;
}

void VGui_ShowPanel(int menuType, bool show)
{
	if (!g_pViewport || !IsMigratedMenuType(menuType))
		return;

	g_pViewport->ShowPanel(menuType, show);
}

void VGui_HidePanel(int menuType)
{
	VGui_ShowPanel(menuType, false);
}

bool VGui_IsAnyPanelVisible()
{
	return g_pViewport && g_pViewport->GetActivePanel() != NULL;
}

bool VGui_IsPanelVisible(int menuType)
{
	if (!g_pViewport || !IsMigratedMenuType(menuType))
		return false;

	switch (menuType)
	{
	case MENU_TEAM:
		return g_pViewport->FindPanelByName(PANEL_TEAM) && g_pViewport->FindPanelByName(PANEL_TEAM)->IsVisible();
	case MENU_CLASS_T:
		return g_pViewport->FindPanelByName(PANEL_CLASS_T) && g_pViewport->FindPanelByName(PANEL_CLASS_T)->IsVisible();
	case MENU_CLASS_CT:
		return g_pViewport->FindPanelByName(PANEL_CLASS_CT) && g_pViewport->FindPanelByName(PANEL_CLASS_CT)->IsVisible();
	default:
		return g_pViewport->FindPanelByName(PANEL_BUY) && g_pViewport->FindPanelByName(PANEL_BUY)->IsVisible();
	}
}

bool VGui_IsSpectatorGUIVisible()
{
	return g_pViewport && g_pViewport->FindPanelByName(PANEL_SPECGUI) && g_pViewport->FindPanelByName(PANEL_SPECGUI)->IsVisible();
}

void VGui_Reset()
{
	if (g_pViewport)
	{
		g_pViewport->Reset();
	}
}
