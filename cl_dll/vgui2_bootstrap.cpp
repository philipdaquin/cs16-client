#include "hud.h"
#include "cl_util.h"
#include "vgui2_bootstrap.h"
#include "vgui2_stub_types.h"

#include "port.h"
#include "cvardef.h"
#include "interface.h"

#define LOG_PREFIX "[VGUI2-BOOTSTRAP] "

struct VGui2BootstrapState
{
	bool attempted;
	bool tier1Connected;
	bool tier2Connected;
	bool interfacesInitialized;
	bool ready;
	bool schemeLoaded;
	bool testPanelCreated;

	vgui2::VPANEL embeddedPanel;
	void *testPanel;
	vgui2::HScheme clientScheme;
};

static VGui2BootstrapState state = {};

static cvar_t cl_vgui2_bootstrap = { "cl_vgui2_bootstrap", "1", 0 };
static cvar_t cl_vgui2_testpanel = { "cl_vgui2_testpanel", "1", 0 };

static const char *GetVGui2FactoryFuncName()
{
	return "VGui2_GetFactory";
}

static CreateInterfaceFn ResolveVGui2Factory(const char **outModuleName)
{
	*outModuleName = "unknown";
	CreateInterfaceFn factory = NULL;

#if defined(_WIN32)
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
	if (handle)
	{
		factory = (CreateInterfaceFn)dlsym(handle, GetVGui2FactoryFuncName());
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
	if (mainHandle)
	{
		factory = (CreateInterfaceFn)dlsym(mainHandle, GetVGui2FactoryFuncName());
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
	gEngfuncs.pfnRegisterVariable("cl_vgui2_bootstrap", "1", 0);
	gEngfuncs.pfnRegisterVariable("cl_vgui2_testpanel", "1", 0);
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
	if (state.attempted)
		return state.ready;

	state.attempted = true;

	gEngfuncs.Con_Printf("\n========== CLIENT VGUI2_Bootstrap HIT ==========\n");

	RegisterCvars();

	gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap starting...\n");

#if defined(VGUI2_STUB_MODE)
	gEngfuncs.Con_Printf(LOG_PREFIX "ARM64 stub mode - using runtime-resolved engine interfaces\n");
	ConnectTier1Libraries(NULL, 0);
	state.tier1Connected = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "Tier1 connected (STUB)\n");

	ConnectTier2Libraries(NULL, 0);
	state.tier2Connected = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "Tier2 connected (STUB)\n");

	if (!VGui_InitInterfacesList("CLIENT", NULL, 0))
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGui_InitInterfacesList failed\n");
		return false;
	}
	state.interfacesInitialized = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "VGUI interfaces initialized (STUB)\n");

	state.ready = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap ready (STUB mode)\n");

	state.schemeLoaded = false;
	gEngfuncs.Con_Printf(LOG_PREFIX "Scheme load skipped (ARM64 stub)\n");

	return true;

#else

	CreateInterfaceFn factories[8];
	int count = BuildFactoryList(factories, ARRAYSIZE(factories));
	if (count <= 0)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap FAILED - BuildFactoryList returned 0 factories\n");
		return false;
	}

	gEngfuncs.Con_Printf(LOG_PREFIX "Connecting tier1 libraries...\n");
	ConnectTier1Libraries(factories, count);
	state.tier1Connected = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "Tier1 connected\n");

	gEngfuncs.Con_Printf(LOG_PREFIX "Connecting tier2 libraries...\n");
	ConnectTier2Libraries(factories, count);
	state.tier2Connected = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "Tier2 connected\n");

	gEngfuncs.Con_Printf(LOG_PREFIX "Initializing VGUI interfaces...\n");
	if (!VGui_InitInterfacesList("CLIENT", factories, count))
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGui_InitInterfacesList failed\n");
		return false;
	}
	state.interfacesInitialized = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "VGUI interfaces initialized\n");

	gEngfuncs.Con_Printf(LOG_PREFIX "  g_pVGui: %s\n", g_pVGui ? "OK" : "NULL");
	gEngfuncs.Con_Printf(LOG_PREFIX "  g_pVGuiPanel: %s\n", g_pVGuiPanel ? "OK" : "NULL");
	gEngfuncs.Con_Printf(LOG_PREFIX "  g_pVGuiSurface: %s\n", g_pVGuiSurface ? "OK" : "NULL");
	gEngfuncs.Con_Printf(LOG_PREFIX "  g_pVGuiSchemeManager: %s\n", g_pVGuiSchemeManager ? "OK" : "NULL");
	gEngfuncs.Con_Printf(LOG_PREFIX "  g_pVGuiSystem: %s\n", g_pVGuiSystem ? "OK" : "NULL");

	if (!g_pVGui || !g_pVGuiPanel || !g_pVGuiSurface || !g_pVGuiSchemeManager || !g_pVGuiSystem)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "Critical interfaces missing - bootstrap failed\n");
		return false;
	}

	state.ready = true;
	gEngfuncs.Con_Printf(LOG_PREFIX "Bootstrap ready\n");

	gEngfuncs.Con_Printf(LOG_PREFIX "Attempting scheme load...\n");
	state.clientScheme = g_pVGuiSchemeManager->LoadSchemeFromFile("resource/ClientScheme.res", "clientscheme");
	state.schemeLoaded = (state.clientScheme != 0);
	gEngfuncs.Con_Printf(LOG_PREFIX "Scheme load: %s\n", state.schemeLoaded ? "OK" : "FAILED (non-fatal)");

	return true;
#endif
}

void VGUI2_OnShutdown()
{
	if (state.testPanelCreated)
	{
		VGUI2_DestroyTestPanel();
	}
}

void VGUI2_CreateTestPanel()
{
	gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_CreateTestPanel() ENTRY\n");

	if (!state.ready)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_CreateTestPanel() SKIP - bootstrap not ready\n");
		return;
	}

	if (state.testPanelCreated)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_CreateTestPanel() SKIP - already created\n");
		return;
	}

#if !defined(VGUI2_STUB_MODE)
	vgui2::ISurface *surface = g_pVGuiSurface;
	vgui2::IVGui *ivgui = g_pVGui;
	vgui2::IPanel *ipan = g_pVGuiPanel;

	vgui2::VPANEL root = surface->GetEmbeddedPanel();
	state.embeddedPanel = root;
	gEngfuncs.Con_Printf(LOG_PREFIX "Embedded panel handle: %u\n", (unsigned int)root);

	vgui2::VPANEL testVPanel = ivgui->AllocPanel();
	gEngfuncs.Con_Printf(LOG_PREFIX "Allocated test panel: %u\n", (unsigned int)testVPanel);

	ipan->Init(testVPanel, NULL);
	ipan->SetPos(testVPanel, 100, 100);
	ipan->SetSize(testVPanel, 200, 150);
	ipan->SetVisible(testVPanel, true);

	if (root != 0)
	{
		ipan->SetParent(testVPanel, root);
		gEngfuncs.Con_Printf(LOG_PREFIX "Test panel parent set to embedded (root)\n");
	}
	else
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "Warning: root embedded panel is 0, test panel has no parent\n");
	}

	gEngfuncs.Con_Printf(LOG_PREFIX "Test panel bounds: pos=(100,100) size=(200x150)\n");
	gEngfuncs.Con_Printf(LOG_PREFIX "Test panel creation SUCCEEDED\n");
#endif

	state.testPanelCreated = true;
}

void VGUI2_DestroyTestPanel()
{
	if (!state.testPanelCreated)
		return;

#if !defined(VGUI2_STUB_MODE)
	if (g_pVGui && state.embeddedPanel != 0)
	{
		g_pVGui->FreePanel(state.embeddedPanel);
	}
#endif

	state.testPanelCreated = false;
	state.embeddedPanel = 0;
	state.testPanel = NULL;

	gEngfuncs.Con_Printf(LOG_PREFIX "Test panel destroyed\n");
}

void VGUI2_OnVidInit()
{
	if (!VGUI2_IsReady())
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_OnVidInit() SKIP - bootstrap not ready\n");
		return;
	}

	if (cl_vgui2_testpanel.value == 0.0f)
	{
		gEngfuncs.Con_Printf(LOG_PREFIX "VGUI2_OnVidInit() SKIP - cl_vgui2_testpanel is 0\n");
		return;
	}

	if (!state.testPanelCreated)
	{
		VGUI2_CreateTestPanel();
	}
}