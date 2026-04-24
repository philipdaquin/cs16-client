#include <string.h>
#include <locale.h>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include "../interfaces/vgui/IInputInternal.h"
#include "../interfaces/vgui/ISchemeManager.h"
#include "../interfaces/vgui/ISurface.h"
#include "../interfaces/vgui/ISystem.h"
#include "../interfaces/vgui/IVGui.h"
#include "../interfaces/vgui/IPanel.h"
#include "../interfaces/vgui/ILocalize.h"
#include "../interfaces/vgui/IKeyValues.h"
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

static void TraceFilesystemState(const char *reason, IFileSystem *fs)
{
	std::fprintf(stderr,
		"[phase3][VGUI2-TRACE] %s filesystem=%p\n",
		reason ? reason : "<null>",
		(void *)fs);
}

// Fallback for wasm/client builds that do not link the engine dbg module.
// A strong definition, if present elsewhere, will override this weak symbol.
void COM_TimestampedLog(char const *fmt, ...) __attribute__((weak));
void COM_TimestampedLog(char const *fmt, ...)
{
	//va_list args;
	//va_start(args, fmt);
	//std::fprintf(stderr, "[VGUI2-TRACE] ");
	//std::vfprintf(stderr, fmt, args);
	//std::fprintf(stderr, "\n");
	//va_end(args);
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
		//std::fprintf(stderr, "[VGUI2-TRACE] surface() &g_pSurfaceInterface=%p value=%p\n",
		//	(void *)&g_pSurfaceInterface,
		//	(void *)g_pSurfaceInterface);

		return g_pSurfaceInterface;
	}

	vgui2::ISystem *system() {
		return g_pSystemInterface;
	}
	
	vgui2::IVGui *ivgui() {
		//std::fprintf(stderr, "[VGUI2-TRACE] ivgui() &g_pVGuiInterface=%p value=%p\n",
		//	(void *)&g_pVGuiInterface,
		//	(void *)g_pVGuiInterface);
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
		TraceFilesystemState("vgui2::filesystem()", g_pFullFileSystem);
		return g_pFullFileSystem;
	}

	namespace
	{
		bool g_FileSystemSearchPathsBootstrapped = false;
		std::string g_LastBootstrapCwd;
		std::string g_LastBootstrapGameDir;

		static bool DirectoryExists( IFileSystem *fs, const std::string &path )
		{
			return fs && !path.empty() && fs->IsDirectory( path.c_str() );
		}

		static std::string JoinPath( const std::string &lhs, const char *rhs )
		{
			if( lhs.empty() )
				return rhs ? rhs : "";

			std::string result = lhs;
			if( result.back() != '/' && result.back() != '\\' )
				result.push_back( '/' );
			if( rhs )
				result.append( rhs );
			return result;
		}

		static bool EndsWithPathComponent( const std::string &path, const char *component )
		{
			if( !component || !*component )
				return false;

			const size_t componentLen = strlen( component );
			if( path.size() < componentLen )
				return false;

			const size_t start = path.size() - componentLen;
			if( path.compare( start, componentLen, component ) != 0 )
				return false;

			return start == 0 || path[start - 1] == '/' || path[start - 1] == '\\';
		}

		static void PushUniqueRoot( std::vector<std::string> &roots, const std::string &path )
		{
			if( path.empty() )
				return;

			for( const auto &existing : roots )
			{
				if( existing == path )
					return;
			}

			roots.push_back( path );
		}
	}

	void BootstrapFileSystemSearchPaths( const char *gameDir )
	{
		IFileSystem *fs = filesystem();
		if( !fs )
			return;

		char cwd[1024];
		if( !fs->GetCurrentDirectory( cwd, sizeof( cwd ) ) )
			return;

		const std::string cwdString = cwd;
		const std::string requestedGameDir = gameDir ? gameDir : "";

		if( g_FileSystemSearchPathsBootstrapped &&
			g_LastBootstrapCwd == cwdString &&
			g_LastBootstrapGameDir == requestedGameDir )
		{
			return;
		}

		std::vector<std::string> roots;
		if( !requestedGameDir.empty() )
		{
			if( EndsWithPathComponent( cwdString, requestedGameDir.c_str() ) )
				PushUniqueRoot( roots, cwdString );
			else
				PushUniqueRoot( roots, JoinPath( cwdString, requestedGameDir.c_str() ) );
		}

		PushUniqueRoot( roots, JoinPath( cwdString, "cstrike" ) );
		PushUniqueRoot( roots, JoinPath( cwdString, "valve" ) );
		PushUniqueRoot( roots, cwdString );

		fs->RemoveAllSearchPaths();

		for( const auto &root : roots )
		{
			if( DirectoryExists( fs, root ) )
			{
				std::fprintf(
					stderr,
					"[phase3][VGUI2-TRACE] BootstrapFileSystemSearchPaths add root='%s' gameDir='%s'\n",
					root.c_str(),
					requestedGameDir.empty() ? "<null>" : requestedGameDir.c_str()
				);
				fs->AddSearchPath( root.c_str(), "GAME" );
			}
			else
			{
				std::fprintf(
					stderr,
					"[phase3][VGUI2-TRACE] BootstrapFileSystemSearchPaths skip missing root='%s' gameDir='%s'\n",
					root.c_str(),
					requestedGameDir.empty() ? "<null>" : requestedGameDir.c_str()
				);
			}
		}

		g_FileSystemSearchPathsBootstrapped = true;
		g_LastBootstrapCwd = cwdString;
		g_LastBootstrapGameDir = requestedGameDir;
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
		TraceFilesystemState("VGuiControls_Init after factory lookup", g_pFullFileSystem);

		g_pKeyValuesInterface = static_cast<IKeyValues*>(InitializeInterface(KEYVALUES_INTERFACE_VERSION, factoryList, numFactories));

		//std::fprintf(stderr,
		//	"[VGUI2-TRACE] VGuiControls_Init after factory lookup module=%s ivgui=%p panel=%p surface=%p scheme=%p system=%p input=%p localize=%p filesystem=%p keyvalues=%p\n",
		//	moduleName,
		//	(void *)g_pVGuiInterface,
		//	(void *)::g_pPanelInterface,
		//	(void *)g_pSurfaceInterface,
		//	(void *)g_pSchemeInterface,
		//	(void *)g_pSystemInterface,
		//	(void *)g_pInputInterface,
		//	(void *)g_pLocalizeInterface,
		//	(void *)g_pFullFileSystem,
		//	(void *)g_pKeyValuesInterface);

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
		TraceFilesystemState("VGuiControls_Init after fallback", g_pFullFileSystem);
		if (!g_pKeyValuesInterface)
			g_pKeyValuesInterface = VGuiKeyValuesSingleton();
#endif

		// Keep the panel interface available even when the factory list does not
		// expose it yet. Panel construction depends on ipanel() during bootstrap.
		if (!::g_pPanelInterface)
			::g_pPanelInterface = VPanelInterfaceSingleton();

		//std::fprintf(stderr,
		//	"[VGUI2-TRACE] VGuiControls_Init after fallbacks module=%s ivgui=%p panel=%p surface=%p scheme=%p system=%p input=%p localize=%p filesystem=%p keyvalues=%p\n",
		//	moduleName,
		//	(void *)g_pVGuiInterface,
		//	(void *)::g_pPanelInterface,
		//	(void *)g_pSurfaceInterface,
		//	(void *)g_pSchemeInterface,
		//	(void *)g_pSystemInterface,
		//	(void *)g_pInputInterface,
		//	(void *)g_pLocalizeInterface,
		//	(void *)g_pFullFileSystem,
		//	(void *)g_pKeyValuesInterface);
		
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
