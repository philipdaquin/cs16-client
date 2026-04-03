#include "hud.h"

#if !defined(__APPLE__)

#include <vgui/VGUI2.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/EditablePanel.h>

#ifndef SOURCE_SDK_VGUI_NS_ALIAS
namespace vgui = vgui2;
#endif

class CCounterStrikeViewport : public vgui::EditablePanel
{
public:
	CCounterStrikeViewport();
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
};

CCounterStrikeViewport::CCounterStrikeViewport() : vgui::EditablePanel( nullptr, "CounterStrikeViewport" ), m_iActiveMenuType( 0 ), m_bPanelVisible( false )
{
	SetProportional( false );
	SetVisible( false );
	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );
	SetPaintEnabled( false );
	SetPaintBackgroundEnabled( false );
	UpdateBounds();
}

void CCounterStrikeViewport::ShowPanel( int menuType, bool show )
{
	m_iActiveMenuType = menuType;
	m_bPanelVisible = show;

	UpdateBounds();
	SetVisible( show );

	if( show )
		MoveToFront();
}

void CCounterStrikeViewport::HidePanel( int menuType )
{
	if( !m_bPanelVisible )
		return;

	if( menuType != -1 && menuType != m_iActiveMenuType )
		return;

	m_bPanelVisible = false;
	SetVisible( false );
}

bool CCounterStrikeViewport::IsAnyPanelVisible() const
{
	return m_bPanelVisible;
}

CCounterStrikeViewport *g_pViewport = nullptr;

bool CounterStrikeViewport_Create( void )
{
	if( g_pViewport )
		return true;

	if( !vgui::surface() || !vgui::ipanel() )
		return false;

	const vgui::VPANEL embeddedPanel = vgui::surface()->GetEmbeddedPanel();
	if( !embeddedPanel )
		return false;

	g_pViewport = new CCounterStrikeViewport();
	g_pViewport->SetParent( embeddedPanel );
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
