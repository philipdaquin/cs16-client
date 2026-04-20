#include "vgui_api.h"
#include "wrect.h"
typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);
#include "cdll_int.h"
#include "exportdef.h"

#include "winsani_in.h"

#include <FileSystem.h>
#include "tier1/interface.h"
#include "vgui/ISurface.h"
#include "vgui_controls/controls.h"
#include "render_api.h"
#include "BaseUISurface.h"
#include "GameUI/IGameConsole.h"
#include "IBaseUI.h"

#include "winsani_out.h"

#ifdef _WIN32
#include "winsani_in.h"
#include <Windows.h>
#include "winsani_out.h"
#else
#include <dlfcn.h>
#endif

namespace vgui2 {
cl_enginefunc_t gEngfuncs;
void VGui_Startup(int width, int height);
extern IGameConsole* staticGameConsole;
extern IBaseUI *staticUIFuncs;
extern vguiapi_t *g_api;
}
class CBaseViewport;
extern CBaseViewport *g_pViewport;
extern BaseUISurface* staticSurface;
extern void RegisterInterface();
extern void RegisterControls();

void VGuiWrap2_Startup();
void VGuiWrap2_Shutdown();
void VGuiWrap2_Paint();

qboolean g_bScissor;

using namespace vgui2;

int VGui2_COM_ExpandFileName(const char *fileName, char *nameOutBuffer, int nameOutBufferSize) {
    IFileSystem *fileSystem = vgui2::filesystem();
    return fileSystem && fileSystem->GetLocalPath(fileName, nameOutBuffer, nameOutBufferSize) != NULL;
}

void VGui2_pfnDrawSetTextColor(float r, float g, float b) {
    vgui2::surface()->DrawSetTextColor(r * 255, g * 255, b * 255, 255);
    gEngfuncs.pfnDrawSetTextColor(r, g, b);
}

extern "C" int DLLEXPORT VGui2_Initialize(cl_enginefunc_t *pEnginefuncs) {
    gEngfuncs = *pEnginefuncs;
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui2_Initialize engine=%p\n", (void *)pEnginefuncs);

	return 0;
}

extern "C" void DLLEXPORT VGui2_Startup()
{
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui2_Startup entry staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    VGuiWrap2_Startup();
}

extern "C" int DLLEXPORT VGui2_VidInit()
{
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui2_VidInit entry staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    void VGUI2_Draw_Init();
    VGUI2_Draw_Init();
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui2_VidInit done font init\n");
	return 0;
}

extern "C" void DLLEXPORT VGui2_Paint()
{
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui2_Paint entry staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    VGuiWrap2_Paint();
}

extern "C" int DLLEXPORT VGui2_Shutdown()
{
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui2_Shutdown entry staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    VGuiWrap2_Shutdown();
    return 0;
}

void VGuiWrap2_Startup()
{
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Startup entry staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    if( staticUIFuncs )
        return;
    RegisterInterface();
    RegisterControls();

    CreateInterfaceFn pEngineFactory = Sys_GetFactoryThis();
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Startup engineFactory=%p\n", (void *)pEngineFactory);
    staticUIFuncs = (IBaseUI *)pEngineFactory(BASEUI_INTERFACE_VERSION, NULL);
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Startup IBaseUI=%p\n", (void *)staticUIFuncs);
    if (!staticUIFuncs)
    {
        gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Startup failed: IBaseUI missing\n");
        return;
    }
    staticUIFuncs->Initialize(&pEngineFactory, 1);
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Startup after Initialize staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    staticUIFuncs->Start(NULL, 0);
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Startup after Start staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
}

void VGuiWrap2_Shutdown()
{
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Shutdown entry staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    if( staticUIFuncs )
    {
        staticUIFuncs->Shutdown();
        staticUIFuncs = nullptr;
    }
}

void VGuiWrap2_Paint() {
    gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Paint entry staticUIFuncs=%p surface=%p viewport=%p\n",
        (void *)staticUIFuncs, (void *)staticSurface, (void *)g_pViewport);
    if (!staticUIFuncs) {
        gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGuiWrap2_Paint skipped: staticUIFuncs missing\n");
        return;
    }

    int wide, tall;
    staticSurface->GetScreenSize(wide, tall);
    g_bScissor = true;
    staticUIFuncs->Paint(0, 0, wide, tall);
    g_bScissor = false;
}

void VGuiWrap2_HideConsole()
{
    if(staticGameConsole)
        staticGameConsole->Hide();
}

void VGuiWrap2_ClearConsole()
{
    if(staticGameConsole)
        staticGameConsole->Clear();
}

void VGuiWrap2_ConPrintf(const char* msg)
{
    if(staticGameConsole)
        staticGameConsole->Printf("%s", msg);
}

void VGuiWrap2_ConDPrintf(const char* msg)
{
    if(staticGameConsole)
        staticGameConsole->DPrintf("%s", msg);
}
