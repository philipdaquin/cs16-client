#include <cstdio>

#include <vector>
#include <string.h>

#include "vgui_api.h"
#include "wrect.h"
typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);
#ifndef _format
#define _format(x)
#endif
#include "cdll_int.h"
#include "menu_int.h"

#include "CBaseUI.h"
#include <FileSystem.h>
#include "interfaces/vgui/IVGui.h"
#include "interfaces/vgui/IPanel.h"
#include "interfaces/vgui/ISurface.h"
#include "interfaces/vgui/ILocalize.h"
#include "interfaces/vgui/IScheme.h"
#include "interfaces/vgui/ISystem.h"
#include "interfaces/IClientVGUI.h"
#include "interfaces/vgui/IInputInternal.h"
#include "vgui_controls/controls.h"
#include "vgui_controls/Panel.h"
#include "BaseUISurface.h"
#include "../cl_dll/vgui2/CBaseViewport.h"
#include "keydefs.h"
#include "keydest.h"

#include "GameUI/IGameUI.h"
#include "GameUI/IGameConsole.h"
#include "GameUI/ICareerUI.h"
#include "GameUI/GameUI_Interface.h"
#include "GameUI/GameConsole.h"

namespace ui
{
	extern ui_enginefuncs_t engfuncs;
}

class CStaticPanel;
class CEnginePanel;

extern "C" vgui2::ISurface *g_pVGuiSurface;
extern "C" vgui2::IInputInternal *g_pVGuiInput;
extern "C" vgui2::IPanel *g_pVGuiPanel;
extern "C" vgui2::IVGui *g_pVGui;
extern "C" vgui2::ISystem *g_pVGuiSystem;
extern "C" vgui2::IPanel *g_pPanelInterface;

namespace vgui2 {
extern cl_enginefunc_t gEngfuncs;
IClientVGUI *staticClient;
IGameUI* staticGameUIFuncs;
IGameConsole* staticGameConsole;
ICareerUI* staticCareerUI;
vgui2::IInputInternal *g_pInputInternal;
extern vgui2::ISystem *g_pSystemInterface;

CStaticPanel *staticPanel;
CEnginePanel *staticClientDLLPanel;
CEnginePanel *staticGameUIPanel;
}
BaseUISurface *staticSurface;
using namespace vgui2;

extern vgui2::IInputInternal *InputInternalSingleton();
extern IClientVGUI* clientVGUIInterface();

CBaseUI g_BaseUI;

class CEnginePanel : public vgui2::Panel {
public:
	CEnginePanel() : Panel() {
		m_bCanFocus = true;
	}

	CEnginePanel(Panel* parent, const char* panelName) : Panel(parent, panelName)
	{
		m_bCanFocus = true;
	}

	vgui2::VPANEL IsWithinTraverse(int x, int y, bool traversePopups) {
		if (!m_bCanFocus) {
			return 0;
		}

		vgui2::VPANEL result = vgui2::Panel::IsWithinTraverse(x, y, traversePopups);

		if (result == GetVPanel()) {
			return 0;
		}

		return result;
	}

private:
	bool m_bCanFocus;
};

class CStaticPanel : public vgui2::Panel {
public:
	vgui2::VPANEL GetCurrentKeyFocus() {
		return staticClientDLLPanel->GetCurrentKeyFocus();
	}

	vgui2::VPANEL IsWithinTraverse(int x, int y, bool traversePopups) {
		vgui2::VPANEL result = vgui2::Panel::IsWithinTraverse(x, y, traversePopups);

		if (result == GetVPanel()) {
			return 0;
		}

		return result;
	}
};

void CBaseUI::Initialize(CreateInterfaceFn* factories, int count) {
	std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseUI::Initialize entry this=%p factories=%p count=%d staticClient=%p\n",
		this, (void *)factories, count, (void *)staticClient);
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize this=%p factories=%p count=%d staticClient=%p\n",
		this, (void *)factories, count, (void *)staticClient);
	if (staticClient) {
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize early-return staticClient=%p\n", (void *)staticClient);
		return;
	}

#ifndef XASH_STATIC_GAMELIB
	m_hFileSystemModule = Sys_LoadModule("FileSystem_stdio.dll");
	m_hVGuiModule = Sys_LoadModule("vgui2.dll");
	m_hChromeModule = Sys_LoadModule("chromehtml.dll");
	m_FactoryList[0] = factories[0];
	m_FactoryList[1] = Sys_GetFactory(m_hVGuiModule);
	m_FactoryList[2] = Sys_GetFactory(m_hFileSystemModule);
	m_FactoryList[3] = Sys_GetFactory(m_hChromeModule);
	m_iNumFactories = 4;
#else
    m_FactoryList[0] = factories[0];
    m_FactoryList[1] = Sys_GetFactoryThis();
    m_FactoryList[2] = Sys_GetFactoryThis();
    m_FactoryList[3] = Sys_GetFactoryThis();
    m_iNumFactories = 4;
#endif
	
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize before VGuiControls_Init factories=%p count=%d\n",
		(void *)m_FactoryList[0], m_iNumFactories);
	if (!vgui2::VGuiControls_Init("BaseUI", m_FactoryList, m_iNumFactories))
	{
		std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseUI::Initialize failed VGuiControls_Init this=%p\n", this);
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize failed: VGuiControls_Init returned false\n");
		return;
	}
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize after VGuiControls_Init staticSurface=%p staticClient=%p\n",
		(void *)staticSurface, (void *)staticClient);

#ifdef XASH_STATIC_GAMELIB
    auto gameUIFactory = Sys_GetFactoryThis();
    m_FactoryList[m_iNumFactories] = gameUIFactory;
#else
	if(!(m_hStaticGameUIModule = Sys_LoadModule("GameUI.dll")))
	{
		char szGameUIDLLPath[_MAX_PATH];
		vgui2::filesystem()->GetLocalPath("cl_dlls/GameUI.dll", szGameUIDLLPath, sizeof(szGameUIDLLPath));
		m_hStaticGameUIModule = Sys_LoadModule(szGameUIDLLPath);
	}
	auto gameUIFactory = Sys_GetFactory(m_hStaticGameUIModule);
	m_FactoryList[m_iNumFactories] = gameUIFactory;
#endif

	if (gameUIFactory)
	{
#ifdef XASH_STATIC_GAMELIB
		staticGameUIFuncs = &GameUI();
		staticGameConsole = &GameConsole();
		staticCareerUI = nullptr;
#else
		staticGameUIFuncs = static_cast<IGameUI*>(gameUIFactory(GAMEUI_INTERFACE_VERSION, nullptr));
		staticGameConsole = static_cast<IGameConsole*>(gameUIFactory(GAMECONSOLE_INTERFACE_VERSION, nullptr));
		staticCareerUI = static_cast<ICareerUI*>(gameUIFactory(CAREERUI_INTERFACE_VERSION, nullptr));
#endif

		++m_iNumFactories;
	}
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize gameui=%p console=%p career=%p\n",
		(void *)staticGameUIFuncs, (void *)staticGameConsole, (void *)staticCareerUI);
	
#ifdef XASH_STATIC_GAMELIB
	g_pInputInternal = InputInternalSingleton();
#else
	g_pInputInternal = (vgui2::IInputInternal *)m_FactoryList[1](VGUI_INPUTINTERNAL_INTERFACE_VERSION, NULL);
#endif
	g_pVGuiInput = g_pInputInternal;
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize inputInternal=%p vguiInput=%p\n",
		(void *)g_pInputInternal, (void *)g_pVGuiInput);

	//char szClientDLLPath[_MAX_PATH];
	//g_pFullFileSystem->GetLocalPath("cl_dlls/client.dll", szClientDLLPath, sizeof(szClientDLLPath));
	//m_hClientModule = Sys_LoadModule(szClientDLLPath);
	m_FactoryList[4] = Sys_GetFactoryThis(); // Sys_GetFactory(m_hClientModule);
	m_iNumFactories = 5;
#ifdef XASH_STATIC_GAMELIB
	staticClient = clientVGUIInterface();
#else
	staticClient = (IClientVGUI *)m_FactoryList[4](CLIENTVGUI_INTERFACE_VERSION, NULL);
#endif
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Initialize staticClient=%p numFactories=%d\n",
		(void *)staticClient, m_iNumFactories);
	//gClDllFuncs.pfnInitialize(&gEngfuncs, CLDLL_INTERFACE_VERSION);
}

	void CBaseUI::Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion) {
		std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseUI::Start entry this=%p engineFuncs=%p interfaceVersion=%d staticClient=%p surface=%p viewport=%p\n",
			this, (void *)engineFuncs, interfaceVersion, (void *)staticClient, (void *)staticSurface, (void *)g_pViewport);
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start this=%p engineFuncs=%p interfaceVersion=%d staticClient=%p surface=%p viewport=%p\n",
			this, (void *)engineFuncs, interfaceVersion, (void *)staticClient, (void *)staticSurface, (void *)g_pViewport);
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start pre-panel ivgui=%p ipanel=%p surface=%p\n",
			(void *)vgui2::ivgui(), (void *)vgui2::ipanel(), (void *)vgui2::surface());
	#ifdef XASH_STATIC_GAMELIB
		staticSurface = BaseUISurfaceSingleton();
	#else
		staticSurface = (BaseUISurface *)m_FactoryList[0](VGUI_SURFACE_INTERFACE_VERSION, NULL);
	#endif
		g_pVGui = vgui2::ivgui();
		g_pVGuiSystem = vgui2::system();
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start surface=%p ivgui=%p system=%p\n",
			(void *)staticSurface, (void *)g_pVGui, (void *)g_pVGuiSystem);
		IHTMLChromeController *chromeController = nullptr;
		if(m_FactoryList[3])
			chromeController = (IHTMLChromeController *)m_FactoryList[3](HTML_CHROME_CONTROLLER_INTERFACE_VERSION, NULL);
		vgui2::ivgui()->Start();
		vgui2::ivgui()->SetSleep(false);
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start ivgui started staticSurface=%p viewport=%p\n",
			(void *)staticSurface, (void *)g_pViewport);

		staticPanel = new CStaticPanel();
		VPANEL staticPanelVPanel = staticPanel ? staticPanel->GetVPanel() : 0;
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start staticPanel=%p vpanel=%p\n", (void *)staticPanel, (void *)(uintptr_t)staticPanelVPanel);
		g_pVGuiPanel = g_pPanelInterface;
		staticPanel->SetCursor(vgui2::dc_none);

		{
			const auto color = staticPanel->GetBgColor();

			//Set alpha to maximum.
			staticPanel->SetBgColor(Color(color.r(), color.g(), color.b(), 0xFF));
		}

		//staticPanel->SetBounds(0, 0, 40, 30);

		staticPanel->SetPaintBorderEnabled(false);
		staticPanel->SetPaintBackgroundEnabled(false);
		staticPanel->SetPaintEnabled(false);

		staticPanel->SetCursor(vgui2::dc_none);
		staticPanel->SetVisible(true);
		staticPanel->SetZPos(0);

		staticSurface->Init(staticPanel->GetVPanel(), chromeController);
		g_pVGuiSurface = staticSurface;
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start surface init embedded=%p\n",
			(void *)staticSurface->GetEmbeddedPanel());
		staticSurface->SetLanguage("schinese");
		staticSurface->IgnoreMouseVisibility(true);

		const char *szGameDir = gEngfuncs.pfnGetGameDirectory();
		vgui2::BootstrapFileSystemSearchPaths( szGameDir );

			vgui2::scheme()->LoadSchemeFromFile("resource/TrackerScheme.res", "BaseUI");
			vgui2::localize()->AddFile(vgui2::filesystem(), "resource/tracker_%language%.txt");
			vgui2::localize()->AddFile(vgui2::filesystem(), "resource/valve_%language%.txt");

		if (strcmp(szGameDir, "valve")) {
			char szModLocalizeFile[_MAX_PATH];
			snprintf(szModLocalizeFile, _MAX_PATH, "resource/%s_%%language%%.txt", szGameDir);
		}

		staticClientDLLPanel = new CEnginePanel(staticPanel, "BaseClientPanel");

	{
		Color color = staticClientDLLPanel->GetBgColor();

		//Set alpha to maximum.
		staticClientDLLPanel->SetBgColor(Color(color.r(), color.g(), color.b(), 0xFF));
	}

	staticClientDLLPanel->SetPaintBorderEnabled(false);
	staticClientDLLPanel->SetPaintBackgroundEnabled(false);
	staticClientDLLPanel->SetPaintEnabled(false);
	
	staticClientDLLPanel->SetVisible(true);
	staticClientDLLPanel->SetCursor(vgui2::dc_none);

	//Draw above static panel.
	staticClientDLLPanel->SetZPos(25);

	staticGameUIPanel = new CEnginePanel(staticPanel, "BaseGameUIPanel");

	{
		Color color = staticGameUIPanel->GetBgColor();

		//Set alpha to maximum.
		staticGameUIPanel->SetBgColor(Color(color.r(), color.g(), color.b(), 0xFF));
	}

	staticGameUIPanel->SetPaintBorderEnabled(false);
	staticGameUIPanel->SetPaintBackgroundEnabled(false);
	staticGameUIPanel->SetPaintEnabled(false);

	staticGameUIPanel->SetVisible(true);
	staticGameUIPanel->SetCursor(vgui2::dc_none);

	//Draw above static and client panels.
	staticGameUIPanel->SetZPos(50);
	
		if (staticGameUIFuncs)
		{
			gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start before GameUI Initialize staticGameUIFuncs=%p factories=%d\n",
				(void *)staticGameUIFuncs, m_iNumFactories);
			staticGameUIFuncs->Initialize(m_FactoryList, m_iNumFactories);
			gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start after GameUI Initialize staticGameUIFuncs=%p staticClient=%p\n",
				(void *)staticGameUIFuncs, (void *)staticClient);
		}
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start after GameUI init staticGameUIFuncs=%p staticClient=%p\n",
		(void *)staticGameUIFuncs, (void *)staticClient);

	if (staticClient) {
		staticClient->Initialize(m_FactoryList, m_iNumFactories);
		staticSurface->SetVGUI2MouseControl(true);
	}
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start after client Initialize staticSurface=%p viewport=%p\n",
		(void *)staticSurface, (void *)g_pViewport);

		if (staticGameUIFuncs)
		{
			void* system = nullptr;
			gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start before GameUI Start staticGameUIFuncs=%p system=%p\n",
				(void *)staticGameUIFuncs, system);
			staticGameUIFuncs->Start(&gEngfuncs, CLDLL_INTERFACE_VERSION, system);
			gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start after GameUI Start staticGameUIFuncs=%p\n",
				(void *)staticGameUIFuncs);
		}

	staticClientDLLPanel->SetScheme("ClientScheme");

	if (staticClient) {
		std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseUI::Start before staticClient calls staticClient=%p root=%p viewport=%p\n",
			(void *)staticClient, (void *)(uintptr_t)(staticClientDLLPanel ? staticClientDLLPanel->GetVPanel() : 0), (void *)g_pViewport);
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start pre-client calls staticClient=%p root=%p viewport=%p\n",
			(void *)staticClient, (void *)(uintptr_t)(staticClientDLLPanel ? staticClientDLLPanel->GetVPanel() : 0), (void *)g_pViewport);
		staticClient->Start();
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start after Start staticClient=%p root=%p viewport=%p\n",
			(void *)staticClient, (void *)(uintptr_t)(staticClientDLLPanel ? staticClientDLLPanel->GetVPanel() : 0), (void *)g_pViewport);
		staticClient->SetParent(staticClientDLLPanel->GetVPanel());
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start after SetParent staticClient=%p root=%p viewport=%p\n",
			(void *)staticClient, (void *)(uintptr_t)(staticClientDLLPanel ? staticClientDLLPanel->GetVPanel() : 0), (void *)g_pViewport);
	}
	VPANEL staticClientRootPanel = staticClientDLLPanel ? staticClientDLLPanel->GetVPanel() : 0;
	std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseUI::Start exit viewport=%p root=%p\n",
		(void *)g_pViewport, (void *)(uintptr_t)staticClientRootPanel);
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseUI::Start after client Start/SetParent viewport=%p root=%p\n",
		(void *)g_pViewport, (void *)(uintptr_t)staticClientRootPanel);

	int wide, tall;
	staticSurface->GetScreenSize(wide, tall);
	
	staticPanel->SetBounds(0, 0, wide, tall);
	staticGameUIPanel->SetBounds(0, 0, wide, tall);
	staticClientDLLPanel->SetBounds(0, 0, wide, tall);
	
	if (staticGameConsole)
	{
		staticGameConsole->Initialize();
		staticGameConsole->SetParent(staticGameUIPanel->GetVPanel());
	}

	staticSurface->IgnoreMouseVisibility(false);

	if (staticGameUIFuncs)
		staticGameUIFuncs->ActivateGameUI();
}

void CBaseUI::Shutdown() {
	//vgui2::ivgui()->RunFrame();
	vgui2::ivgui()->Shutdown();

	if (staticGameUIFuncs)
		staticGameUIFuncs->Shutdown();
	
	if (staticClient) {
		staticClient->Shutdown();
	}

#ifndef XASH_STATIC_GAMELIB
	Sys_UnloadModule(m_hStaticGameUIModule);
	m_hStaticGameUIModule = nullptr;
#endif

	staticGameUIFuncs = nullptr;
	staticGameConsole = nullptr;
	staticCareerUI = nullptr;

	vgui2::system()->SaveUserConfigFile();
#ifndef XASH_STATIC_GAMELIB
	Sys_UnloadModule(m_hClientModule);
	m_hClientModule = NULL;
	staticSurface->Shutdown();
	g_pVGuiSurface = nullptr;
	g_pVGuiInput = nullptr;
	g_pVGuiPanel = nullptr;
	g_pVGui = nullptr;
	g_pVGuiSystem = nullptr;
	Sys_UnloadModule(m_hChromeModule);
	m_hChromeModule = NULL;
	Sys_UnloadModule(m_hVGuiModule);
	m_hVGuiModule = NULL;
	Sys_UnloadModule(m_hFileSystemModule);
	m_hFileSystemModule = NULL;
#endif
}

int CBaseUI::Key_Event(int down, int keynum, const char* pszCurrentBinding) {
	if (keynum == '`' || keynum == '~')
	{
		// toggle console in keys.cpp -> Key_Event Con_ToggleConsole_f
	}
	else
	{
		if (keynum == K_ESCAPE && down)
		{
			const char* pszLevelName = gEngfuncs.pfnGetLevelName();
			if (pszLevelName && *pszLevelName)
			{
			if (staticGameUIFuncs && staticGameUIFuncs->IsGameUIActive())
			{
				HideGameUI();
				return false;
			}
				ActivateGameUI();
				HideConsole();
				return true;
			}
		}
		else if (m_bHidingGameUI && keynum == K_MOUSE1 && down)
		{
			m_bHidingGameUI = false;
			return false;
		}
		else
		{
			return vgui2::surface()->NeedKBInput();
		}
	}

	return false;
}

void CBaseUI::CallEngineSurfaceProc(void* hwnd, unsigned int msg, unsigned int wparam, long lparam) {
}

void CBaseUI::Paint(int x, int y, int right, int bottom) {
	if (!staticSurface || !staticSurface->GetEmbeddedPanel()) {
		return;
	}

	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CBaseUI::Paint entry x=%d y=%d right=%d bottom=%d surface=%p embedded=%p\n",
	// 	x, y, right, bottom, (void *)staticSurface, (void *)staticSurface->GetEmbeddedPanel());
    if (staticGameUIFuncs)
	    staticGameUIFuncs->RunFrame();
	vgui2::ivgui()->RunFrame();
	staticSurface->SetScreenBounds(x, y, right - x, bottom - y);
	vgui2::surface()->DrawSetAlphaMultiplier( 1.0f );
	staticPanel->SetBounds(0, 0, right, bottom);
	staticGameUIPanel->SetBounds(0, 0, right, bottom);
	staticClientDLLPanel->SetBounds(0, 0, right, bottom);
	//staticPanel->PerformApplySchemeSettings();
	//staticPanel->InvalidateLayout(false, true);
	static_cast<vgui2::IClientPanel*>( staticPanel )->Think();

	vgui2::surface()->PaintTraverse(staticSurface->GetEmbeddedPanel());
	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CBaseUI::Paint exit x=%d y=%d right=%d bottom=%d surface=%p embedded=%p\n",
	// 	x, y, right, bottom, (void *)staticSurface, (void *)staticSurface->GetEmbeddedPanel());
}

void CBaseUI::HideGameUI() {
	ui::engfuncs.pfnSetKeyDest(key_game);
	
	if (staticGameUIFuncs)
		staticGameUIFuncs->HideGameUI();
	if (staticGameConsole)
		staticGameConsole->Hide();

	const char* pszLevelName = gEngfuncs.pfnGetLevelName();

	if (pszLevelName && *pszLevelName)
	{
		staticGameUIPanel->SetVisible(false);
		staticGameUIPanel->SetPaintBackgroundEnabled(false);

		staticClientDLLPanel->SetVisible(true);
		staticClientDLLPanel->SetMouseInputEnabled(true);

		if (staticClient)
			staticClient->ActivateClientUI();
	}

	if (vgui2::input()->IsMouseDown(vgui2::MOUSE_LEFT))
		m_bHidingGameUI = true;
}

void CBaseUI::ActivateGameUI() {
	ui::engfuncs.pfnSetKeyDest(key_menu);
	
	if (staticGameUIFuncs)
		staticGameUIFuncs->ActivateGameUI();
	staticGameUIPanel->SetVisible(true);

	staticClientDLLPanel->SetVisible(false);
	staticClientDLLPanel->SetMouseInputEnabled(false);

	if (staticClient)
	{
		staticClient->HideClientUI();
		staticClient->HideScoreBoard();
	}
}

void CBaseUI::HideConsole() {
	if (staticGameConsole)
		staticGameConsole->Hide();
}

void CBaseUI::ShowConsole() {
	if (staticGameConsole)
		staticGameConsole->Activate();
	ui::engfuncs.pfnSetKeyDest(key_console);
}

bool CBaseUI::IsGameUIVisible() {
	return staticGameUIPanel->IsVisible();
}

vgui2::VPANEL CEngineVGui::GetPanel(VGUIPANEL type)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CEngineVGui::GetPanel enter type=%d staticPanel=%p staticClientDLLPanel=%p staticGameUIPanel=%p\n",
		(int)type, (void *)staticPanel, (void *)staticClientDLLPanel, (void *)staticGameUIPanel);
	switch (type)
	{
	default:
	case PANEL_ROOT:		return staticPanel->GetVPanel();
	case PANEL_CLIENTDLL:	return staticClientDLLPanel->GetVPanel();
	case PANEL_GAMEUIDLL:	return staticGameUIPanel->GetVPanel();
	}
	return NULL;
}

bool CEngineVGui::SteamRefreshLogin(const char* password, bool isSecure)
{
	return true;
}

bool CEngineVGui::SteamProcessCall(bool* finished, TSteamProgress* progress, TSteamError* steamError)
{
	return true;
}

void CEngineVGui::SetEngineVisible(bool state)
{
	m_bVisible = state;
#if 0
	if (!g_bIsDedicatedServer)
	{
		if (state)
			ClientDLL_ActivateMouse();
		else
			ClientDLL_DeactivateMouse();
	}
#endif
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CBaseUI, IBaseUI, BASEUI_INTERFACE_VERSION, g_BaseUI );
