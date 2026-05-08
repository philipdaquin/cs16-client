#include <cstdio>

#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>
#include <vgui_controls/Label.h>
#include <tier1/KeyValues.h>

#include "CBackGroundPanel.h"
#include "IViewportPanel.h"
#include "IGameUIPanel.h"

#include "CClientVGUI.h"

#include "CBaseViewport.h"

#include "hud.h"
#include "cl_util.h"
#include "demo_api.h"
#include "csmoe/CSBackGroundPanel.h"
namespace vgui2
{
class Panel;
class Button;
}

CBaseViewport* g_pViewport = nullptr;
vgui2::Panel *g_lastPanel = nullptr;
vgui2::Button *g_lastButton = nullptr;

CBaseViewport::CBaseViewport()
	: BaseClass( nullptr, "CBaseViewport" )
{
	std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseViewport ctor entry this=%p g_pViewport(before)=%p\n", this, (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport ctor this=%p before g_pViewport=%p\n", this, (void *)g_pViewport);
	g_pViewport = this;
	std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseViewport ctor assigned this=%p g_pViewport=%p\n", this, (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport ctor assigned g_pViewport=%p\n", (void *)g_pViewport);

	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );

	// vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");
	vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile("resource/ClientScheme.res", "ClientScheme");

	SetScheme( scheme );
	SetProportional( true );

	m_pAnimController = new vgui2::AnimationController( this );
	// create our animation controller
	m_pAnimController->SetScheme( scheme );
	m_pAnimController->SetProportional( true );
#if 0
	if( !m_pAnimController->SetScriptFile( GetVPanel(), UI_HUDANIMS_FILENAME ) )
	{
		Assert( false );
	}
#endif
}

CBaseViewport::~CBaseViewport()
{
	if( m_pBackGround )
	{
		m_pBackGround->MarkForDeletion();
		m_pBackGround = nullptr;
	}

	RemoveAllPanels();
}

void CBaseViewport::Initialize( CreateInterfaceFn* pFactories, int iNumFactories )
{
	std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseViewport::Initialize entry this=%p factories=%p count=%d viewport=%p\n",
		this, (void *)pFactories, iNumFactories, (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport::Initialize this=%p factories=%p count=%d viewport=%p\n",
		this, (void *)pFactories, iNumFactories, (void *)g_pViewport);
	ReloadScheme();
}

void CBaseViewport::Start()
{
	// recreate all the default panels
	RemoveAllPanels();

	// m_pBackGround = new CBackGroundPanel( nullptr );
	m_pBackGround = new CCSBackGroundPanel( nullptr );

	m_pBackGround->SetZPos( -20 ); // send it to the back 
	m_pBackGround->SetVisible( false );

	CreateDefaultPanels();

	vgui2::ipanel()->MoveToBack( m_pBackGround->GetVPanel() ); // really send it to the back 
}

void CBaseViewport::Init()
{
	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->Init();

	HideAllVGUIMenu();
}

void CBaseViewport::VidInit()
{
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport::VidInit this=%p viewport=%p panels=%d\n",
		this, (void *)g_pViewport, m_Panels.Count());
	for (int i = 0; i < m_Panels.Count(); i++)
		m_Panels[i]->VidInit();

	HideAllVGUIMenu();
}

void CBaseViewport::SetParent( vgui2::VPANEL parent )
{
	const bool bIsProportional = IsProportional();
	std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseViewport::SetParent entry this=%p parent=%p viewport=%p background=%p\n",
		this, (void *)parent, (void *)g_pViewport, (void *)m_pBackGround);
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport::SetParent this=%p parent=%p viewport=%p panels=%d\n",
		this, (void *)parent, (void *)g_pViewport, m_Panels.Count());
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport::SetParent state this=%p viewportParent(before)=%p currentParent(before)=%p background=%p backgroundParent(before)=%p\n",
		this,
		(void *)GetVParent(),
		(void *)GetParent(),
		(void *)m_pBackGround,
		(void *)(m_pBackGround ? m_pBackGround->GetVParent() : 0));

	BaseClass::SetParent( parent );

	//NOTE: the engine doesn't set the root to be proportional so it will override our settings. We must restore our settings here. - Solokiller
	SetProportional( bIsProportional );

	if (m_pBackGround)
		m_pBackGround->SetParent( parent );
	else
		gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport::SetParent no background yet this=%p parent=%p\n",
			this, (void *)parent);

	for( int i = 0; i< m_Panels.Count(); i++ )
	{
		m_Panels[ i ]->SetParent( parent );
	}

	// restore proportionality on animation controller
	// TODO: should all panels be restored to being proportional? 
	m_pAnimController->SetProportional( true );

	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport::SetParent done this=%p parent=%p\n", this, (void *)parent);
	std::fprintf(stderr, "[phase2][VGUI2-TRACE] CBaseViewport::SetParent exit this=%p parent=%p background=%p\n",
		this, (void *)parent, (void *)m_pBackGround);
}

bool CBaseViewport::UseVGUI1()
{
	return true;
}

void CBaseViewport::HideScoreBoard()
{
}

void CBaseViewport::HideAllVGUIMenu()
{
	// Hide-all is terminal; do not restore a previous active panel while closing.
	m_pLastActivePanel = NULL;

    for (int i = 0; i < m_Panels.Count(); i++)
    {
        if (m_Panels[i]->IsVisible())
            ShowPanel(m_Panels[i], false);
    }

    m_pActivePanel = NULL;
    m_pLastActivePanel = NULL;
}

void CBaseViewport::ActivateClientUI()
{
	for (int i = 0; i < m_GameUIPanels.Count(); i++)
	{
		m_GameUIPanels[i]->OnGameUIDeactivated();
	}
}

void CBaseViewport::HideClientUI()
{
	for (int i = 0; i < m_GameUIPanels.Count(); i++)
	{
		m_GameUIPanels[i]->OnGameUIActivated();
	}
}

void CBaseViewport::Shutdown()
{
}

void CBaseViewport::OnThink()
{
	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CBaseViewport::OnThink entry this=%p panels=%d gamePanels=%d active=%p last=%p\n",
	// 	this, m_Panels.Count(), m_GameUIPanels.Count(), (void *)m_pActivePanel, (void *)m_pLastActivePanel);
	// Clear our active panel pointer if the panel has made
	// itself invisible. Need this so we don't bring up dead panels
	// if they are stored as the last active panel
	if( m_pActivePanel && !m_pActivePanel->IsVisible() )
	{
		if( m_pLastActivePanel )
		{
			m_pActivePanel = m_pLastActivePanel;
			ShowPanel( m_pActivePanel, true );
			m_pLastActivePanel = NULL;
		}
		else
			m_pActivePanel = NULL;
	}

	m_pAnimController->UpdateAnimations( gEngfuncs.GetClientTime() );

	// check the auto-reload cvar
	//TODO - Solokiller
	//m_pAnimController->SetAutoReloadScript( hud_autoreloadscript.GetBool() );

	auto count = m_Panels.Count();

	for( decltype( count ) i = 0; i< count; ++i )
	{
		auto panel = m_Panels[ i ];
		if( panel->NeedsUpdate() && panel->IsVisible() )
		{
			panel->Update();
		}
	}

	int w, h;
	vgui2::surface()->GetScreenSize( w, h );

	if( m_OldSize[ 0 ] != w || m_OldSize[ 1 ] != h )
	{
		m_OldSize[ 0 ] = w;
		m_OldSize[ 1 ] = h;
		/*g_pClientMode->*/Layout();
	}

	BaseClass::OnThink();
	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CBaseViewport::OnThink exit this=%p panels=%d gamePanels=%d active=%p last=%p\n",
	// 	this, m_Panels.Count(), m_GameUIPanels.Count(), (void *)m_pActivePanel, (void *)m_pLastActivePanel);
}

void CBaseViewport::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	BaseClass::OnScreenSizeChanged( iOldWide, iOldTall );

	// reload the script file, so the screen positions in it are correct for the new resolution
	ReloadScheme( NULL );

	// recreate all the default panels
	RemoveAllPanels();

	// m_pBackGround = new CBackGroundPanel( nullptr );
	m_pBackGround = new CCSBackGroundPanel( nullptr );
	// gEngfuncs.Con_Printf("[phase2][VGUI2-CLIENT] CBaseViewport::OnScreenSizeChanged background=%p parent before attach=%p currentParent=%p\n",
	// 	(void *)m_pBackGround,
	// 	(void *)GetVParent(),
	// 	(void *)m_pBackGround->GetVParent());

	m_pBackGround->SetZPos( -20 ); // send it to the back 
	m_pBackGround->SetVisible( false );

	CreateDefaultPanels();

	vgui2::ipanel()->MoveToBack( m_pBackGround->GetVPanel() ); // really send it to the back 
}

void CBaseViewport::Paint()
{
}

void CBaseViewport::MoveToCenterOfScreen()
{
	int wx = 0, wy = 0, ww = 0, wh = 0;
	vgui2::surface()->GetWorkspaceBounds(wx, wy, ww, wh);
	int x = 0, y = 0, wide = 0, tall = 0;
	GetBounds(x, y, wide, tall);

	const int centeredX = (ww - wide) / 2;
	const int centeredY = (wh - tall) / 2;

	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CBaseViewport::MoveToCenterOfScreen this=%p workspace=%d,%d %dx%d bounds=%d,%d %dx%d\n",
		this, wx, wy, ww, wh, x, y, wide, tall);

	SetPos(centeredX, centeredY);
}

void CBaseViewport::Layout()
{
	vgui2::VPANEL pRoot;
	int wide, tall;

	// Make the viewport fill the root panel.
	if( ( pRoot = clientVGUI()->GetRootPanel() ) != NULL_HANDLE )
	{
		vgui2::ipanel()->GetSize( pRoot, wide, tall );

		const bool changed = wide != m_nRootSize[ 0 ] || tall != m_nRootSize[ 1 ];
		m_nRootSize[ 0 ] = wide;
		m_nRootSize[ 1 ] = tall;

		SetBounds( 0, 0, wide, tall );
		MoveToCenterOfScreen();
		if( changed )
		{
			ReloadScheme();
		}
	}
}

void CBaseViewport::CreateDefaultPanels()
{
}

void CBaseViewport::UpdateAllPanels()
{
	auto count = m_Panels.Count();

	for( decltype( count ) iIndex = 0; iIndex < count; ++iIndex )
	{
		auto pPanel = m_Panels[ iIndex ];

		if( pPanel->IsVisible() )
		{
			pPanel->Update();
		}
	}
}

IViewportPanel* CBaseViewport::CreatePanelByName( const char* pszName )
{
	IViewportPanel* pPanel = nullptr;

	/*
	if( Q_strcmp( "name", pszName ) == 0 )
	{
		pPanel = new CClassName( this );
	}
	*/

	return pPanel;
}

IViewportPanel* CBaseViewport::FindPanelByName( const char* pszName )
{
	auto count = m_Panels.Count();

	for( decltype( count ) iIndex = 0; iIndex < count; ++iIndex )
	{
		if( Q_strcmp( m_Panels[ iIndex ]->GetName(), pszName ) == 0 )
			return m_Panels[ iIndex ];
	}

	return nullptr;
}

bool CBaseViewport::AddNewPanel( IViewportPanel* pPanel )
{
	if( !pPanel )
	{
		gEngfuncs.Con_Printf( "CBaseViewport::AddNewPanel: Null panel!\n" );
		return false;
	}

	if( FindPanelByName( pPanel->GetName() ) )
	{
		gEngfuncs.Con_Printf( "CBaseViewport::AddNewPanel: A panel with name '%s' already exists.\n", pPanel->GetName() );
		return false;
	}

	m_Panels.AddToTail( pPanel );
	pPanel->SetParent( GetVPanel() );

	return true;
}

void CBaseViewport::ShowPanel( const char* pszName, bool bState )
{
	if( Q_strcmp( "all", pszName ) == 0 )
	{
		auto count = m_Panels.Count();

		for( decltype( count ) iIndex = 0; iIndex < count; ++iIndex )
		{
			ShowPanel( m_Panels[ iIndex ], bState );
		}

		return;
	}

	IViewportPanel* pPanel = nullptr;

	if( Q_strcmp("active", pszName ) == 0 )
	{
		pPanel = m_pActivePanel;
	}
	else
	{
		pPanel = FindPanelByName( pszName );
	}

	if( !pPanel )
	{
		pPanel = CreatePanelByName( pszName );
		if( !pPanel )
			return;

		AddNewPanel( pPanel );
	}

	ShowPanel( pPanel, bState );
}

void CBaseViewport::ShowPanel( IViewportPanel* pPanel, bool bState )
{
	std::fprintf(stderr, "[phase4][VGUI2-TRACE] CBaseViewport::ShowPanel entry this=%p panel=%p state=%d active=%p last=%p viewport=%p\n",
		this, (void *)pPanel, bState ? 1 : 0, (void *)m_pActivePanel, (void *)m_pLastActivePanel, (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CBaseViewport::ShowPanel this=%p panel=%p state=%d active=%p last=%p viewport=%p\n",
		this, (void *)pPanel, bState ? 1 : 0, (void *)m_pActivePanel, (void *)m_pLastActivePanel, (void *)g_pViewport);
	if( bState )
	{
		gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CBaseViewport::ShowPanel open this=%p panel=%p active=%p last=%p parent=%p\n",
			this, (void *)pPanel, (void *)m_pActivePanel, (void *)m_pLastActivePanel, (void *)pPanel->GetVPanel());

		// if this is an 'active' panel, deactivate old active panel
		if( pPanel->HasInputElements() )
		{
				gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CBaseViewport::ShowPanel activating input panel this=%p panel=%p visible=%d active=%p\n",
					this, (void *)pPanel, pPanel->IsVisible() ? 1 : 0, (void *)m_pActivePanel);
			// don't show input panels during normal demo playback
			if( gEngfuncs.pDemoAPI->IsPlayingback() && !gEngfuncs.IsSpectateOnly() )
				return;

			if( ( m_pActivePanel != nullptr ) && ( m_pActivePanel != pPanel ) )
			{
				// store a pointer to the currently active panel
				// so we can restore it later
				m_pLastActivePanel = m_pActivePanel;
				m_pActivePanel->ShowPanel( false );
			}

			m_pActivePanel = pPanel;
		}
	}
	else
	{
		// if this is our current active panel
		// update m_pActivePanel pointer
		if( m_pActivePanel == pPanel )
		{
			m_pActivePanel = nullptr;
		}

		// restore the previous active panel if it exists
		if( m_pLastActivePanel )
		{
			m_pActivePanel = m_pLastActivePanel;
			m_pLastActivePanel = nullptr;

			m_pActivePanel->ShowPanel( true );
		}
	}

	// just show/hide panel
	int preX = 0, preY = 0, preW = 0, preH = 0;
	vgui2::ipanel()->GetPos( pPanel->GetVPanel(), preX, preY );
	vgui2::ipanel()->GetSize( pPanel->GetVPanel(), preW, preH );
	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CBaseViewport::ShowPanel before-show this=%p panel=%p state=%d pos=%d,%d size=%dx%d visible=%d vpanel=%p\n",
		this, (void *)pPanel, bState ? 1 : 0, preX, preY, preW, preH, pPanel->IsVisible() ? 1 : 0, (void *)pPanel->GetVPanel());

		gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CBaseViewport::ShowPanel calling panel->ShowPanel this=%p panel=%p state=%d parent=%p\n",
			this, (void *)pPanel, bState ? 1 : 0, (void *)pPanel->GetVPanel());
	pPanel->ShowPanel( bState );
	int postX = 0, postY = 0, postW = 0, postH = 0;
	vgui2::ipanel()->GetPos( pPanel->GetVPanel(), postX, postY );
	vgui2::ipanel()->GetSize( pPanel->GetVPanel(), postW, postH );
	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CBaseViewport::ShowPanel after-show this=%p panel=%p state=%d pos=%d,%d size=%dx%d visible=%d vpanel=%p\n",
		this, (void *)pPanel, bState ? 1 : 0, postX, postY, postW, postH, pPanel->IsVisible() ? 1 : 0, (void *)pPanel->GetVPanel());
	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CBaseViewport::ShowPanel panel->ShowPanel done this=%p panel=%p state=%d visible=%d active=%p last=%p\n",
		this, (void *)pPanel, bState ? 1 : 0, pPanel->IsVisible() ? 1 : 0, (void *)m_pActivePanel, (void *)m_pLastActivePanel);

	UpdateAllPanels(); // let other panels rearrange
	std::fprintf(stderr, "[phase4][VGUI2-TRACE] CBaseViewport::ShowPanel exit this=%p panel=%p state=%d visible=%d active=%p last=%p\n",
		this, (void *)pPanel, bState ? 1 : 0, pPanel->IsVisible() ? 1 : 0, (void *)m_pActivePanel, (void *)m_pLastActivePanel);
}

void CBaseViewport::RemoveAllPanels()
{
	auto count = m_Panels.Count();

	for( decltype( count ) iIndex = 0; iIndex < count; ++iIndex )
	{
		auto vPanel = m_Panels[ iIndex ]->GetVPanel();

		vgui2::ipanel()->DeletePanel( vPanel );
	}

	if( m_pBackGround )
	{
		m_pBackGround->MarkForDeletion();
		m_pBackGround = nullptr;
	}

	m_Panels.Purge();

	m_pActivePanel = nullptr;
	m_pLastActivePanel = nullptr;
}

IViewportPanel* CBaseViewport::GetActivePanel()
{
	return m_pActivePanel;
}

IViewportPanel* CBaseViewport::GetLastActivePanel()
{
	return m_pLastActivePanel;
}

bool CBaseViewport::IsBackGroundVisible() const
{
	return m_pBackGround->IsVisible();
}

void CBaseViewport::ShowBackGround( bool bState )
{
	int x = 0, y = 0, wide = 0, tall = 0;
	if (m_pBackGround)
		m_pBackGround->GetBounds(x, y, wide, tall);

	gEngfuncs.Con_Printf(
		"[VGUI2-CLIENT] CBaseViewport::ShowBackGround this=%p show=%d background=%p parent=%p parentName='%s' bounds=%d,%d %dx%d visible=%d\n",
		this,
		bState ? 1 : 0,
		(void *)m_pBackGround,
		(void *)(m_pBackGround ? m_pBackGround->GetVParent() : 0),
		(m_pBackGround && m_pBackGround->GetParent()) ? m_pBackGround->GetParent()->GetName() : "<null>",
		x, y, wide, tall,
		m_pBackGround ? (m_pBackGround->IsVisible() ? 1 : 0) : 0);

	// disbaled tempoarily
	// if (bState && m_pBackGround)
	// {
	// 	m_pBackGround->Activate();
	// 	vgui2::ipanel()->MoveToBack( m_pBackGround->GetVPanel() );
	// }

	m_pBackGround->SetVisible( bState );
}

void CBaseViewport::ReloadScheme()
{
	ReloadScheme( "resource/ClientScheme.res" );
}

void CBaseViewport::ReloadScheme( const char* pszFromFile )
{
	// See if scheme should change

	if( pszFromFile != nullptr )
	{
		vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFile(pszFromFile, "ClientScheme");

		SetScheme( scheme );
		SetProportional( true );
		m_pAnimController->SetScheme( scheme );
	}

	// Force a reload
#if 0
	if( !m_pAnimController->SetScriptFile( GetVPanel(), UI_HUDANIMS_FILENAME, true ) )
	{
		Assert( false );
	}
#endif

	SetProportional( true );

	// reload the .res file from disk
	//LoadControlSettings( UI_HUDLAYOUT_FILENAME );

	//TODO: implement - Solokiller
	//Hud().RefreshHudTextures();

	InvalidateLayout( true, true );

	// reset the hud
	gHUD.MsgFunc_ResetHUD(nullptr, 0, nullptr);
}

IGameUIPanel *CBaseViewport::CreateGameUIPanelByName(const char *pszName)
{
	return nullptr;
}

bool CBaseViewport::AddNewGameUIPanel(IGameUIPanel *pPanel)
{
	if (!pPanel)
	{
		gEngfuncs.Con_Printf("CBaseViewport::AddNewGameUIPanel: Null panel!\n");
		return false;
	}

	if (FindGameUIPanelByName(pPanel->GetName()))
	{
		gEngfuncs.Con_Printf("CBaseViewport::AddNewGameUIPanel: A panel with name '%s' already exists.\n", pPanel->GetName());
		return false;
	}

	m_GameUIPanels.AddToTail(pPanel);

	return true;
}

IGameUIPanel *CBaseViewport::FindGameUIPanelByName(const char *pszName)
{
	auto count = m_GameUIPanels.Count();

	for (decltype(count) iIndex = 0; iIndex < count; ++iIndex)
	{
		if (Q_strcmp(m_GameUIPanels[iIndex]->GetName(), pszName) == 0)
			return m_GameUIPanels[iIndex];
	}

	return nullptr;
}
