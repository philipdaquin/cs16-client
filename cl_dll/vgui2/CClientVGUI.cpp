#include <cassert>
#include <clocale>


#include <vgui/VGUI.h>
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>

#include <vgui/ISurface.h>
#include <IEngineVGui.h>
#include "CHudViewPort.h"

#include <vgui_controls/Controls.h>

#include "IGameUIFuncs.h"
#include "IBaseUI.h"
#include "hud.h"

#include "CClientVGUI.h"

namespace vgui2
{
bool VGuiControls_Init( const char *moduleName, CreateInterfaceFn *factoryList, int numFactories );
}

namespace
{
CClientVGUI g_ClientVGUI;

IGameUIFuncs* g_GameUIFuncs = nullptr;

IEngineVGui *g_EngineVgui = nullptr;
}

IBaseUI *g_pBaseUI = nullptr;
IEngineVGui *g_pEngineVGui = nullptr;

CClientVGUI* clientVGUI()
{
	return &g_ClientVGUI;
}

IGameUIFuncs* gameUIFuncs()
{
	return g_GameUIFuncs;
}

IBaseUI* baseUI()
{
	return ::g_pBaseUI;
}

IEngineVGui* engineVgui()
{
	return g_EngineVgui;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CClientVGUI, IClientVGUI, CLIENTVGUI_INTERFACE_VERSION, g_ClientVGUI );

CClientVGUI::CClientVGUI()
{
	if (gEngfuncs.Con_Printf)
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI ctor this=%p\n", this);
}

void CClientVGUI::Initialize( CreateInterfaceFn* pFactories, int iNumFactories )
{
	/*
	*	Factories in the given array:
	*	engine
	*	vgui2
	*	filesystem
	*	chrome HTML
	*	GameUI
	*	client (this library)
	*/

	//4 factories to use.
	assert( static_cast<size_t>( iNumFactories ) >= NUM_FACTORIES - 1 );
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Initialize this=%p factories=%p count=%d\n", this, (void *)pFactories, iNumFactories);

	m_FactoryList[ 0 ] = Sys_GetFactoryThis();

	for( size_t uiIndex = 0; uiIndex < NUM_FACTORIES - 1; ++uiIndex )
	{
		m_FactoryList[ uiIndex + 1 ] = pFactories[ uiIndex ];
	}

	if( !vgui2::VGuiControls_Init( "CLIENT", m_FactoryList, NUM_FACTORIES ) )
	{
		Msg( "Failed to initialize VGUI2\n" );
		return;
	}
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Initialize controls ok baseui=%p enginevgui=%p\n", (void *)::g_pBaseUI, (void *)g_EngineVgui);

	g_GameUIFuncs = ( IGameUIFuncs* ) pFactories[ 0 ](ENGINE_GAMEUIFUNCS_INTERFACE_VERSION, nullptr );
	::g_pBaseUI = ( IBaseUI* ) pFactories[ 0 ](BASEUI_INTERFACE_VERSION, nullptr );
	g_EngineVgui = (IEngineVGui* ) pFactories[ 0 ](VENGINE_VGUI_VERSION, nullptr );
	::g_pEngineVGui = g_EngineVgui;

	//Constructor sets itself as the viewport.

	new CHudViewport();
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Initialize viewport=%p after new CHudViewport\n", (void *)g_pViewport);

	g_pViewport->Initialize( pFactories, iNumFactories );
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Initialize viewport->Initialize done viewport=%p\n", (void *)g_pViewport);
}

void CClientVGUI::Start()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Start viewport=%p\n", (void *)g_pViewport);
	g_pViewport->Start();
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Start done viewport=%p\n", (void *)g_pViewport);

#if 0
	vgui2::Frame* pFrame = new vgui2::Frame(nullptr, "TestFrame");
	//pFrame->SetParent(engineVgui()->GetPanel(PANEL_GAMEUIDLL));
	pFrame->SetProportional(false);
	pFrame->SetTitleBarVisible(true);
	pFrame->SetMinimizeButtonVisible(false);
	pFrame->SetMaximizeButtonVisible(false);
	pFrame->SetCloseButtonVisible(true);
	pFrame->SetSizeable(true);
	pFrame->SetMoveable(true);
	pFrame->SetVisible(true);
	//pFrame->SetScheme(vgui2::scheme()->LoadSchemeFromFile("ui/resource/SourceScheme.res", "SourceScheme"));
	pFrame->SetScheme(vgui2::scheme()->LoadSchemeFromFile("Resource/TrackerScheme.res", "BaseUI"));
	pFrame->SetSize(500, 500);
	pFrame->SetTitle("Test Frame", true);
	pFrame->Activate();
#endif

}

void CClientVGUI::Init()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Init viewport=%p\n", (void *)g_pViewport);
	g_pViewport->Init();
}

void CClientVGUI::VidInit()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::VidInit viewport=%p\n", (void *)g_pViewport);
	g_pViewport->VidInit();
}

void CClientVGUI::SetParent( vgui2::VPANEL parent )
{
	m_vRootPanel = parent;
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::SetParent this=%p parent=%p viewport=%p\n", this, (void *)parent, (void *)g_pViewport);

	g_pViewport->SetParent( parent );
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::SetParent done viewport=%p root=%p\n", (void *)g_pViewport, (void *)m_vRootPanel);
}

bool CClientVGUI::UseVGUI1()
{
	return g_pViewport->UseVGUI1();
}

void CClientVGUI::HideScoreBoard()
{
	g_pViewport->HideScoreBoard();
}

void CClientVGUI::HideAllVGUIMenu()
{
	g_pViewport->HideAllVGUIMenu();
}

void CClientVGUI::ActivateClientUI()
{
	g_pViewport->ActivateClientUI();
}

void CClientVGUI::HideClientUI()
{
	g_pViewport->HideClientUI();
}

void CClientVGUI::Shutdown()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClientVGUI::Shutdown viewport=%p\n", (void *)g_pViewport);
	g_pViewport->Shutdown();
}
