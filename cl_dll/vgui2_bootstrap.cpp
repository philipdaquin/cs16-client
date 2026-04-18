#include "hud.h"
#include "vgui2_bootstrap.h"

int VGui2_Initialize(cl_enginefunc_t *pEnginefuncs);
int VGui2_VidInit();
int VGui2_Shutdown();

namespace
{
bool g_vgui2Ready = false;
bool g_vgui2VidInit = false;
}

void VGUI2_Bootstrap()
{
	if (g_vgui2Ready)
		return;

	VGui2_Initialize(&gEngfuncs);
	g_vgui2Ready = true;
}

void VGUI2_OnVidInit()
{
	if (!g_vgui2Ready)
		VGUI2_Bootstrap();

	VGui2_VidInit();
	g_vgui2VidInit = true;
}

void VGUI2_OnShutdown()
{
	if (!g_vgui2Ready)
		return;

	VGui2_Shutdown();
	g_vgui2Ready = false;
	g_vgui2VidInit = false;
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
