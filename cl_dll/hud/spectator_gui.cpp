/*
spectator_gui.cpp - HUD Overlays
Copyright (C) 2015 a1batross

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

In addition, as a special exception, the author gives permission to
link the code of this program with the Half-Life Game Engine ("HL
Engine") and Modified Game Libraries ("MODs") developed by Valve,
L.L.C ("Valve").  You must obey the GNU General Public License in all
respects for all of the code used other than the HL Engine and MODs
from Valve.  If you modify this file, you may extend this exception
to your version of the file, but you are not obligated to do so.  If
you do not wish to do so, delete this exception statement from your
version.
*/

#include <string.h>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "vgui_parser.h"
#include "triangleapi.h"
#include "draw_util.h"

/*
 * We will draw all elements inside a box. It's size 16x10.
 */

#define XPOS( x ) ( (x) / 16.0f )
#define YPOS( y ) ( (y) / 10.0f  )

#define INT_XPOS(x) int(XPOS(x) * ScreenWidth)
#define INT_YPOS(y) int(YPOS(y) * ScreenHeight)

static inline int ResX( int x )
{
	return ( x * ScreenWidth ) / 640;
}

static inline int ResY( int y )
{
	return ( y * ScreenHeight ) / 480;
}

int CHudSpectatorGui::Init()
{
	if( !g_iXash )
		return 1;

	HOOK_MESSAGE( gHUD.m_SpectatorGui, SpecHealth );
	HOOK_MESSAGE( gHUD.m_SpectatorGui, SpecHealth2 );

	HOOK_COMMAND( gHUD.m_SpectatorGui, "_spec_toggle_menu", ToggleSpectatorMenu );
	HOOK_COMMAND( gHUD.m_SpectatorGui, "_spec_toggle_menu_options", ToggleSpectatorMenuOptions );
	// close
	// help
	// settings
	// pip
	// autodirector
	// showscores

	HOOK_COMMAND( gHUD.m_SpectatorGui, "_spec_toggle_menu_options_settings", ToggleSpectatorMenuOptionsSettings );
	// settings
	// // chat msgs
	// // show status
	// // view cone
	// // player names

	HOOK_COMMAND( gHUD.m_SpectatorGui, "_spec_toggle_menu_spectate_options", ToggleSpectatorMenuSpectateOptions );
	// chase map overview
	// free map overview
	// first person
	// free look
	// free chase camera
	// locked chase camera

	HOOK_COMMAND_FUNC( "_spec_find_next_player_reverse", gHUD.m_Spectator.FindNextPlayer, true );
	HOOK_COMMAND_FUNC( "_spec_find_next_player", gHUD.m_Spectator.FindNextPlayer, false );

	gHUD.AddHudElem(this);
	m_iFlags = HUD_DRAW;
	m_menuFlags = 0;
	m_hTimerTexture = 0;
	return 1;
}

int CHudSpectatorGui::VidInit()
{
	if( !g_iXash )
	{
		ConsolePrint("Warning: CHudSpectatorGui is disabled! Dude, are you running me on old GoldSrc?\n");
		m_iFlags = 0;
		return 0;
	}

	m_hTimerTexture = gRenderAPI.GL_LoadTexture("gfx/vgui/timer.tga", NULL, 0, TF_NEAREST |TF_NOMIPMAP|TF_CLAMP );
	return 1;
}

void CHudSpectatorGui::Shutdown()
{
	gRenderAPI.GL_FreeTexture( m_hTimerTexture );
}

inline void DrawButtonWithText( int x1, int y1, int wide, int tall, const char *sz, int r, int g, int b )
{
	DrawUtils::DrawRectangle(x1, y1, wide, tall);
	DrawUtils::DrawHudString(x1 + INT_XPOS(0.5), y1 + tall*0.5 - gHUD.GetCharHeight() * 0.5, x1 + wide, sz,
							 r, g, b );
}

int CHudSpectatorGui::Draw( float flTime )
{
	if( !g_iUser1 )
	{
		if( m_menuFlags & ROOT_MENU )
		{
			UserCmd_ToggleSpectatorMenu(); // this will remove any submenus;
			m_menuFlags = 0;
		}
		return 1;
	}

	// function name says it
	CalcAllNeededData( );

	// Spectator GUI scheme values from the resource design, applied directly in HUD code.
	const int whiteR = 255;
	const int whiteG = 255;
	const int whiteB = 255;
	const int barAlpha = 153;

	const int topBarH = ResY( 52 );
	const int bottomBarY = ResY( 429 );
	const int bottomBarH = ResY( 52 );
	const int extrainfoLeft = ScreenWidth - ResX( 56 );
	const int extrainfoRight = extrainfoLeft + ResX( 44 );
	const int timerLeft = ScreenWidth - ResX( 42 );
	const int timerRight = timerLeft + ResX( 40 );
	const int timerImageLeft = ScreenWidth - ResX( 56 );
	const int timerImageSize = ResY( 12 );
	const int timerImageRight = timerImageLeft + timerImageSize;
	const int ctLabelLeft = ScreenWidth - ResX( 233 );
	const int ctLabelRight = ctLabelLeft + ResX( 150 );
	const int ctValueLeft = ScreenWidth - ResX( 80 );
	const int ctValueRight = ctValueLeft + ResX( 10 );
	const int terLabelLeft = ctLabelLeft;
	const int terLabelRight = ctLabelRight;
	const int terValueLeft = ctValueLeft;
	const int terValueRight = ctValueRight;
	const int playerLabelLeft = ( ScreenWidth - ResX( 216 ) ) / 2;
	const int playerLabelRight = playerLabelLeft + ResX( 216 );

	// Top and bottom translucent bars.
	FillRGBABlend( 0, 0, ScreenWidth, topBarH, 0, 0, 0, barAlpha );
	FillRGBABlend( 0, bottomBarY, ScreenWidth, bottomBarH, 0, 0, 0, barAlpha );
	// "bottombarblank" from the design is represented here as a transparent overlay region.
	FillRGBABlend( 0, bottomBarY, ScreenWidth, bottomBarH, 0, 0, 0, 0 );

	// DividerBar
	FillRGBABlend( ScreenWidth - ResX( 64 ), ResY( 12 ), 1, ResY( 30 ), whiteR, whiteG, whiteB, 255 );

	// Extrainfo and timer area
	DrawUtils::DrawHudString( extrainfoLeft, ResY( 12 ), extrainfoRight, label.m_szMap, whiteR, whiteG, whiteB );
	if( !m_bBombPlanted )
	{
		if( m_hTimerTexture )
		{
			gRenderAPI.GL_SelectTexture( 0 );
			gRenderAPI.GL_Bind( 0, m_hTimerTexture );
			gEngfuncs.pTriAPI->RenderMode( kRenderTransAlpha );
			DrawUtils::Draw2DQuad( timerImageLeft, ResY( 26 ), timerImageRight, ResY( 26 ) + timerImageSize );
		}
		DrawUtils::DrawHudString( timerLeft - 15, ResY( 24 ) + 15, timerRight, label.m_szTimer, whiteR, whiteG, whiteB );
	}

	// CT/Terrorist score block
	DrawUtils::DrawHudStringReverse( ctLabelRight, ResY( 12 ), ctLabelLeft,
									 Localize( "#Cstrike_Spec_CT_Score" ), whiteR, whiteG, whiteB );
	{
		char szScore[16];
		snprintf( szScore, sizeof( szScore ), "%d", label.m_iCounterTerrorists );
		DrawUtils::DrawHudString( ctValueLeft, ResY( 12 ), ctValueRight, szScore, whiteR, whiteG, whiteB );
	}
	DrawUtils::DrawHudStringReverse( terLabelRight, ResY( 24 ), terLabelLeft,
									 Localize( "#Cstrike_Spec_Ter_Score" ), whiteR, whiteG, whiteB );
	{
		char szScore[16];
		snprintf( szScore, sizeof( szScore ), "%d", label.m_iTerrorists );
		DrawUtils::DrawHudString( terValueLeft, ResY( 24 ), terValueRight, szScore, whiteR, whiteG, whiteB );
	}

	// Temporarily disabled for Counter Strike Xtreme and Cs1.6 design style guide
	// if( m_menuFlags & ROOT_MENU )
	// {
	// 	const int r = whiteR;
	// 	const int g = whiteG;
	// 	const int b = whiteB;
	// 	// draw the root menu
	// 	DrawButtonWithText(INT_XPOS(0.5),  INT_YPOS(8.5), INT_XPOS(4), INT_YPOS(1), "Options", r, g, b);
	// 	DrawButtonWithText(INT_XPOS(5),    INT_YPOS(8.5), INT_XPOS(1), INT_YPOS(1), "<", r, g, b);

	// 	DrawUtils::DrawRectangle(INT_XPOS(6), INT_YPOS(8.5), INT_XPOS(4), INT_YPOS(1));
	// 	// name will be drawn later

	// 	DrawButtonWithText(INT_XPOS(10),   INT_YPOS(8.5), INT_XPOS(1), INT_YPOS(1), ">", r, g, b );
	// 	DrawButtonWithText(INT_XPOS(11.5), INT_YPOS(8.5), INT_XPOS(4), INT_YPOS(1), "Spectate Options", r, g, b);
	// 	if( m_menuFlags & MENU_OPTIONS )
	// 	{
	// 		DrawButtonWithText(INT_XPOS(0.5), INT_YPOS(2.5), INT_XPOS(4), INT_YPOS(1), "Close", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(0.5), INT_YPOS(3.5), INT_XPOS(4), INT_YPOS(1), "Help", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(0.5), INT_YPOS(4.5), INT_XPOS(4), INT_YPOS(1), "Settings", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(0.5), INT_YPOS(5.5), INT_XPOS(4), INT_YPOS(1), "Picture-in-Picture", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(0.5), INT_YPOS(6.5), INT_XPOS(4), INT_YPOS(1), "Autodirector", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(0.5), INT_YPOS(7.5), INT_XPOS(4), INT_YPOS(1), "Show scores", r, g, b );
	// 		if( m_menuFlags & MENU_OPTIONS_SETTINGS )
	// 		{
	// 			DrawButtonWithText(INT_XPOS(4.5), INT_YPOS(4.5), INT_XPOS(4), INT_YPOS(1), "Chat messages", r, g, b );
	// 			DrawButtonWithText(INT_XPOS(4.5), INT_YPOS(5.5), INT_XPOS(4), INT_YPOS(1), "Show status", r, g, b );
	// 			DrawButtonWithText(INT_XPOS(4.5), INT_YPOS(6.5), INT_XPOS(4), INT_YPOS(1), "View cone", r, g, b );
	// 			DrawButtonWithText(INT_XPOS(4.5), INT_YPOS(7.5), INT_XPOS(4), INT_YPOS(1), "Player names", r, g, b );
	// 		}
	// 	}

	// 	if( m_menuFlags & MENU_SPEC_OPTIONS )
	// 	{
	// 		DrawButtonWithText(INT_XPOS(11.5), INT_YPOS(2.5), INT_XPOS(4), INT_YPOS(1), "Chase Map Overview", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(11.5), INT_YPOS(3.5), INT_XPOS(4), INT_YPOS(1), "Free Map Overview", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(11.5), INT_YPOS(4.5), INT_XPOS(4), INT_YPOS(1), "First Person", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(11.5), INT_YPOS(5.5), INT_XPOS(4), INT_YPOS(1), "Free look", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(11.5), INT_YPOS(6.5), INT_XPOS(4), INT_YPOS(1), "Free Chase Camera", r, g, b );
	// 		DrawButtonWithText(INT_XPOS(11.5), INT_YPOS(7.5), INT_XPOS(4), INT_YPOS(1), "Locked Chase Camera", r, g, b );
	// 	}
	// }

	//if( !label.m_szNameAndHealth[0] )
	//{
		if( label.m_szNameAndHealth[0] )
		{
			int playerLabelWidth = DrawUtils::HudStringLen( label.m_szNameAndHealth );
			int playerLabelX = ( ScreenWidth - playerLabelWidth ) / 2;
			int r = whiteR, g = whiteG, b = whiteB;
			if( g_iUser2 > 0 && g_iUser2 < MAX_PLAYERS )
				GetTeamColor( r, g, b, g_PlayerExtraInfo[ g_iUser2 ].teamnumber );
			DrawUtils::DrawHudString( playerLabelX, ResY( 441 ), playerLabelRight,
									  label.m_szNameAndHealth, r, g, b );
		}
	//}

	return 1;
}

void CHudSpectatorGui::CalcAllNeededData( )
{
	// mapname
	if( !label.m_szMap[0] )
	{
		static char szMapNameStripped[55];
		const char *szMapName = gEngfuncs.pfnGetLevelName(); //  "maps/%s.bsp"
		strncpy( szMapNameStripped, szMapName + 5, sizeof( szMapNameStripped ) );
		szMapNameStripped[strlen(szMapNameStripped) - 4] = '\0';
		snprintf( label.m_szMap, sizeof( label.m_szMap ), "Map: %s", szMapNameStripped );
	}

	// team
	/*label.m_iTerrorists        = 0;
	label.m_iCounterTerrorists = 0;
	for( int i = 0; i < MAX_PLAYERS; i++ )
	{
		if( g_PlayerExtraInfo[i].dead )
			continue; // show remaining

		switch( g_PlayerExtraInfo[i].teamnumber )
		{
		case TEAM_CT:
			label.m_iCounterTerrorists++;
		case TEAM_TERRORIST:
			label.m_iTerrorists++;
		}
	}*/

	label.m_iCounterTerrorists = 0;
	label.m_iTerrorists = 0;
	for( int i = 1; i <= gHUD.m_Scoreboard.m_iNumTeams; i++ )
	{
		switch( g_TeamInfo[i].teamnumber )
		{
		case TEAM_CT:
			label.m_iCounterTerrorists = g_TeamInfo[i].frags;
			break;
		case TEAM_TERRORIST:
			label.m_iTerrorists = g_TeamInfo[i].frags;
			break;
		}
	}

	// timer
	// time must be positive
	if( !m_bBombPlanted )
	{
		int iMinutes = max( 0, (int)( gHUD.m_Timer.m_iTime + gHUD.m_Timer.m_fStartTime - gHUD.m_flTime ) / 60);
		int iSeconds = max( 0, (int)( gHUD.m_Timer.m_iTime + gHUD.m_Timer.m_fStartTime - gHUD.m_flTime ) - (iMinutes * 60));

		sprintf( label.m_szTimer, "%i:%02i", iMinutes, iSeconds );
	}

	// player name
	if( g_iUser2 > 0 && g_iUser2 < MAX_PLAYERS )
	{
		hud_player_info_t sInfo;
		GetPlayerInfo( g_iUser2, &sInfo );

		snprintf( label.m_szNameAndHealth, sizeof( label.m_szNameAndHealth ),
				  "%s (%i)",  sInfo.name, g_PlayerExtraInfo[g_iUser2].health );
	}
	else label.m_szNameAndHealth[0] = '\0';
}

void CHudSpectatorGui::InitHUDData()
{
	m_bBombPlanted = false;
	label.m_szMap[0] = '\0';
}

void CHudSpectatorGui::Reset()
{
	m_bBombPlanted = false;
	if( m_menuFlags & ROOT_MENU )
	{
		UserCmd_ToggleSpectatorMenu(); // this will remove any submenus;
		m_menuFlags = 0;
	}
}

int CHudSpectatorGui::MsgFunc_SpecHealth(const char *pszName, int iSize, void *buf)
{
	BufferReader reader( pszName, buf, iSize );

	int health = reader.ReadByte();

	g_PlayerExtraInfo[g_iUser2].health = health;
	gHUD.m_Health.m_iPlayerLastPointedAt = g_iUser2;

	return 1;
}

int CHudSpectatorGui::MsgFunc_SpecHealth2(const char *pszName, int iSize, void *buf)
{
	BufferReader reader( pszName, buf, iSize );

	int health = reader.ReadByte();
	int client = reader.ReadByte();

	g_PlayerExtraInfo[client].health = health;
	gHUD.m_Health.m_iPlayerLastPointedAt = g_iUser2;

	return 1;
}

#define PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y(x, y) XPOS(x), YPOS(y), XPOS(x + 4.0f), YPOS(y + 1.0f)

void CHudSpectatorGui::UserCmd_ToggleSpectatorMenu()
{
	static byte color[4] = {0, 0, 0, 0};

	if( !g_iMobileAPIVersion )
		return;

	gMobileAPI.pfnTouchSetClientOnly( !(m_menuFlags & ROOT_MENU) );

	if( !(m_menuFlags & ROOT_MENU) )
	{
		m_menuFlags |= ROOT_MENU;

		gMobileAPI.pfnTouchAddClientButton( "_spec_menu_options", "*white", "_spec_toggle_menu_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 0.5f, 8.5f ), color, 0, 1.0f, 0 );

		gMobileAPI.pfnTouchAddClientButton( "_spec_menu_find_next_player_reverse", "*white", "_spec_find_next_player_reverse",
			XPOS(5.0f), YPOS(8.5f), XPOS(6.0f), YPOS(9.5f), color, 0, 1.0f, 0 );

		gMobileAPI.pfnTouchAddClientButton( "_spec_menu_find_next_player", "*white", "_spec_find_next_player",
			XPOS(10.0f),YPOS(8.5f), XPOS(11.0f),YPOS(9.5f), color, 0, 1.0f, 0 );

		gMobileAPI.pfnTouchAddClientButton( "_spec_menu_spectate_options", "*white", "_spec_toggle_menu_spectate_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 11.5f, 8.5f ),color, 0, 1.0f, 0 );
	}
	else
	{
		m_menuFlags &= ~ROOT_MENU;
		m_menuFlags &= ~MENU_OPTIONS;
		m_menuFlags &= ~MENU_OPTIONS_SETTINGS;
		m_menuFlags &= ~MENU_SPEC_OPTIONS;
		gMobileAPI.pfnTouchRemoveButton( "_spec_*" );
	}
}

void CHudSpectatorGui::UserCmd_ToggleSpectatorMenuOptions()
{
	static byte color[4] = {0, 0, 0, 0};

	if( !(m_menuFlags & ROOT_MENU) || !g_iMobileAPIVersion )
		return;

	if( !(m_menuFlags & MENU_OPTIONS) )
	{
		m_menuFlags |= MENU_OPTIONS;
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_close", "*white", "_spec_toggle_menu",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 0.5f, 2.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_help", "*white", "spec_help; _spec_toggle_menu_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 0.5f, 3.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_settings", "*white", "_spec_toggle_menu_options_settings",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 0.5f, 4.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_pip", "*white", "spec_pip t; _spec_toggle_menu_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 0.5f, 5.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_ad", "*white", "spec_autodirector t; _spec_toggle_menu_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 0.5f, 6.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_showscores", "*white", "_spec_toggle_menu_options; scoreboard",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 0.5f, 7.5f ), color, 0, 1.0f, 0 );
	}
	else
	{
		m_menuFlags &= ~MENU_OPTIONS;
		m_menuFlags &= ~MENU_OPTIONS_SETTINGS;
		gMobileAPI.pfnTouchRemoveButton( "_spec_opt_*" );
	}
}

void CHudSpectatorGui::UserCmd_ToggleSpectatorMenuOptionsSettings()
{
	static byte color[4] = {0, 0, 0, 0};

	if( !(m_menuFlags & ROOT_MENU) || !g_iMobileAPIVersion )
		return;

	if( !(m_menuFlags & MENU_OPTIONS_SETTINGS) )
	{
		m_menuFlags |= MENU_OPTIONS_SETTINGS;
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_chat_msgs", "*white", "messagemode; _spec_toggle_menu_options_settings",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 4.5f, 4.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_set_status", "*white", "spec_drawstatus t; _spec_toggle_menu_options_settings",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 4.5f, 5.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_draw_cones", "*white", "spec_drawcone t; _spec_toggle_menu_options_settings",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 4.5f, 6.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_opt_draw_names", "*white", "spec_drawnames t; _spec_toggle_menu_options_settings",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 4.5f, 7.5f ), color, 0, 1.0f, 0 );
	}
	else
	{
		m_menuFlags &= ~MENU_OPTIONS_SETTINGS;
		gMobileAPI.pfnTouchRemoveButton( "_spec_opt_set_*" );
	}
}

void CHudSpectatorGui::UserCmd_ToggleSpectatorMenuSpectateOptions()
{
	static byte color[4] = {0, 0, 0, 0};

	if( !(m_menuFlags & ROOT_MENU) || !g_iMobileAPIVersion )
		return;

	if( !(m_menuFlags & MENU_SPEC_OPTIONS) )
	{
		m_menuFlags |= MENU_SPEC_OPTIONS;
		gMobileAPI.pfnTouchAddClientButton( "_spec_spec_6", "*white", "spec_mode 6; _spec_toggle_menu_spectate_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 11.5f, 2.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_spec_5", "*white", "spec_mode 5; _spec_toggle_menu_spectate_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 11.5f, 3.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_spec_4", "*white", "spec_mode 4; _spec_toggle_menu_spectate_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 11.5f, 4.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_spec_3", "*white", "spec_mode 3; _spec_toggle_menu_spectate_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 11.5f, 5.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_spec_2", "*white", "spec_mode 2; _spec_toggle_menu_spectate_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 11.5f, 6.5f ), color, 0, 1.0f, 0 );
		gMobileAPI.pfnTouchAddClientButton( "_spec_spec_1", "*white", "spec_mode 1; _spec_toggle_menu_spectate_options",
			PLACE_DEFAULT_SIZE_BUTTON_AT_X_Y( 11.5f, 7.5f ), color, 0, 1.0f, 0 );
	}
	else
	{
		m_menuFlags &= ~MENU_SPEC_OPTIONS;
		gMobileAPI.pfnTouchRemoveButton( "_spec_spec_*" );
	}
}
