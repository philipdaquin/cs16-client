#include "VGUI/counterstrikeviewport.h"
#include "VGUI/vgui_legacy_api.h"
#include "VGUI/vgui_int.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>

static bool g_bVGuiInterfacesInitialized = false;
static vgui2::HScheme g_hClientScheme = 0;

namespace vgui2
{
HScheme VGui_GetDefaultScheme()
{
	return g_hClientScheme;
}
}

static void EnsureVGuiInterfaces()
{
	if (g_bVGuiInterfacesInitialized)
		return;

	if (!vgui2::VGui_InitInterfacesList("ClientDLL", NULL, 0))
	{
		VGuiLegacy_ConsolePrint("[VGUI2] Failed to initialize vgui_controls interfaces\n");
		return;
	}

	g_bVGuiInterfacesInitialized = true;
}

void VGui_EnsureInit()
{
	EnsureVGuiInterfaces();

	if (!g_hClientScheme && g_pVGuiSchemeManager)
	{
		g_hClientScheme = vgui2::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	}

	if (g_pVGuiLocalize && g_pFullFileSystem)
	{
		g_pVGuiLocalize->AddFile(g_pFullFileSystem, "resource/cstrike_english.txt");
		g_pVGuiLocalize->AddFile(g_pFullFileSystem, "resource/valve_english.txt");
	}
}
