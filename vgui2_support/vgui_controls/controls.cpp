#include <string.h>
#include <locale.h>
#include <cstdarg>
#include <cstdio>
#include <vgui/IInputInternal.h>
#include <vgui/ISchemeManager.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/IPanel.h>
#include <vgui/ILocalize.h>
#include <vgui/IKeyValues.h>
#include <tier1/KeyValues.h>
#include <FileSystem.h>
#include "../BaseUISurface.h"
#include "controls.h"
#include <tier0/dbg.h>

IKeyValues* g_pKeyValuesInterface = nullptr;
IFileSystem *g_pFullFileSystem = nullptr;
extern "C" vgui2::IPanel *g_pPanelInterface = nullptr;

extern vgui2::IVGui *VGuiInterfaceSingleton();
extern vgui2::IPanel *VPanelInterfaceSingleton();
extern BaseUISurface *BaseUISurfaceSingleton();
extern vgui2::ISchemeManager *SchemeInterfaceSingleton();
extern vgui2::ISystem *SystemInterfaceSingleton();
extern vgui2::IInputInternal *InputInternalSingleton();
extern vgui2::ILocalize *LocalizeInterfaceSingleton();
extern IKeyValues *VGuiKeyValuesSingleton();
extern IFileSystem *filesystem(void);

IKeyValues* keyvalues()
{
	return g_pKeyValuesInterface;
}

// Fallback for wasm/client builds that do not link the engine dbg module.
// A strong definition, if present elsewhere, will override this weak symbol.
void COM_TimestampedLog(char const *fmt, ...) __attribute__((weak));
void COM_TimestampedLog(char const *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::fprintf(stderr, "[VGUI2-TRACE] ");
	std::vfprintf(stderr, fmt, args);
	std::fprintf(stderr, "\n");
	va_end(args);
}

// Fallback for wasm/client builds that do not link the engine dbg module.
// Strong definitions elsewhere will override this weak symbol.
void Warning( const tchar *pMsgFormat, ... ) __attribute__((weak));
void Warning( const tchar *pMsgFormat, ... )
{
	va_list args;
	va_start(args, pMsgFormat);
	std::fprintf(stderr, "[Xash3D][warning] ");
	std::vfprintf(stderr, pMsgFormat, args);
	va_end(args);
}

namespace vgui2
{
	vgui2::IInputInternal *g_pInputInterface = NULL;
	vgui2::ISchemeManager *g_pSchemeInterface = NULL;
	vgui2::ISurface *g_pSurfaceInterface = NULL;
	vgui2::ISystem *g_pSystemInterface = NULL;
	vgui2::IVGui *g_pVGuiInterface = NULL;
	vgui2::ILocalize *g_pLocalizeInterface = NULL;

	vgui2::IInputInternal *input() {
		return g_pInputInterface;
	}

	vgui2::ISchemeManager *scheme() {
		return g_pSchemeInterface;
	}

	vgui2::ISurface *surface() {
		std::fprintf(stderr, "[VGUI2-TRACE] surface() &g_pSurfaceInterface=%p value=%p\n",
			(void *)&g_pSurfaceInterface,
			(void *)g_pSurfaceInterface);

		return g_pSurfaceInterface;
	}

	vgui2::ISystem *system() {
		return g_pSystemInterface;
	}
	
	vgui2::IVGui *ivgui() {
		std::fprintf(stderr, "[VGUI2-TRACE] ivgui() &g_pVGuiInterface=%p value=%p\n",
			(void *)&g_pVGuiInterface,
			(void *)g_pVGuiInterface);
		return g_pVGuiInterface;
	}

	vgui2::IPanel *ipanel() {
		// std::fprintf(stderr, "[VGUI2-TRACE] ipanel() &g_pPanelInterface=%p value=%p\n",
		// 	(void *)&::g_pPanelInterface,
		// 	(void *)::g_pPanelInterface);
		return ::g_pPanelInterface;
	}

	vgui2::ILocalize *localize() {
		return g_pLocalizeInterface;
	}
	
	IFileSystem *filesystem() {
		std::fprintf(stderr, "[VGUI2-TRACE] filesystem() &g_pFullFileSystem=%p value=%p\n",
			(void *)&g_pFullFileSystem,
			(void *)g_pFullFileSystem);
		return g_pFullFileSystem;
	}

	static void *InitializeInterface(char const *interfaceName, CreateInterfaceFn *factoryList, int numFactories) {
		void *retval;

		for (int i = 0; i < numFactories; i++) {
			CreateInterfaceFn factory = factoryList[i];
			if (!factory)
				continue;

			retval = factory(interfaceName, NULL);
			if (retval)
				return retval;
		}

		return NULL;
	}

	static char g_szControlsModuleName[256] = "root";

	bool VGuiControls_Init(const char *moduleName, CreateInterfaceFn *factoryList, int numFactories) {
#ifndef XASH_STATIC_GAMELIB
		strncpy(g_szControlsModuleName, moduleName, sizeof(g_szControlsModuleName));
		g_szControlsModuleName[sizeof(g_szControlsModuleName) - 1] = 0;
#endif

		setlocale(LC_CTYPE, "");
		setlocale(LC_TIME, "");
		setlocale(LC_COLLATE, "");
		setlocale(LC_MONETARY, "");

		g_pVGuiInterface = (IVGui *)InitializeInterface(VGUI_IVGUI_INTERFACE_VERSION, factoryList, numFactories);
		::g_pPanelInterface = (IPanel *)InitializeInterface(VGUI_PANEL_INTERFACE_VERSION, factoryList, numFactories);
		g_pSurfaceInterface = (ISurface *)InitializeInterface(VGUI_SURFACE_INTERFACE_VERSION, factoryList, numFactories);
		g_pSchemeInterface = (ISchemeManager *)InitializeInterface(VGUI_SCHEME_INTERFACE_VERSION, factoryList, numFactories);
		g_pSystemInterface = (ISystem *)InitializeInterface(VGUI_SYSTEM_INTERFACE_VERSION, factoryList, numFactories);
		g_pInputInterface = (IInputInternal *)InitializeInterface(VGUI_INPUTINTERNAL_INTERFACE_VERSION, factoryList, numFactories);
		g_pLocalizeInterface = (ILocalize *)InitializeInterface(VGUI_LOCALIZE_INTERFACE_VERSION, factoryList, numFactories);
        g_pFullFileSystem = (IFileSystem *)InitializeInterface(FILESYSTEM_INTERFACE_VERSION, factoryList, numFactories);

		g_pKeyValuesInterface = static_cast<IKeyValues*>(InitializeInterface(KEYVALUES_INTERFACE_VERSION, factoryList, numFactories));

		std::fprintf(stderr,
			"[VGUI2-TRACE] VGuiControls_Init after factory lookup module=%s ivgui=%p panel=%p surface=%p scheme=%p system=%p input=%p localize=%p filesystem=%p keyvalues=%p\n",
			moduleName,
			(void *)g_pVGuiInterface,
			(void *)::g_pPanelInterface,
			(void *)g_pSurfaceInterface,
			(void *)g_pSchemeInterface,
			(void *)g_pSystemInterface,
			(void *)g_pInputInterface,
			(void *)g_pLocalizeInterface,
			(void *)g_pFullFileSystem,
			(void *)g_pKeyValuesInterface);

#ifdef XASH_STATIC_GAMELIB
		if (!g_pVGuiInterface)
			g_pVGuiInterface = VGuiInterfaceSingleton();
		if (!::g_pPanelInterface)
			::g_pPanelInterface = VPanelInterfaceSingleton();
		if (!g_pSurfaceInterface)
			g_pSurfaceInterface = static_cast<ISurface *>(BaseUISurfaceSingleton());
		if (!g_pSchemeInterface)
			g_pSchemeInterface = SchemeInterfaceSingleton();
		if (!g_pSystemInterface)
			g_pSystemInterface = SystemInterfaceSingleton();
		if (!g_pInputInterface)
			g_pInputInterface = InputInternalSingleton();
		if (!g_pLocalizeInterface)
			g_pLocalizeInterface = LocalizeInterfaceSingleton();
		if (!g_pFullFileSystem)
			g_pFullFileSystem = ::filesystem();
		if (!g_pKeyValuesInterface)
			g_pKeyValuesInterface = VGuiKeyValuesSingleton();
#endif

		// Keep the panel interface available even when the factory list does not
		// expose it yet. Panel construction depends on ipanel() during bootstrap.
		if (!::g_pPanelInterface)
			::g_pPanelInterface = VPanelInterfaceSingleton();

		std::fprintf(stderr,
			"[VGUI2-TRACE] VGuiControls_Init after fallbacks module=%s ivgui=%p panel=%p surface=%p scheme=%p system=%p input=%p localize=%p filesystem=%p keyvalues=%p\n",
			moduleName,
			(void *)g_pVGuiInterface,
			(void *)::g_pPanelInterface,
			(void *)g_pSurfaceInterface,
			(void *)g_pSchemeInterface,
			(void *)g_pSystemInterface,
			(void *)g_pInputInterface,
			(void *)g_pLocalizeInterface,
			(void *)g_pFullFileSystem,
			(void *)g_pKeyValuesInterface);
		
		if (!g_pVGuiInterface) {
			return false;
		}

		g_pVGuiInterface->Init(factoryList, numFactories);

		if (g_pKeyValuesInterface)
			g_pKeyValuesInterface->RegisterSizeofKeyValues(sizeof(KeyValues));

		if (g_pSchemeInterface &&
			g_pSurfaceInterface &&
			g_pSystemInterface &&
			g_pInputInterface &&
			g_pVGuiInterface &&
			g_pFullFileSystem &&
			g_pLocalizeInterface &&
			::g_pPanelInterface)
			return true;

		return false;
	}

	bool VGui_InitInterfacesList(const char *moduleName, CreateInterfaceFn *factoryList, int numFactories) {
		return VGuiControls_Init(moduleName, factoryList, numFactories);
	}

	const char *GetControlsModuleName() {
		return g_szControlsModuleName;
	}
}
