#include "vgui_api.h"
#include "wrect.h"
typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);
#include "cdll_int.h"

#include "BaseUISurface.h"

#include <FileSystem.h>
#include "IBaseUI.h"
#include "vgui/IVGui.h"
#include "vgui/IPanel.h"
#include "vgui/IInput.h"
#include "vgui/IInputInternal.h"
#include "vgui/ISystem.h"
#include "vgui/KeyCode.h"
#include "vgui/MouseCode.h"

#include <stdint.h>

extern BaseUISurface *staticSurface;

void VGuiWrap2_Startup();
void VGuiWrap2_Shutdown();
void VGuiWrap2_Paint();
namespace vgui2 {

vguiapi_t *g_api;

IBaseUI *staticUIFuncs;

extern cl_enginefunc_t gEngfuncs;
extern vgui2::IInputInternal *g_pInputInternal;

void VGui_Startup(int width, int height) {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui_Startup width=%d height=%d\n", width, height);
	if (gEngfuncs.pfnGetGameDirectory == NULL) {
		return;
	}
    VGuiWrap2_Startup();
}

void VGui_Shutdown() {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui_Shutdown\n");
    VGuiWrap2_Shutdown();
    g_api->DrawShutdown();
}

void *VGui_GetPanel() {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui_GetPanel surface=%p\n", (void *)staticSurface);
	return staticSurface ? reinterpret_cast<void *>(static_cast<uintptr_t>(staticSurface->GetEmbeddedPanel())) : NULL;
}

void VGui_Paint() {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGui_Paint\n");
    VGuiWrap2_Paint();
}

void VGUI_Mouse(enum VGUI_MouseAction action, int code) {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI_Mouse action=%d code=%d input=%p\n", (int)action, code, (void *)g_pInputInternal);
	if (!g_pInputInternal) {
		return;
	}
	switch (action) {
    case VGUI_MouseAction::MA_PRESSED:
		g_pInputInternal->InternalMousePressed((vgui2::MouseCode)code);
		break;
	case VGUI_MouseAction::MA_DOUBLE:
		g_pInputInternal->InternalMouseDoublePressed((vgui2::MouseCode)code);
		break;
	case VGUI_MouseAction::MA_RELEASED:
		g_pInputInternal->InternalMouseReleased((vgui2::MouseCode)code);
		break;
	case VGUI_MouseAction::MA_WHEEL:
		g_pInputInternal->InternalMouseWheeled(-code);
		break;
	}
}

void VGUI_Key(enum VGUI_KeyAction action, enum VGUI_KeyCode code) {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI_Key action=%d code=%d input=%p\n", (int)action, (int)code, (void *)g_pInputInternal);
	if (!g_pInputInternal) {
		return;
	}
	switch (action) {
    case VGUI_KeyAction::KA_TYPED:
		if (VGUI_KeyCode::KEY_0 <= code && code <= VGUI_KeyCode::KEY_9) {
			g_pInputInternal->InternalKeyTyped('0' + (int)code);
		} else if (VGUI_KeyCode::KEY_A <= code && code <= VGUI_KeyCode::KEY_Z) {
			g_pInputInternal->InternalKeyTyped('a' + ((int)code - (int)VGUI_KeyCode::KEY_A));
		} else {
			switch (code) {
			case VGUI_KeyCode::KEY_LBRACKET:
				g_pInputInternal->InternalKeyTyped('[');
				break;
			case VGUI_KeyCode::KEY_RBRACKET:
				g_pInputInternal->InternalKeyTyped(']');
				break;
			case VGUI_KeyCode::KEY_SEMICOLON:
				g_pInputInternal->InternalKeyTyped('.');
				break;
			case VGUI_KeyCode::KEY_APOSTROPHE:
				g_pInputInternal->InternalKeyTyped('\'');
				break;
			case VGUI_KeyCode::KEY_BACKQUOTE:
				g_pInputInternal->InternalKeyTyped('`');
				break;
			case VGUI_KeyCode::KEY_COMMA:
				g_pInputInternal->InternalKeyTyped(',');
				break;
			case VGUI_KeyCode::KEY_PERIOD:
				g_pInputInternal->InternalKeyTyped('.');
				break;
			case VGUI_KeyCode::KEY_SLASH:
				g_pInputInternal->InternalKeyTyped('/');
				break;
			case VGUI_KeyCode::KEY_BACKSLASH:
				g_pInputInternal->InternalKeyTyped('\\');
				break;
			case VGUI_KeyCode::KEY_MINUS:
				g_pInputInternal->InternalKeyTyped('-');
				break;
			case VGUI_KeyCode::KEY_EQUAL:
				g_pInputInternal->InternalKeyTyped('=');
				break;
			case VGUI_KeyCode::KEY_SPACE:
				g_pInputInternal->InternalKeyTyped(' ');
				break;
			}
		}

		break;
    case VGUI_KeyAction::KA_PRESSED:
		g_pInputInternal->InternalKeyCodePressed((vgui2::KeyCode)((int)code + 1));
		g_pInputInternal->InternalKeyCodeTyped((vgui2::KeyCode)((int)code + 1));
		break;
	case VGUI_KeyAction::KA_RELEASED:
		g_pInputInternal->InternalKeyCodeReleased((vgui2::KeyCode)((int)code + 1));
		break;
	}
}

void VGUI_MouseMove(int x, int y) {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] VGUI_MouseMove x=%d y=%d input=%p\n", x, y, (void *)g_pInputInternal);
	if (!g_pInputInternal) {
		return;
	}
	g_pInputInternal->InternalCursorMoved(x, y);
}

extern "C" void EXPORT InitAPI(vguiapi_t * api) {
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] InitAPI api=%p\n", (void *)api);
	g_api = api;
	g_api->Startup = VGui_Startup;
	g_api->Shutdown = VGui_Shutdown;
	g_api->GetPanel = VGui_GetPanel;
	g_api->Paint = VGui_Paint;
	g_api->Mouse = VGUI_Mouse;
	g_api->Key = VGUI_Key;
	g_api->MouseMove = VGUI_MouseMove;
}

extern "C" void EXPORT InitVGUISupportAPI(vguiapi_t *api)
{
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] InitVGUISupportAPI api=%p\n", (void *)api);
	InitAPI(api);
}

#ifdef XASH_STATIC_GAMELIB
    typedef struct dllexport_s
    {
        const char *name;
        void *func;
    } dllexport_t;

    static dllexport_t switch_vgui_exports[] = {
            { "InitAPI", (void*)InitAPI },
            { NULL, NULL }
    };

    extern "C" int dll_register( const char *name, dllexport_t *exports );

    extern "C" int switch_installdll_vgui( void )
    {
        return dll_register( "vgui2_support", switch_vgui_exports );
    }
#endif

}

extern "C" vgui2::ISurface *g_pVGuiSurface = nullptr;
extern "C" vgui2::IInputInternal *g_pVGuiInput = nullptr;
extern "C" vgui2::IPanel *g_pVGuiPanel = nullptr;
extern "C" vgui2::IVGui *g_pVGui = nullptr;
extern "C" vgui2::ISystem *g_pVGuiSystem = nullptr;
