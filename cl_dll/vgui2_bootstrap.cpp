#if !defined(VGUI2_STUB_MODE)
#define __INTERFACE_H__
#include <tier1/interface.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Controls.h>
#include <IEngineVGui.h>

namespace vgui2
{
bool VGui_InitInterfacesList(const char *moduleName, CreateInterfaceFn *factoryList, int numFactories);
}
#endif

#include "hud.h"
#include "cl_util.h"
#include "vgui2_bootstrap.h"
#include "vgui2_stub_types.h"
#include "vgui2_client_runtime.h"
#include "VGUI/cs_rootpanel.h"
#include "VGUI/counterstrikeviewport_interface.h"

#include "cvardef.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (int)(sizeof(x) / sizeof((x)[0]))
#endif

#ifndef CREATEINTERFACE_PROCNAME
#define CREATEINTERFACE_PROCNAME "CreateInterface"
#endif

#ifndef VGUI_INPUTINTERNAL_INTERFACE_VERSION
#define VGUI_INPUTINTERNAL_INTERFACE_VERSION "VGUI_InputInternal001"
#endif

#ifndef VGUI_LOCALIZE_INTERFACE_VERSION
#define VGUI_LOCALIZE_INTERFACE_VERSION "VGUI_Localize003"
#endif

#define LOG_PREFIX "[VGUI2-BOOTSTRAP] "

struct VGui2BootstrapState
{
	bool inProgress;
	bool attempted;
	bool tier1Connected;
	bool tier2Connected;
	bool interfacesInitialized;
	bool ready;
	bool schemeLoaded;
	vgui2::HScheme clientScheme;
};

static VGui2BootstrapState state = {};

static cvar_t *cl_vgui2_bootstrap = NULL;
static cvar_t *cl_vgui2_menus = NULL;

#if !defined(VGUI2_STUB_MODE)
static void VGUI2_PreRender()
{
	if (!state.ready || !g_pVGuiPanel)
		return;

	int wide = gHUD.m_scrinfo.iWidth;
	int tall = gHUD.m_scrinfo.iHeight;
	if (wide <= 0 || tall <= 0)
	{
		wide = ScreenWidth;
		tall = ScreenHeight;
	}

	if (wide <= 0 || tall <= 0)
	{
		if (g_pVGuiSurface)
			g_pVGuiSurface->GetScreenSize(wide, tall);
	}

	if (wide <= 0)
		wide = 640;
	if (tall <= 0)
		tall = 480;

	if (g_pEngineVGui)
	{
		vgui2::VPANEL rootHost = g_pEngineVGui->GetPanel(PANEL_CLIENTDLL);
		if (rootHost)
			g_pVGuiPanel->SetSize(rootHost, wide, tall);
	}

	VGUI2_UpdateClientRootPanelBounds();

	vgui2::VPANEL root = VGUI2_GetClientRootPanel();
	if (root)
	{
		g_pVGuiPanel->SetPos(root, 0, 0);
		g_pVGuiPanel->SetSize(root, wide, tall);
	}
}
#endif

static void ResetBootstrapAttemptState()
{
	state.inProgress = false;
	state.attempted = false;
	state.tier1Connected = false;
	state.tier2Connected = false;
	state.interfacesInitialized = false;
	state.ready = false;
	state.schemeLoaded = false;
	state.clientScheme = 0;
}

static bool BootstrapFail(int step, const char *name, const char *version)
{
	gEngfuncs.Con_Printf(LOG_PREFIX "FAIL Step %d - %s ('%s') resolved to NULL\n",
		step, name, version ? version : "<none>");
	ResetBootstrapAttemptState();
	return false;
}

static void BootstrapStepOk(int step, const char *name, const void *ptr)
{
	gEngfuncs.Con_Printf(LOG_PREFIX "Step %d OK - %s=%p\n", step, name, ptr);
}

static const char *GetVGui2FactoryFuncName()
{
	return "VGui2_GetFactory";
}

#if defined(__EMSCRIPTEN__)
extern "C" CreateInterfaceFn VGui2_GetFactory(void);
#endif

static bool InitVGuiInterfacesList(const char *moduleName, CreateInterfaceFn *factories, int count)
{
#if defined(VGUI2_STUB_MODE)
	return VGui_InitInterfacesList(moduleName, factories, count);
#else
	return vgui2::VGui_InitInterfacesList(moduleName, factories, count);
#endif
}

static CreateInterfaceFn ResolveVGui2Factory(const char **outModuleName)
{
	*outModuleName = "unknown";
	CreateInterfaceFn factory = NULL;

#if defined(__EMSCRIPTEN__)
	gEngfuncs.Con_Printf(LOG_PREFIX "Emscripten: resolving VGui2_GetFactory via direct engine import\n");
	factory = VGui2_GetFactory();
	gEngfuncs.Con_Printf(LOG_PREFIX "Emscripten: VGui2_GetFactory() returned %p\n", factory);
	if (factory)
	{
		*outModuleName = "engine main module (direct import)";
		return factory;
	}
	gEngfuncs.Con_Printf(LOG_PREFIX "Emscripten: direct VGui2_GetFactory() returned NULL\n");
	return NULL;
#elif defined(_WIN32)
	const char *modules[] = { "xash3d", "xash3d-fwgs", "hw", "sw" };
	for (int i = 0; i < ARRAYSIZE(modules); ++i)
	{
		HMODULE hModule = GetModuleHandleA(modules[i]);
		if (!hModule)
			continue;

		factory = (CreateInterfaceFn)GetProcAddress(hModule, GetVGui2FactoryFuncName());
		if (factory)
		{
			*outModuleName = modules[i];
			gEngfuncs.Con_Printf(LOG_PREFIX "Resolved VGui2_GetFactory from module '%s'\n", modules[i]);
			return factory;
		}

		factory = (CreateInterfaceFn)GetProcAddress(hModule, CREATEINTERFACE_PROCNAME);
		if (factory)
		{
			*outModuleName = modules[i];
			gEngfuncs.Con_Printf(LOG_PREFIX "Resolved CreateInterface from module '%s' (fallback)\n", modules[i]);
			return factory;
		}
	}
	gEngfuncs.Con_Printf(LOG_PREFIX "Windows: could not resolve factory from any known engine module\n");
	return NULL;

#elif defined(__APPLE__)
	void *handle = dlopen(NULL, RTLD_NOW);
	const char *openError = dlerror();
	gEngfuncs.Con_Printf(LOG_PREFIX "macOS: dlopen(NULL) handle=%p error=%s\n",
		handle, openError ? openError : "<none>");
	if (handle)
	{
		factory = (CreateInterfaceFn)dlsym(handle, GetVGui2FactoryFuncName());
		const char *symError = dlerror();
		gEngfuncs.Con_Printf(LOG_PREFIX "macOS: dlsym('%s') -> %p error=%s\n",
			GetVGui2FactoryFuncName(), factory, symError ? symError : "<none>");
		if (factory)
		{
			*outModuleName = "main executable (dlopen NULL)";
			gEngfuncs.Con_Printf(LOG_PREFIX "Resolved VGui2_GetFactory from main executable\n");
			dlclose(handle);
			return factory;
		}
		dlclose(handle);
	}
	gEngfuncs.Con_Printf(LOG_PREFIX "macOS: could not resolve VGui2_GetFactory from main executable\n");
	return NULL;

#else
	void *mainHandle = dlopen(NULL, RTLD_NOW);
	const char *openError = dlerror();
	gEngfuncs.Con_Printf(LOG_PREFIX "Linux: dlopen(NULL) handle=%p error=%s\n",
		mainHandle, openError ? openError : "<none>");
	if (mainHandle)
	{
		factory = (CreateInterfaceFn)dlsym(mainHandle, GetVGui2FactoryFuncName());
		const char *symError = dlerror();
		gEngfuncs.Con_Printf(LOG_PREFIX "Linux: dlsym('%s') -> %p error=%s\n",
			GetVGui2FactoryFuncName(), factory, symError ? symError : "<none>");
		if (factory)
		{
			*outModuleName = "main executable (dlopen NULL)";
			gEngfuncs.Con_Printf(LOG_PREFIX "Resolved VGui2_GetFactory from main executable\n");
			dlclose(mainHandle);
			return factory;
		}
		dlclose(mainHandle);
	}
	gEngfuncs.Con_Printf(LOG_PREFIX "Linux: could not resolve VGui2_GetFactory via dlopen\n");
	return NULL;
#endif
}

static int BuildFactoryList(CreateInterfaceFn *outFactories, int maxFactories)
{
	if (maxFactories < 1)
		return 0;

	const char *moduleName = NULL;
	CreateInterfaceFn factory = ResolveVGui2Factory(&moduleName);

	gEngfuncs.Con_Printf(LOG_PREFIX "BuildFactoryList: factory=%s, module=%s\n",
		factory ? "RESOLVED" : "NULL", moduleName);

	if (!factory)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "BuildFactoryList() returning 0 - NO FACTORY AVAILABLE\n");
		return 0;
	}

	gEngfuncs.Con_Printf(LOG_PREFIX "Using factory from: %s\n", moduleName);
	outFactories[0] = factory;
	return 1;
}

static void RegisterCvars()
{
	if (!cl_vgui2_bootstrap)
		cl_vgui2_bootstrap = gEngfuncs.pfnRegisterVariable("cl_vgui2_bootstrap", "1", 0);
	if (!cl_vgui2_menus)
		cl_vgui2_menus = gEngfuncs.pfnRegisterVariable("cl_vgui2_menus", "1", 0);

	gEngfuncs.Con_Printf(LOG_PREFIX "RegisterCvars bootstrap=%p(%.1f) menus=%p(%.1f)\n",
		cl_vgui2_bootstrap, cl_vgui2_bootstrap ? cl_vgui2_bootstrap->value : -1.0f,
		cl_vgui2_menus, cl_vgui2_menus ? cl_vgui2_menus->value : -1.0f);
}

bool VGUI2_IsReady()
{
	return state.ready;
}

bool VGUI2_HasScheme()
{
	return state.schemeLoaded && state.clientScheme != 0;
}

bool VGUI2_Bootstrap()
{
	gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_Bootstrap() ENTRY inProgress=%d attempted=%d ready=%d\n",
		state.inProgress ? 1 : 0, state.attempted ? 1 : 0, state.ready ? 1 : 0);

	if (state.inProgress)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_Bootstrap() SKIP - bootstrap already in progress\n");
		return false;
	}

	if (state.attempted && state.ready)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_Bootstrap() SKIP - already attempted, ready=%d\n", state.ready);
		return true;
	}

	ResetBootstrapAttemptState();
	state.inProgress = true;

	gEngfuncs.Con_Printf("\n========== CLIENT VGUI2_Bootstrap HIT ==========\n");

	RegisterCvars();

	gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap starting...\n");

#if defined(VGUI2_STUB_MODE)
	gEngfuncs.Con_Printf(LOG_PREFIX "Stub mode enabled - using stubbed VGUI2 client interfaces\n");
	ConnectTier1Libraries(NULL, 0);
	state.tier1Connected = true;
	BootstrapStepOk(1, "ConnectTier1Libraries(STUB)", (void *)1);

	ConnectTier2Libraries(NULL, 0);
	state.tier2Connected = true;
	BootstrapStepOk(2, "ConnectTier2Libraries(STUB)", (void *)1);

	if (!InitVGuiInterfacesList("CLIENT", NULL, 0))
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "FAIL Step 3 - VGui_InitInterfacesList('CLIENT') returned false\n");
		ResetBootstrapAttemptState();
		return false;
	}
	state.interfacesInitialized = true;
	BootstrapStepOk(3, "VGui_InitInterfacesList(STUB)", (void *)1);

	state.ready = true;
	state.attempted = true;
	state.inProgress = false;
	gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap READY (STUB mode) ready=%d\n", state.ready);

	state.schemeLoaded = false;
	gEngfuncs.Con_Printf(LOG_PREFIX "Scheme load skipped (stub mode)\n");
	gEngfuncs.Con_Printf(LOG_PREFIX "COMPLETE ready=%d attempted=%d (IVGui=%p IPanel=%p ISurface=%p IInputInternal=%p ILocalize=%p ISchemeManager=%p ISystem=%p viewport=%p)\n",
		state.ready ? 1 : 0, state.attempted ? 1 : 0,
		g_pVGui, g_pVGuiPanel, g_pVGuiSurface, g_pVGuiInput, g_pVGuiLocalize,
		g_pVGuiSchemeManager, g_pVGuiSystem, VGUI2_GetViewportPtr());

	return true;

#else
	gEngfuncs.Con_Printf(LOG_PREFIX "Stub mode DISABLED - using engine/Tier2 VGUI2 interfaces\n");

	CreateInterfaceFn factories[8];
	int count = BuildFactoryList(factories, ARRAYSIZE(factories));
	if (count <= 0)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap FAILED - BuildFactoryList returned 0 factories\n");
		ResetBootstrapAttemptState();
		return false;
	}

	gEngfuncs.Con_Printf(LOG_PREFIX "Connecting tier1 libraries...\n");
	ConnectTier1Libraries(factories, count);
	state.tier1Connected = true;
	BootstrapStepOk(1, "ConnectTier1Libraries", (void *)1);

	gEngfuncs.Con_Printf(LOG_PREFIX "Connecting tier2 libraries...\n");
	ConnectTier2Libraries(factories, count);
	state.tier2Connected = true;
	BootstrapStepOk(2, "ConnectTier2Libraries", (void *)1);

	gEngfuncs.Con_Printf(LOG_PREFIX "Initializing VGUI interfaces...\n");
	if (!InitVGuiInterfacesList("CLIENT", factories, count))
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "FAIL Step 3 - VGui_InitInterfacesList('CLIENT') returned false\n");
		ResetBootstrapAttemptState();
		return false;
	}
	state.interfacesInitialized = true;
	BootstrapStepOk(3, "VGui_InitInterfacesList", (void *)1);

	if (!VGUI2_ClientRuntimeInstall())
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "FAIL Step 3.5 - VGUI2_ClientRuntimeInstall() returned false\n");
		ResetBootstrapAttemptState();
		return false;
	}
	BootstrapStepOk(3, "VGUI2_ClientRuntimeInstall", (void *)1);

	if (!g_pVGui)
		return BootstrapFail(4, "IVGui", VGUI_IVGUI_INTERFACE_VERSION_GS);
	BootstrapStepOk(4, "IVGui", g_pVGui);

	if (!g_pVGuiPanel)
		return BootstrapFail(5, "IPanel", VGUI_PANEL_INTERFACE_VERSION_GS);
	BootstrapStepOk(5, "IPanel", g_pVGuiPanel);

	if (!g_pVGuiSurface)
		return BootstrapFail(6, "ISurface", VGUI_SURFACE_INTERFACE_VERSION_GS);
	BootstrapStepOk(6, "ISurface", g_pVGuiSurface);

	if (!g_pVGuiInput)
		return BootstrapFail(7, "IInputInternal", VGUI_INPUTINTERNAL_INTERFACE_VERSION);
	BootstrapStepOk(7, "IInputInternal", g_pVGuiInput);

	if (!g_pVGuiLocalize)
		gEngfuncs.Con_Printf(LOG_PREFIX "WARN Step 8 - ILocalize ('%s') resolved to NULL\n", VGUI_LOCALIZE_INTERFACE_VERSION);
	else
		BootstrapStepOk(8, "ILocalize", g_pVGuiLocalize);

	if (!g_pVGuiSchemeManager)
		return BootstrapFail(9, "ISchemeManager", VGUI_SCHEME_INTERFACE_VERSION_GS);
	BootstrapStepOk(9, "ISchemeManager", g_pVGuiSchemeManager);

	if (!g_pVGuiSystem)
		return BootstrapFail(10, "ISystem", VGUI_SYSTEM_INTERFACE_VERSION_GS);
	BootstrapStepOk(10, "ISystem", g_pVGuiSystem);

	state.ready = true;
	state.attempted = true;
	state.inProgress = false;
	gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap ready\n");

	gEngfuncs.Con_Printf(LOG_PREFIX "Attempting scheme load...\n");
	state.clientScheme = vgui2::scheme()->LoadSchemeFromFilePath("resource/ClientScheme.res", NULL, "ClientScheme");
	state.schemeLoaded = (state.clientScheme != 0);
	VGUI2_CreateClientRootPanel();
	gEngfuncs.Con_Printf(LOG_PREFIX "Scheme load: %s\n", state.schemeLoaded ? "OK" : "FAILED (non-fatal)");
	gEngfuncs.Con_Printf(LOG_PREFIX "COMPLETE ready=%d attempted=%d (IVGui=%p IPanel=%p ISurface=%p IInputInternal=%p ILocalize=%p ISchemeManager=%p ISystem=%p viewport=%p scheme=%d)\n",
		state.ready ? 1 : 0, state.attempted ? 1 : 0,
		g_pVGui, g_pVGuiPanel, g_pVGuiSurface, g_pVGuiInput, g_pVGuiLocalize,
		g_pVGuiSchemeManager, g_pVGuiSystem, VGUI2_GetViewportPtr(), state.schemeLoaded ? 1 : 0);

	return true;
#endif
}

void VGUI2_OnShutdown()
{
#if !defined(VGUI2_STUB_MODE)
	VGUI2_ClientRuntimeShutdown();
	VGUI2_DestroyViewport();
	VGUI2_DestroyClientRootPanel();
#endif
}

void VGUI2_OnVidInit()
{
	gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_OnVidInit() ENTRY ready=%d viewport=%p\n",
		state.ready ? 1 : 0, VGUI2_GetViewportPtr());

	if (!state.ready)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_OnVidInit() SKIP - bootstrap not ready\n");
		return;
	}

#if !defined(VGUI2_STUB_MODE)
	VGUI2_UpdateClientRootPanelBounds();
	VGUI2_ClientRuntimeOnVidInit(gHUD.m_scrinfo.iWidth, gHUD.m_scrinfo.iHeight);
	if (cl_vgui2_menus && cl_vgui2_menus->value != 0.0f)
		VGUI2_CreateViewport();
#endif

	gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_OnVidInit() EXIT ready=%d viewport=%p\n",
		state.ready ? 1 : 0, VGUI2_GetViewportPtr());
}

void VGUI2_RunFrame()
{
#if !defined(VGUI2_STUB_MODE)
	if (state.ready)
	{
		VGUI2_PreRender();
	}
#endif
}

void VGUI2_RenderFrame()
{
#if !defined(VGUI2_STUB_MODE)
	if (!state.ready)
		return;

	VGUI2_ClientRuntimeRunFrame();
#endif
}
