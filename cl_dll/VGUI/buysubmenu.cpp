#include "hud.h"

#if !defined(__APPLE__)

#include <strings.h>

#include <KeyValues.h>
#include <vgui/KeyCode.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/EditablePanel.h>

#ifndef SOURCE_SDK_VGUI_NS_ALIAS
namespace vgui = vgui2;
#endif

class CBuyMenu;

void BuyMenu_ShowMainPanel( CBuyMenu *menu );
void BuyMenu_CloseFromSubMenu( CBuyMenu *menu );

enum
{
	BUY_CATEGORY_NONE = 0,
	BUY_CATEGORY_PISTOLS,
	BUY_CATEGORY_SHOTGUNS,
	BUY_CATEGORY_SUBMACHINEGUNS,
	BUY_CATEGORY_RIFLES,
	BUY_CATEGORY_MACHINEGUNS,
	BUY_CATEGORY_EQUIPMENT,
};

static bool CommandEquals( const char *lhs, const char *rhs )
{
	return lhs && rhs && strcasecmp( lhs, rhs ) == 0;
}

static const char *GetButtonCommand( vgui::Button *button )
{
	if( !button )
		return nullptr;

	KeyValues *command = button->GetCommand();
	if( !command )
		return nullptr;

	return command->GetString( "command", "" );
}

static int GetButtonCost( vgui::Button *button )
{
	const char *command = GetButtonCommand( button );
	if( !command || !command[0] )
		return -1;

	struct CostEntry
	{
		const char *command;
		int cost;
	};

	static const CostEntry kCosts[] =
	{
		{ "glock", 400 },
		{ "usp", 500 },
		{ "p228", 600 },
		{ "deagle", 650 },
		{ "fn57", 750 },
		{ "elites", 800 },
		{ "m3", 1700 },
		{ "xm1014", 3000 },
		{ "tmp", 1250 },
		{ "mac10", 1400 },
		{ "mp5", 1500 },
		{ "ump45", 1700 },
		{ "p90", 2350 },
		{ "famas", 2250 },
		{ "galil", 2000 },
		{ "ak47", 2500 },
		{ "scout", 2750 },
		{ "m4a1", 3100 },
		{ "aug", 3500 },
		{ "sg552", 3500 },
		{ "sg550", 4200 },
		{ "awp", 4750 },
		{ "g3sg1", 5000 },
		{ "m249", 5750 },
		{ "vest", 650 },
		{ "vesthelm", 1000 },
		{ "flash", 200 },
		{ "hegren", 300 },
		{ "sgren", 300 },
		{ "defuser", 200 },
		{ "nvgs", 1250 },
		{ "nvg", 1250 },
		{ "shield", 2200 },
	};

	for( size_t i = 0; i < ARRAYSIZE( kCosts ); ++i )
	{
		if( CommandEquals( command, kCosts[i].command ) )
			return kCosts[i].cost;
	}

	return -1;
}

static void ApplyButtonStatesRecursive( vgui::Panel *panel, int currentMoney )
{
	if( !panel )
		return;

	if( vgui::Button *button = dynamic_cast<vgui::Button *>( panel ) )
	{
		const int cost = GetButtonCost( button );
		if( cost >= 0 )
			button->SetEnabled( currentMoney >= cost );
	}

	for( int i = 0; i < panel->GetChildCount(); ++i )
		ApplyButtonStatesRecursive( panel->GetChild( i ), currentMoney );
}

class CBuySubMenu;

class CBuySubMenuPage : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CBuySubMenuPage, vgui::EditablePanel );

public:
	CBuySubMenuPage( vgui::Panel *parent, CBuySubMenu *owner, const char *resourceName );

protected:
	void OnCommand( const char *command ) override;

private:
	CBuySubMenu *m_pOwner;
};

class CBuySubMenu : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CBuySubMenu, vgui::EditablePanel );

public:
	CBuySubMenu( vgui::Panel *parent, CBuyMenu *owner );

	void ShowCategory( int category, int teamNumber );
	void HideSubMenu();
	void UpdateButtonStates( int currentMoney );
	bool IsSubMenuVisible() const;
	void HandleCommand( const char *command );

protected:
	void OnCommand( const char *command ) override;
	void OnKeyCodePressed( vgui::KeyCode code ) override;

private:
	void RebuildPage( const char *resourceName );
	const char *GetResourceForCategory( int category, int teamNumber ) const;
	void UpdateBounds();
	void SendBuyCommand( const char *command ) const;

private:
	CBuyMenu *m_pOwner;
	CBuySubMenuPage *m_pPage;
	vgui::Button *m_pBackButton;
	int m_iCurrentCategory;
	int m_iCurrentTeam;
	bool m_bVisible;
};

CBuySubMenuPage::CBuySubMenuPage( vgui::Panel *parent, CBuySubMenu *owner, const char *resourceName ) :
	BaseClass( parent, "BuySubMenuPage" ),
	m_pOwner( owner )
{
	SetProportional( true );
	SetPaintBackgroundEnabled( false );
	SetPaintEnabled( false );
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	LoadControlSettings( resourceName );
}

void CBuySubMenuPage::OnCommand( const char *command )
{
	if( m_pOwner )
		m_pOwner->HandleCommand( command );
}

CBuySubMenu::CBuySubMenu( vgui::Panel *parent, CBuyMenu *owner ) :
	BaseClass( parent, "BuySubMenu" ),
	m_pOwner( owner ),
	m_pPage( nullptr ),
	m_pBackButton( nullptr ),
	m_iCurrentCategory( BUY_CATEGORY_NONE ),
	m_iCurrentTeam( 0 ),
	m_bVisible( false )
{
	SetProportional( true );
	SetPaintBackgroundEnabled( false );
	SetPaintEnabled( false );
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetVisible( false );

	m_pBackButton = new vgui::Button( this, "BackButton", "Back", this, "vguiback" );
	m_pBackButton->SetProportional( true );
	m_pBackButton->SetBounds( 249, 380, 160, 20 );

	UpdateBounds();
}

void CBuySubMenu::ShowCategory( int category, int teamNumber )
{
	const char *resourceName = GetResourceForCategory( category, teamNumber );
	if( !resourceName )
		return;

	if( category != m_iCurrentCategory || teamNumber != m_iCurrentTeam || !m_pPage )
	{
		RebuildPage( resourceName );
		m_iCurrentCategory = category;
		m_iCurrentTeam = teamNumber;
	}

	UpdateBounds();
	SetVisible( true );
	MoveToFront();
	RequestFocus();

	if( m_pPage )
	{
		m_pPage->SetVisible( true );
		m_pPage->MoveToFront();
		m_pPage->RequestFocus();
	}

	if( m_pBackButton )
		m_pBackButton->MoveToFront();

	m_bVisible = true;
}

void CBuySubMenu::HideSubMenu()
{
	SetVisible( false );
	if( m_pPage )
		m_pPage->SetVisible( false );
	m_bVisible = false;
}

void CBuySubMenu::UpdateButtonStates( int currentMoney )
{
	ApplyButtonStatesRecursive( m_pPage, currentMoney );
}

bool CBuySubMenu::IsSubMenuVisible() const
{
	return m_bVisible && IsVisible();
}

void CBuySubMenu::OnCommand( const char *command )
{
	HandleCommand( command );
}

void CBuySubMenu::HandleCommand( const char *command )
{
	if( !command || !command[0] )
	{
		BaseClass::OnCommand( command );
		return;
	}

	if( CommandEquals( command, "vguiback" ) )
	{
		HideSubMenu();
		BuyMenu_ShowMainPanel( m_pOwner );
		return;
	}

	if( CommandEquals( command, "vguicancel" ) )
	{
		BuyMenu_CloseFromSubMenu( m_pOwner );
		return;
	}

	SendBuyCommand( command );
}

void CBuySubMenu::OnKeyCodePressed( vgui::KeyCode code )
{
	if( code == vgui::KEY_ESCAPE )
	{
		BuyMenu_CloseFromSubMenu( m_pOwner );
		return;
	}

	BaseClass::OnKeyCodePressed( code );
}

void CBuySubMenu::RebuildPage( const char *resourceName )
{
	if( m_pPage )
	{
		delete m_pPage;
		m_pPage = nullptr;
	}

	m_pPage = new CBuySubMenuPage( this, this, resourceName );
	m_pPage->SetBounds( 0, 0, GetWide(), GetTall() );
}

const char *CBuySubMenu::GetResourceForCategory( int category, int teamNumber ) const
{
	switch( category )
	{
	case BUY_CATEGORY_PISTOLS:
		return teamNumber == TEAM_TERRORIST ? "resource/UI/BuyPistols_TER.res" : "resource/UI/BuyPistols_CT.res";
	case BUY_CATEGORY_SHOTGUNS:
		return teamNumber == TEAM_TERRORIST ? "resource/UI/BuyShotguns_TER.res" : "resource/UI/BuyShotguns_CT.res";
	case BUY_CATEGORY_SUBMACHINEGUNS:
		return teamNumber == TEAM_TERRORIST ? "resource/UI/BuySubMachineguns_TER.res" : "resource/UI/BuySubMachineguns_CT.res";
	case BUY_CATEGORY_RIFLES:
		return teamNumber == TEAM_TERRORIST ? "resource/UI/BuyRifles_TER.res" : "resource/UI/BuyRifles_CT.res";
	case BUY_CATEGORY_MACHINEGUNS:
		return teamNumber == TEAM_TERRORIST ? "resource/UI/BuyMachineguns_TER.res" : "resource/UI/BuyMachineguns_CT.res";
	case BUY_CATEGORY_EQUIPMENT:
		if( teamNumber == TEAM_TERRORIST )
			return "resource/UI/BuyEquipment_TER.res";
		if( teamNumber == TEAM_CT )
			return "resource/UI/BuyEquipment_CT.res";
		return "resource/UI/BuyEquipment.res";
	default:
		return nullptr;
	}
}

void CBuySubMenu::UpdateBounds()
{
	int wide = ScreenWidth;
	int tall = ScreenHeight;

	if( vgui::surface() )
		vgui::surface()->GetScreenSize( wide, tall );

	SetBounds( 0, 0, wide, tall );

	if( m_pPage )
		m_pPage->SetBounds( 0, 0, wide, tall );
}

void CBuySubMenu::SendBuyCommand( const char *command ) const
{
	if( command && command[0] )
		gEngfuncs.pfnClientCmd( command );
}

CBuySubMenu *BuySubMenu_Create( vgui::Panel *parent, CBuyMenu *owner )
{
	return new CBuySubMenu( parent, owner );
}

void BuySubMenu_Destroy( CBuySubMenu *submenu )
{
	delete submenu;
}

void BuySubMenu_Show( CBuySubMenu *submenu, int category, int teamNumber )
{
	if( submenu )
		submenu->ShowCategory( category, teamNumber );
}

void BuySubMenu_Hide( CBuySubMenu *submenu )
{
	if( submenu )
		submenu->HideSubMenu();
}

void BuySubMenu_UpdateMoney( CBuySubMenu *submenu, int currentMoney )
{
	if( submenu )
		submenu->UpdateButtonStates( currentMoney );
}

bool BuySubMenu_IsVisible( const CBuySubMenu *submenu )
{
	return submenu && submenu->IsSubMenuVisible();
}

#else

class CBuyMenu
{
};

class CBuySubMenu
{
};

CBuySubMenu *BuySubMenu_Create( void *, CBuyMenu * )
{
	return nullptr;
}

void BuySubMenu_Destroy( CBuySubMenu * )
{
}

void BuySubMenu_Show( CBuySubMenu *, int, int )
{
}

void BuySubMenu_Hide( CBuySubMenu * )
{
}

void BuySubMenu_UpdateMoney( CBuySubMenu *, int )
{
}

bool BuySubMenu_IsVisible( const CBuySubMenu * )
{
	return false;
}

#endif
