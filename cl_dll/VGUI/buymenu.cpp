#include "hud.h"

#if !defined(__APPLE__)

#include <stdlib.h>
#include <strings.h>

#include <KeyValues.h>
#include <vgui/KeyCode.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/MessageMap.h>

#ifndef SOURCE_SDK_VGUI_NS_ALIAS
namespace vgui = vgui2;
#endif

class CBuySubMenu;

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

class MouseOverPanelButton : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( MouseOverPanelButton, vgui::Button );

public:
	MouseOverPanelButton( vgui::Panel *parent, const char *panelName, const char *text ) : BaseClass( parent, panelName, text )
	{
		SetContentAlignment( vgui::Label::a_west );
	}

protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override
	{
		BaseClass::ApplySchemeSettings( pScheme );
		SetContentAlignment( vgui::Label::a_west );
	}
};

DECLARE_BUILD_FACTORY_DEFAULT_TEXT( MouseOverPanelButton, "" );

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

class CBuyMenu;

CBuySubMenu *BuySubMenu_Create( vgui::Panel *parent, CBuyMenu *owner );
void BuySubMenu_Destroy( CBuySubMenu *submenu );
void BuySubMenu_Show( CBuySubMenu *submenu, int category, int teamNumber );
void BuySubMenu_Hide( CBuySubMenu *submenu );
void BuySubMenu_UpdateMoney( CBuySubMenu *submenu, int currentMoney );
bool BuySubMenu_IsVisible( const CBuySubMenu *submenu );

class CBuyMenuPage : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CBuyMenuPage, vgui::EditablePanel );

public:
	CBuyMenuPage( vgui::Panel *parent, CBuyMenu *owner );

protected:
	void OnCommand( const char *command ) override;

private:
	CBuyMenu *m_pOwner;
};

class CBuyMenu : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CBuyMenu, vgui::EditablePanel );

public:
	CBuyMenu( vgui::Panel *parent );
	~CBuyMenu() override;

	void Open();
	void Hide();
	bool IsMenuVisible() const;
	void ShowMainMenu();
	void CloseMenu();
	void HandleCommand( const char *command );
	void UpdateButtonStates();
	int GetCurrentTeam() const;

protected:
	void OnTick() override;
	void OnCommand( const char *command ) override;
	void OnKeyCodePressed( vgui::KeyCode code ) override;

private:
	int GetCurrentMoney() const;
	int GetCategoryForCommand( const char *command ) const;
	void UpdateBounds();
	void SendClientCommand( const char *command ) const;

private:
	CBuyMenuPage *m_pMainMenuPage;
	CBuySubMenu *m_pSubMenu;
	bool m_bVisible;
	int m_iCurrentMoney;
};

CBuyMenuPage::CBuyMenuPage( vgui::Panel *parent, CBuyMenu *owner ) :
	BaseClass( parent, "BuyMenuMainPage" ),
	m_pOwner( owner )
{
	SetProportional( true );
	SetPaintBackgroundEnabled( false );
	SetPaintEnabled( false );
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	LoadControlSettings( "resource/UI/MainBuyMenu.res" );
}

void CBuyMenuPage::OnCommand( const char *command )
{
	if( m_pOwner )
		m_pOwner->HandleCommand( command );
}

CBuyMenu::CBuyMenu( vgui::Panel *parent ) :
	BaseClass( parent, "BuyMenu" ),
	m_pMainMenuPage( nullptr ),
	m_pSubMenu( nullptr ),
	m_bVisible( false ),
	m_iCurrentMoney( -1 )
{
	SetProportional( true );
	SetPaintBackgroundEnabled( false );
	SetPaintEnabled( false );
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetVisible( false );

	LoadControlSettings( "resource/UI/BuyMenu.res" );

	m_pMainMenuPage = new CBuyMenuPage( this, this );
	m_pMainMenuPage->SetBounds( 0, 0, GetWide(), GetTall() );

	m_pSubMenu = BuySubMenu_Create( this, this );
	BuySubMenu_Hide( m_pSubMenu );

	UpdateBounds();

	if( vgui::ivgui() )
		vgui::ivgui()->AddTickSignal( GetVPanel() );
}

CBuyMenu::~CBuyMenu()
{
	BuySubMenu_Destroy( m_pSubMenu );
	m_pSubMenu = nullptr;
}

void CBuyMenu::Open()
{
	UpdateBounds();
	ShowMainMenu();
	m_iCurrentMoney = -1;
	UpdateButtonStates();

	SetVisible( true );
	MoveToFront();
	RequestFocus();

	m_bVisible = true;
}

void CBuyMenu::Hide()
{
	if( !m_bVisible )
		return;

	SetVisible( false );
	if( m_pMainMenuPage )
		m_pMainMenuPage->SetVisible( true );
	BuySubMenu_Hide( m_pSubMenu );

	m_bVisible = false;

	vgui::Panel *parent = GetParent();
	if( parent )
	{
		parent->SetVisible( false );
		parent->SetMouseInputEnabled( false );
		parent->SetKeyBoardInputEnabled( false );
	}
}

bool CBuyMenu::IsMenuVisible() const
{
	return m_bVisible && IsVisible();
}

void CBuyMenu::ShowMainMenu()
{
	if( m_pMainMenuPage )
	{
		m_pMainMenuPage->SetVisible( true );
		m_pMainMenuPage->MoveToFront();
		m_pMainMenuPage->RequestFocus();
	}

	BuySubMenu_Hide( m_pSubMenu );
}

void CBuyMenu::CloseMenu()
{
	gEngfuncs.pfnClientCmd( "cancelselect" );
	Hide();
}

void CBuyMenu::UpdateButtonStates()
{
	const int currentMoney = GetCurrentMoney();
	if( currentMoney == m_iCurrentMoney )
		return;

	m_iCurrentMoney = currentMoney;

	ApplyButtonStatesRecursive( m_pMainMenuPage, currentMoney );
	BuySubMenu_UpdateMoney( m_pSubMenu, currentMoney );
}

int CBuyMenu::GetCurrentTeam() const
{
	if( g_iTeamNumber == TEAM_CT || g_iTeamNumber == TEAM_TERRORIST )
		return g_iTeamNumber;

	const int playerNum = gHUD.m_Scoreboard.m_iPlayerNum;
	if( playerNum >= 0 && playerNum < MAX_PLAYERS + 1 )
	{
		const int teamNumber = g_PlayerExtraInfo[playerNum].teamnumber;
		if( teamNumber == TEAM_CT || teamNumber == TEAM_TERRORIST )
			return teamNumber;
	}

	return 0;
}

void CBuyMenu::OnTick()
{
	if( !IsMenuVisible() )
		return;

	UpdateButtonStates();
}

void CBuyMenu::OnCommand( const char *command )
{
	HandleCommand( command );
}

void CBuyMenu::HandleCommand( const char *command )
{
	if( !command || !command[0] )
	{
		BaseClass::OnCommand( command );
		return;
	}

	if( CommandEquals( command, "vguicancel" ) )
	{
		CloseMenu();
		return;
	}

	const int category = GetCategoryForCommand( command );
	if( category != BUY_CATEGORY_NONE )
	{
		if( m_pMainMenuPage )
			m_pMainMenuPage->SetVisible( false );

		BuySubMenu_Show( m_pSubMenu, category, GetCurrentTeam() );
		BuySubMenu_UpdateMoney( m_pSubMenu, GetCurrentMoney() );
		return;
	}

	if( CommandEquals( command, "autobuy" ) ||
		CommandEquals( command, "rebuy" ) ||
		CommandEquals( command, "primammo" ) ||
		CommandEquals( command, "secammo" ) )
	{
		SendClientCommand( command );
		return;
	}

	BaseClass::OnCommand( command );
}

void CBuyMenu::OnKeyCodePressed( vgui::KeyCode code )
{
	if( code == vgui::KEY_ESCAPE )
	{
		CloseMenu();
		return;
	}

	BaseClass::OnKeyCodePressed( code );
}

int CBuyMenu::GetCurrentMoney() const
{
	if( !gHUD.cscl_currentmoney )
		return 0;

	return static_cast<int>( gHUD.cscl_currentmoney->value );
}

int CBuyMenu::GetCategoryForCommand( const char *command ) const
{
	if( CommandEquals( command, "Resource/UI/BuyPistols.res" ) )
		return BUY_CATEGORY_PISTOLS;
	if( CommandEquals( command, "Resource/UI/BuyShotguns.res" ) )
		return BUY_CATEGORY_SHOTGUNS;
	if( CommandEquals( command, "Resource/UI/BuySubMachineGuns.res" ) )
		return BUY_CATEGORY_SUBMACHINEGUNS;
	if( CommandEquals( command, "Resource/UI/BuyRifles.res" ) )
		return BUY_CATEGORY_RIFLES;
	if( CommandEquals( command, "Resource/UI/BuyMachineGuns.res" ) )
		return BUY_CATEGORY_MACHINEGUNS;
	if( CommandEquals( command, "Resource/UI/BuyEquipment.res" ) )
		return BUY_CATEGORY_EQUIPMENT;

	return BUY_CATEGORY_NONE;
}

void CBuyMenu::UpdateBounds()
{
	int wide = ScreenWidth;
	int tall = ScreenHeight;

	if( vgui::surface() )
		vgui::surface()->GetScreenSize( wide, tall );

	SetBounds( 0, 0, wide, tall );

	if( m_pMainMenuPage )
		m_pMainMenuPage->SetBounds( 0, 0, wide, tall );
}

void CBuyMenu::SendClientCommand( const char *command ) const
{
	if( command && command[0] )
		gEngfuncs.pfnClientCmd( command );
}

void BuyMenu_ShowMainPanel( CBuyMenu *menu )
{
	if( menu )
		menu->ShowMainMenu();
}

void BuyMenu_CloseFromSubMenu( CBuyMenu *menu )
{
	if( menu )
		menu->CloseMenu();
}

CBuyMenu *BuyMenu_Create( vgui::Panel *parent )
{
	return new CBuyMenu( parent );
}

void BuyMenu_Destroy( CBuyMenu *menu )
{
	delete menu;
}

void BuyMenu_Open( CBuyMenu *menu )
{
	if( menu )
		menu->Open();
}

void BuyMenu_Hide( CBuyMenu *menu )
{
	if( menu )
		menu->Hide();
}

bool BuyMenu_IsVisible( const CBuyMenu *menu )
{
	return menu && menu->IsMenuVisible();
}

#else

class CBuyMenu
{
};

CBuyMenu *BuyMenu_Create( void * )
{
	return nullptr;
}

void BuyMenu_Destroy( CBuyMenu * )
{
}

void BuyMenu_Open( CBuyMenu * )
{
}

void BuyMenu_Hide( CBuyMenu * )
{
}

bool BuyMenu_IsVisible( const CBuyMenu * )
{
	return false;
}

void BuyMenu_ShowMainPanel( CBuyMenu * )
{
}

void BuyMenu_CloseFromSubMenu( CBuyMenu * )
{
}

#endif
