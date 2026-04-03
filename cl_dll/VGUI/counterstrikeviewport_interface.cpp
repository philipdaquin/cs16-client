// #include "hud.h"

// #if !defined(__APPLE__)

// #include "interface.h"

// #include <IClientVGUI.h>
// #include <tier2/tier2.h>
// #include <vgui/VGUI2.h>
// #include <vgui_controls/Controls.h>

// #ifndef SOURCE_SDK_VGUI_NS_ALIAS
// namespace vgui = vgui2;
// #endif

// extern CCounterStrikeViewport *g_pViewport;

// void CounterStrikeViewport_Destroy( void );

// namespace
// {
// bool g_bClientVGUIInitialized = false;

// class CClientVGUI : public IClientVGUI
// {
// public:
// 	void Initialize( CreateInterfaceFn *pFactories, int iNumFactories ) override
// 	{
// 		if( g_bClientVGUIInitialized )
// 			return;

// 		if( !pFactories || iNumFactories <= 0 )
// 			return;

// 		ConnectTier2Libraries( pFactories, iNumFactories );

// 		if( !vgui::VGui_InitInterfacesList( "CLIENT", pFactories, iNumFactories ) )
// 			return;

// 		g_bClientVGUIInitialized = ( vgui::surface() != nullptr && vgui::ivgui() != nullptr && vgui::ipanel() != nullptr );
// 	}

// 	void Start() override
// 	{
// 		if( !g_bClientVGUIInitialized || !g_pViewport )
// 			return;

// 		if( m_hParent )
// 			g_pViewport->SetParent( m_hParent );
// 	}

// 	void SetParent( vgui::VPANEL parent ) override
// 	{
// 		m_hParent = parent;

// 		if( g_pViewport && m_hParent )
// 			g_pViewport->SetParent( m_hParent );
// 	}

// 	int UseVGUI1() override
// 	{
// 		return 0;
// 	}

// 	void HideScoreBoard() override
// 	{
// 	}

// 	void HideAllVGUIMenu() override
// 	{
// 		if( g_pViewport )
// 			g_pViewport->HidePanel( -1 );
// 	}

// 	void ActivateClientUI() override
// 	{
// 	}

// 	void HideClientUI() override
// 	{
// 		if( g_pViewport )
// 			g_pViewport->HidePanel( -1 );
// 	}

// 	void Shutdown() override
// 	{
// 		CounterStrikeViewport_Destroy();
// 		DisconnectTier2Libraries();
// 		g_bClientVGUIInitialized = false;
// 		m_hParent = 0;
// 	}

// private:
// 	vgui::VPANEL m_hParent = 0;
// };
// }

// bool CounterStrikeViewport_IsVGUI2Available( void )
// {
// 	return vgui::surface() && vgui::ivgui() && vgui::ipanel() && vgui::surface()->GetEmbeddedPanel();
// }

// /*
//  * Phase 1 note:
//  * `hud_vgui2print.cpp` is still intentionally unimplemented. The local codebase only describes it as a
//  * "VGUI print handler", and no live call sites or confirmed Steam CS 1.6 behavior were found that would
//  * let us determine whether it should replace center-print, HUD text, chat text, or a dedicated surface
//  * text overlay. That behavior needs to be confirmed before we add a client-facing implementation.
//  */

// static CClientVGUI g_ClientVGUI;
// EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CClientVGUI, IClientVGUI, ICLIENTVGUI_NAME, g_ClientVGUI );

// #else

// bool CounterStrikeViewport_IsVGUI2Available( void )
// {
// 	return false;
// }

// #endif


#include "hud.h"

class CCounterStrikeViewport;
extern CCounterStrikeViewport *g_pViewport;

bool CounterStrikeViewport_Create( void );
void CounterStrikeViewport_Destroy( void );

bool CounterStrikeViewport_IsVGUI2Available( void )
{
#if !defined(__APPLE__)
    // check vgui2 surface is live
    extern bool VGui2_IsEnabled( void );
    return VGui2_IsEnabled();
#else
    return false;
#endif
}