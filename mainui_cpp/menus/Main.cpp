/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/


#include "Framework.h"
#include "Action.h"
#include "Bitmap.h"
#include "PicButton.h"
#include "YesNoMessageBox.h"
#include "keydefs.h"
#include "MenuStrings.h"
#include "PlayerIntroduceDialog.h"
#include "gameinfo.h"
#include "AnimatedBanner.h"
#include "MovieBanner.h"

#define ART_MINIMIZE_N	"gfx/shell/min_n"
#define ART_MINIMIZE_F	"gfx/shell/min_f"
#define ART_MINIMIZE_D	"gfx/shell/min_d"
#define ART_CLOSEBTN_N	"gfx/shell/cls_n"
#define ART_CLOSEBTN_F	"gfx/shell/cls_f"
#define ART_CLOSEBTN_D	"gfx/shell/cls_d"

class CMenuMain: public CMenuFramework
{
public:
	CMenuMain() : CMenuFramework( "CMenuMain" ) { }

	bool KeyDown( int key ) override;

private:
	void _Init() override;
	void _VidInit( ) override;
	void Think() override;

	void VidInit(bool connected);

	void QuitDialogCb();
	void DisconnectCb();
	void DisconnectDialogCb();
	void HazardCourseDialogCb();
	void HazardCourseCb();

	CMenuAnimatedBanner animatedBanner;
	CMenuMovieBanner movieBanner;

	CMenuPicButton	console;
	CMenuPicButton	resumeGame;
	CMenuPicButton	disconnect;
	CMenuPicButton	newGame;
	CMenuPicButton	hazardCourse;
	CMenuPicButton	configuration;
	CMenuPicButton	saveRestore;
	CMenuPicButton	multiPlayer;
	CMenuPicButton	findServers;
	CMenuPicButton	customGame;
	CMenuPicButton	readme;
	CMenuPicButton	previews;
	CMenuPicButton	quit;

	// buttons on top right. Maybe should be drawn if fullscreen == 1?
	CMenuBitmap	minimizeBtn;
	CMenuBitmap	quitButton;

	// quit dialog
	CMenuYesNoMessageBox dialog;

	bool bTrainMap;
	bool bCustomGame;
};

void CMenuMain::QuitDialogCb()
{
	if( CL_IsActive() && EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) == 1.0f )
		dialog.SetMessage( L( "StringsList_235" ) );
	else
		dialog.SetMessage( L( "GameUI_QuitConfirmationText" ) );

	dialog.onPositive.SetCommand( FALSE, "quit\n" );
	dialog.Show();
}

void CMenuMain::DisconnectCb()
{
	// EngFuncs::ClientCmd( false, "disconnect\n" );
	// VidInit( false );
	// CalcPosition();
	// CalcSizes();
	// VidInitItems();

	// Return straight to the root game menu after disconnecting.
	UI_CloseMenu();
	EngFuncs::KEY_SetDest( KEY_GAME );

	EngFuncs::ClientCmd( false, "disconnect\n" );
}

void CMenuMain::DisconnectDialogCb()
{
	dialog.onPositive = VoidCb( &CMenuMain::DisconnectCb );
	dialog.SetMessage( L( "Really disconnect?" ) );
	dialog.Show();
}

void CMenuMain::HazardCourseDialogCb()
{
	dialog.onPositive = VoidCb( &CMenuMain::HazardCourseCb );;
	dialog.SetMessage( L( "StringsList_234" ) );
	dialog.Show();
}

/*
=================
CMenuMain::Key
=================
*/
bool CMenuMain::KeyDown( int key )
{
	if( UI::Key::IsEscape( key ) )
	{
		if ( CL_IsActive( ))
		{
			if( !dialog.IsVisible() )
				UI_CloseMenu();
		}
		else
		{
			QuitDialogCb( );
		}
		return true;
	}
	return CMenuFramework::KeyDown( key );
}

/*
=================
UI_Main_HazardCourse
=================
*/
void CMenuMain::HazardCourseCb()
{
	if( EngFuncs::GetCvarFloat( "host_serverstate" ) && EngFuncs::GetCvarFloat( "maxplayers" ) > 1 )
		EngFuncs::HostEndGame( "end of the game" );

	EngFuncs::CvarSetValue( "skill", 1.0f );
	EngFuncs::CvarSetValue( "deathmatch", 0.0f );
	EngFuncs::CvarSetValue( "teamplay", 0.0f );
	EngFuncs::CvarSetValue( "pausable", 1.0f ); // singleplayer is always allowing pause
	EngFuncs::CvarSetValue( "coop", 0.0f );
	EngFuncs::CvarSetValue( "maxplayers", 1.0f ); // singleplayer

	EngFuncs::PlayBackgroundTrack( NULL, NULL );

	EngFuncs::ClientCmd( FALSE, "hazardcourse\n" );
}

void CMenuMain::_Init( void )
{
	if( gMenu.m_gameinfo.trainmap[0] && stricmp( gMenu.m_gameinfo.trainmap, gMenu.m_gameinfo.startmap ) != 0 )
		bTrainMap = true;
	else bTrainMap = false;

	if( EngFuncs::GetCvarFloat( "host_allow_changegame" ))
		bCustomGame = true;
	else bCustomGame = false;

	// console
	console.SetNameAndStatus( L( "GameUI_Console" ), NULL );
	console.iFlags |= QMF_NOTIFY;
	console.SetPicture( PC_CONSOLE );
	console.SetVisibility( gpGlobals->developer );
	SET_EVENT_MULTI( console.onReleased,
	{
		UI_SetActiveMenu( FALSE );
		EngFuncs::KEY_SetDest( KEY_CONSOLE );
	});

	// resumeGame.SetNameAndStatus( L( "GameUI_GameMenu_ResumeGame" ), L( "StringsList_188" ) );
	resumeGame.SetNameAndStatus( L( "GameUI_GameMenu_ResumeGame" ), NULL );
	resumeGame.SetPicture( PC_RESUME_GAME );
	resumeGame.iFlags |= QMF_NOTIFY;
	// resumeGame.charSize = 6;
	// resumeGame.SetTextHeight( 10 );
	// resumeGame.SetTextHeight( 8 );
	resumeGame.SetTextHeight( 6 );
	resumeGame.onReleased = UI_CloseMenu;

	// disconnect.SetNameAndStatus( L( "GameUI_GameMenu_Disconnect" ), L( "Disconnect from server." ) );
	disconnect.SetNameAndStatus( L( "GameUI_GameMenu_Disconnect" ), NULL );
	disconnect.SetPicture( PC_DISCONNECT );
	disconnect.iFlags |= QMF_NOTIFY;
	// disconnect.charSize = 6;
	// disconnect.SetTextHeight( 10 );
	// disconnect.SetTextHeight( 8 );
	disconnect.SetTextHeight( 6 );
	disconnect.onReleased = VoidCb( &CMenuMain::DisconnectDialogCb );

	// multiPlayer.SetNameAndStatus( "New Multiplayer Game", L( "StringsList_198" ) );
	multiPlayer.SetNameAndStatus( "New Multiplayer Game", NULL );
	multiPlayer.SetPicture( PC_MULTIPLAYER );
	multiPlayer.iFlags |= QMF_NOTIFY;
	// multiPlayer.charSize = 6;
	// multiPlayer.SetTextHeight( 10 );
	// multiPlayer.SetTextHeight( 8 );
	multiPlayer.SetTextHeight( 6 );
	multiPlayer.onReleased = UI_MultiPlayer_Menu;

	// findServers.SetNameAndStatus( "Find Servers", L( "StringsList_198" ) );
	findServers.SetNameAndStatus( "Find Servers", NULL );
	findServers.SetPicture( PC_FIND );
	findServers.iFlags |= QMF_NOTIFY;
	// findServers.charSize = 6;
	// findServers.SetTextHeight( 10 );
	// findServers.SetTextHeight( 8 );
	findServers.SetTextHeight( 6 );
	findServers.onReleased = UI_InternetGames_Menu;

	// configuration.SetNameAndStatus( L( "GameUI_Options" ), L( "StringsList_193" ) );
	configuration.SetNameAndStatus( L( "GameUI_Options" ), NULL );
	configuration.SetPicture( PC_CONFIG );
	configuration.iFlags |= QMF_NOTIFY;
	// configuration.charSize = 6;
	// configuration.SetTextHeight( 10 );
	// configuration.SetTextHeight( 8 );
	configuration.SetTextHeight( 6 );
	configuration.onReleased = UI_Options_Menu;

	// quit.SetNameAndStatus( L( "GameUI_GameMenu_Quit" ), L( "GameUI_QuitConfirmationText" ) );
	quit.SetNameAndStatus( L( "GameUI_GameMenu_Quit" ), NULL );
	quit.SetPicture( PC_QUIT );
	quit.iFlags |= QMF_NOTIFY;
	// quit.charSize = 6;
	// quit.SetTextHeight( 10 );
	// quit.SetTextHeight( 8 );
	quit.SetTextHeight( 6 );
	quit.onReleased = VoidCb( &CMenuMain::QuitDialogCb );

	// Top-right window controls are disabled for the classic main menu layout.
	// quitButton.SetPicture( ART_CLOSEBTN_N, ART_CLOSEBTN_F, ART_CLOSEBTN_D );
	// quitButton.iFlags = QMF_MOUSEONLY;
	// quitButton.eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	// quitButton.onReleased = VoidCb( &CMenuMain::QuitDialogCb );

	// minimizeBtn.SetPicture( ART_MINIMIZE_N, ART_MINIMIZE_F, ART_MINIMIZE_D );
	// minimizeBtn.iFlags = QMF_MOUSEONLY;
	// minimizeBtn.eFocusAnimation = QM_HIGHLIGHTIFFOCUS;
	// minimizeBtn.onReleased.SetCommand( FALSE, "minimize\n" );

	if( FBitSet( gMenu.m_gameinfo.flags, GFL_ANIMATED_TITLE ))
	{
		if( animatedBanner.TryLoad())
			AddItem( animatedBanner );
	}
	else if( CMenuBackgroundBitmap::ShouldDrawLogoMovie( ))
	{
		AddItem( movieBanner );
	}

	dialog.Link( this );

	AddItem( banner );
	AddItem( console );
	AddItem( disconnect );
	AddItem( resumeGame );
	AddItem( multiPlayer );
	AddItem( findServers );
	AddItem( configuration );
	AddItem( quit );
	// AddItem( minimizeBtn );
	// AddItem( quitButton );
}

/*
=================
UI_Main_Init
=================
*/
void CMenuMain::VidInit( bool connected )
{
	int hoffset = ( 70 / 640.0 ) * 1024.0 - 30;

	// in original menu Previews is located at specific point
	int topY = ( 404 / 480.0 ) * 768.0 + 50;
	// int ygap = (( 404 - 373 ) / 480.0 ) * 768.0;
	int ygap = 36;

	// statically positioned items
	minimizeBtn.SetRect( uiStatic.width - 72, 13, 32, 32 );
	quitButton.SetRect( uiStatic.width - 36, 13, 32, 32 );

	// Match the classic CS ordering from top to bottom:
	// New Multiplayer Game, Find Servers, Options, Quit.
	quit.SetCoord( hoffset, topY );
	configuration.SetCoord( hoffset, topY - ygap );
	findServers.SetCoord( hoffset, topY - ygap * 2 );
	multiPlayer.SetCoord( hoffset, topY - ygap * 3 );

	if( connected )
	{
		disconnect.SetCoord( hoffset, topY - ygap * 4 );
		resumeGame.SetCoord( hoffset, topY - ygap * 5 );
	}

	// now figure out what's visible
	console.SetVisibility( false );
	resumeGame.SetVisibility( connected );
	disconnect.SetVisibility( connected );
	multiPlayer.SetVisibility( true );
	findServers.SetVisibility( true );
	configuration.SetVisibility( true );
}

void CMenuMain::_VidInit()
{
	VidInit( CL_IsActive() );
}

void CMenuMain::Think()
{
	// The classic CS main menu does not expose the developer console button here.
	// if( gpGlobals->developer )
	// {
	// 	if( !console.IsVisible( ))
	// 		console.Show();
	// }
	// else
	// {
	// 	if( console.IsVisible( ))
	// 		console.Hide();
	// }
	if( console.IsVisible( ))
		console.Hide();

	CMenuFramework::Think();
}

ADD_MENU( menu_main, CMenuMain, UI_Main_Menu );
