#include "vgui2_stub_types.h"
#include "hud.h"

#include "cvardef.h"
#include "interface.h"

#if defined(VGUI2_STUB_MODE)

#define LOG_PREFIX "[VGUI2-STUB] "

namespace vgui2
{
ISurface *Stub_g_pVGuiSurface = NULL;
IInputInternal *Stub_g_pVGuiInput = NULL;
IVGui *Stub_g_pVGui = NULL;
IPanel *Stub_g_pVGuiPanel = NULL;
ILocalize *Stub_g_pVGuiLocalize = NULL;
ISchemeManager *Stub_g_pVGuiSchemeManager = NULL;
ISystem *Stub_g_pVGuiSystem = NULL;
}

bool Stub_ConnectTier1Libraries(CreateInterfaceFn *, int)
{
	gEngfuncs.Con_Printf(LOG_PREFIX "ConnectTier1Libraries called (STUB)\n");
	return true;
}

bool Stub_ConnectTier2Libraries(CreateInterfaceFn *, int)
{
	gEngfuncs.Con_Printf(LOG_PREFIX "ConnectTier2Libraries called (STUB)\n");
	return true;
}

bool Stub_VGui_InitInterfacesList(const char *moduleName, CreateInterfaceFn *, int)
{
	gEngfuncs.Con_Printf(LOG_PREFIX "VGui_InitInterfacesList called (STUB)\n");
	return true;
}

#endif