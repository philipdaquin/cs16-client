#include "hud.h"

#if !defined(__APPLE__)

#include <vgui/VGUI2.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/EditablePanel.h>

#ifndef SOURCE_SDK_VGUI_NS_ALIAS
namespace vgui = vgui2;
#endif

class CBuyMenu;

CBuyMenu *BuyMenu_Create( vgui::Panel *parent );
void BuyMenu_Destroy( CBuyMenu *menu );
void BuyMenu_Open( CBuyMenu *menu );
void BuyMenu_Hide( CBuyMenu *menu );
bool BuyMenu_IsVisible( const CBuyMenu *menu );

class CCounterStrikeViewport : public vgui::EditablePanel
{
public:
	CCounterStrikeViewport();
	~CCounterStrikeViewport() override;
	void ShowPanel( int menuType, bool show );
	void HidePanel( int menuType );
	bool IsAnyPanelVisible() const;

private:
	void UpdateBounds()
	{
		int wide = ScreenWidth;
		int tall = ScreenHeight;

		if( vgui::surface() )
			vgui::surface()->GetScreenSize( wide, tall );

		SetBounds( 0, 0, wide, tall );
	}

private:
	int m_iActiveMenuType;
	bool m_bPanelVisible;
	CBuyMenu *m_pBuyMenu;
};

CCounterStrikeViewport::CCounterStrikeViewport() : vgui::EditablePanel( nullptr, "CounterStrikeViewport" ), m_iActiveMenuType( 0 ), m_bPanelVisible( false ), m_pBuyMenu( nullptr )
{
	SetProportional( false );
	SetVisible( false );
	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );
	SetPaintEnabled( false );
	SetPaintBackgroundEnabled( false );
	UpdateBounds();
}

CCounterStrikeViewport::~CCounterStrikeViewport()
{
	BuyMenu_Destroy( m_pBuyMenu );
	m_pBuyMenu = nullptr;
}

void CCounterStrikeViewport::ShowPanel( int menuType, bool show )
{
	UpdateBounds();

	if( menuType == MENU_BUY )
	{
		if( show )
		{
			if( !m_pBuyMenu )
				m_pBuyMenu = BuyMenu_Create( this );

			if( !m_pBuyMenu )
				return;

			m_iActiveMenuType = menuType;
			m_bPanelVisible = true;
			SetVisible( true );
			SetMouseInputEnabled( true );
			SetKeyBoardInputEnabled( true );
			BuyMenu_Open( m_pBuyMenu );
			MoveToFront();
		}
		else if( m_pBuyMenu )
		{
			BuyMenu_Hide( m_pBuyMenu );
			m_bPanelVisible = BuyMenu_IsVisible( m_pBuyMenu );
			if( !m_bPanelVisible )
			{
				m_iActiveMenuType = 0;
				SetVisible( false );
				SetMouseInputEnabled( false );
				SetKeyBoardInputEnabled( false );
			}
		}

		return;
	}

	m_iActiveMenuType = menuType;
	m_bPanelVisible = show;
	SetVisible( show );
	SetMouseInputEnabled( show );
	SetKeyBoardInputEnabled( show );

	if( show )
		MoveToFront();
}

void CCounterStrikeViewport::HidePanel( int menuType )
{
	if( !m_bPanelVisible )
		return;

	if( menuType != -1 && menuType != m_iActiveMenuType )
		return;

	if( m_iActiveMenuType == MENU_BUY && m_pBuyMenu )
		BuyMenu_Hide( m_pBuyMenu );

	m_bPanelVisible = false;
	m_iActiveMenuType = 0;
	SetVisible( false );
	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );
}

bool CCounterStrikeViewport::IsAnyPanelVisible() const
{
	if( m_iActiveMenuType == MENU_BUY && m_pBuyMenu )
		return BuyMenu_IsVisible( m_pBuyMenu );

	return m_bPanelVisible;
}

CCounterStrikeViewport *g_pViewport = nullptr;

bool CounterStrikeViewport_Create( void )
{
	if( g_pViewport )
	{
		gEngfuncs.Con_Printf( "VGUI2: viewport already exists\n" );
		return true;
	}

	if( !vgui::surface() )
	{
		gEngfuncs.Con_Printf( "VGUI2: surface() is null\n" );
		return false;
	}

	if( !vgui::ipanel() )
	{
		gEngfuncs.Con_Printf( "VGUI2: ipanel() is null\n" );
		return false;
	}

	const vgui::VPANEL embeddedPanel = vgui::surface()->GetEmbeddedPanel();
	if( !embeddedPanel )
	{
		gEngfuncs.Con_Printf( "VGUI2: GetEmbeddedPanel() returned 0\n" );
		return false;
	}

	g_pViewport = new CCounterStrikeViewport();
	g_pViewport->SetParent( embeddedPanel );
	gEngfuncs.Con_Printf( "VGUI2: viewport created successfully, g_pViewport=%p\n", g_pViewport );
	return true;
}

void CounterStrikeViewport_Destroy( void )
{
	if( !g_pViewport )
		return;

	delete g_pViewport;
	g_pViewport = nullptr;
}

bool CounterStrikeViewport_IsVisible( void )
{
	return g_pViewport && g_pViewport->IsAnyPanelVisible();
}

#else

class CCounterStrikeViewport
{
public:
	void ShowPanel( int menuType, bool show );
	void HidePanel( int menuType );
	bool IsAnyPanelVisible() const;
};

void CCounterStrikeViewport::ShowPanel( int, bool )
{
}

void CCounterStrikeViewport::HidePanel( int )
{
}

bool CCounterStrikeViewport::IsAnyPanelVisible() const
{
	return false;
}

CCounterStrikeViewport *g_pViewport = nullptr;

bool CounterStrikeViewport_Create( void )
{
	gEngfuncs.Con_Printf( "VGUI2: viewport creation is stubbed on __APPLE__ build\n" );
	return false;
}

void CounterStrikeViewport_Destroy( void )
{
	g_pViewport = nullptr;
}

bool CounterStrikeViewport_IsVisible( void )
{
	return false;
}

#endif
