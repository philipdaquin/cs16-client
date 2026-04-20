#include "hud.h"
#include "vgui2_bootstrap.h"

extern "C" int VGui2_Initialize(cl_enginefunc_t *pEnginefuncs);
extern "C" int VGui2_VidInit();
extern "C" int VGui2_Shutdown();

namespace
{
bool g_vgui2Ready = false;
bool g_vgui2VidInit = false;
}

void VGUI2_Bootstrap()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_Bootstrap ready=%d vidinit=%d\n", g_vgui2Ready ? 1 : 0, g_vgui2VidInit ? 1 : 0);
	if (g_vgui2Ready)
		return;

	VGui2_Initialize(&gEngfuncs);
	g_vgui2Ready = true;
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_Bootstrap done ready=%d vidinit=%d\n", g_vgui2Ready ? 1 : 0, g_vgui2VidInit ? 1 : 0);
}

void VGUI2_OnVidInit()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_OnVidInit ready=%d vidinit=%d\n", g_vgui2Ready ? 1 : 0, g_vgui2VidInit ? 1 : 0);
	if (!g_vgui2Ready)
		VGUI2_Bootstrap();

	VGui2_VidInit();
	g_vgui2VidInit = true;
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_OnVidInit done ready=%d vidinit=%d\n", g_vgui2Ready ? 1 : 0, g_vgui2VidInit ? 1 : 0);
}

void VGUI2_OnShutdown()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_OnShutdown ready=%d vidinit=%d\n", g_vgui2Ready ? 1 : 0, g_vgui2VidInit ? 1 : 0);
	if (!g_vgui2Ready)
		return;

	VGui2_Shutdown();
	g_vgui2Ready = false;
	g_vgui2VidInit = false;
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI2_OnShutdown done ready=%d vidinit=%d\n", g_vgui2Ready ? 1 : 0, g_vgui2VidInit ? 1 : 0);
}

void VGUI2_RunFrame()
{
}

void VGUI2_RenderFrame()
{
}

bool VGUI2_IsReady()
{
	return g_vgui2Ready;
}

bool VGUI2_HasScheme()
{
	return g_vgui2VidInit;
}
