/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// death notice
//
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <vgui/ISurface.h>
#include "../vgui2_support/vgui_controls/controls.h"
#include "draw_util.h"

float color[3];

struct DeathNoticeItem {
	char szKiller[MAX_PLAYER_NAME_LENGTH*2];
	char szVictim[MAX_PLAYER_NAME_LENGTH*2];
	int iId;	// the index number of the associated sprite
	bool bSuicide;
	bool bTeamKill;
	bool bNonPlayerKill;
	float flDisplayTime;
	float *KillerColor;
	float *VictimColor;
	int iHeadShotId;
};

#define MAX_DEATHNOTICES	4
static int DEATHNOTICE_DISPLAY_TIME = 6;

#define DEATHNOTICE_TOP		32
static const int DEATHNOTICE_ROW_HEIGHT = 24;
static const int DEATHNOTICE_RIGHT_MARGIN = 35;
static vgui2::HFont g_DeathNoticeFont = vgui2::INVALID_FONT;

static void EnsureDeathNoticeFont()
{
	if ( g_DeathNoticeFont != vgui2::INVALID_FONT )
		return;

	g_DeathNoticeFont = vgui2::surface()->CreateFont();
	if ( g_DeathNoticeFont != vgui2::INVALID_FONT )
	{
		vgui2::surface()->AddGlyphSetToFont( g_DeathNoticeFont, "Verdana", 20, 600, 0, 0,
			vgui2::ISurface::FONTFLAG_ANTIALIAS | vgui2::ISurface::FONTFLAG_DROPSHADOW, 0x0, 0xFFFF );
	}
}

static int DeathNoticeTextWidth( const char *text )
{
	EnsureDeathNoticeFont();

	if ( g_DeathNoticeFont == vgui2::INVALID_FONT )
	{
		int width = 0;
		int height = 0;
		VGUI2_Surface_DrawStringLen( text, &width, &height );
		return width;
	}

	uchar32 utf32[256];
	const int utf32Bytes = Q_UTF8ToUTF32( text, utf32, sizeof( utf32 ), STRINGCONVERT_REPLACE );
	const int len = utf32Bytes > 0 ? utf32Bytes / (int)sizeof( uchar32 ) - 1 : 0;
	if ( len <= 0 )
		return 0;

	int width = 0;
	int height = 0;
	vgui2::surface()->GetTextSize( g_DeathNoticeFont, utf32, width, height );
	return width;
}

static int DeathNoticeDrawText( int x, int y, const char *text, const float *color )
{
	const int r = color ? (int)( color[0] * 255.0f ) : 255;
	const int g = color ? (int)( color[1] * 255.0f ) : 255;
	const int b = color ? (int)( color[2] * 255.0f ) : 255;

	EnsureDeathNoticeFont();

	if ( g_DeathNoticeFont == vgui2::INVALID_FONT )
		return x + VGUI2_Surface_DrawConsoleString( x, y, text, (byte)r, (byte)g, (byte)b, 255 );

	uchar32 utf32[256];
	const int utf32Bytes = Q_UTF8ToUTF32( text, utf32, sizeof( utf32 ), STRINGCONVERT_REPLACE );
	const int len = utf32Bytes > 0 ? utf32Bytes / (int)sizeof( uchar32 ) - 1 : 0;
	if ( len <= 0 )
		return x;

	vgui2::surface()->DrawSetTextFont( g_DeathNoticeFont );
	vgui2::surface()->DrawSetTextColor( r, g, b, 255 );
	vgui2::surface()->DrawSetTextPos( x, y );
	vgui2::surface()->DrawPrintText( utf32, len );

	int width = 0;
	int height = 0;
	vgui2::surface()->GetTextSize( g_DeathNoticeFont, utf32, width, height );
	return x + width;
}

DeathNoticeItem rgDeathNoticeList[ MAX_DEATHNOTICES + 1 ];

int CHudDeathNotice :: Init( void )
{
	gHUD.AddHudElem( this );

	HOOK_MESSAGE( gHUD.m_DeathNotice, DeathMsg );

	hud_deathnotice_time = CVAR_CREATE( "hud_deathnotice_time", "6", FCVAR_ARCHIVE );
	m_iFlags = 0;

	return 1;
}


void CHudDeathNotice :: InitHUDData( void )
{
	memset( rgDeathNoticeList, 0, sizeof(rgDeathNoticeList) );
}


int CHudDeathNotice :: VidInit( void )
{
	EnsureDeathNoticeFont();

	m_HUD_d_skull = gHUD.GetSpriteIndex( "d_skull" );
	m_HUD_d_headshot = gHUD.GetSpriteIndex("d_headshot");

	return 1;
}

int CHudDeathNotice :: Draw( float flTime )
{
	int x, y, r, g, b, i;

	for( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;  // we've gone through them all

		if ( rgDeathNoticeList[i].flDisplayTime < flTime )
		{ // display time has expired
			// remove the current item from the list
			memmove( &rgDeathNoticeList[i], &rgDeathNoticeList[i+1], sizeof(DeathNoticeItem) * (MAX_DEATHNOTICES - i) );
			i--;  // continue on the next item;  stop the counter getting incremented
			continue;
		}

		rgDeathNoticeList[i].flDisplayTime = min( rgDeathNoticeList[i].flDisplayTime, flTime + DEATHNOTICE_DISPLAY_TIME );

		// Hide when scoreboard drawing. It will break triapi
		//if ( gViewPort && gViewPort->AllowedToPrintText() )
		//if ( !gHUD.m_iNoConsolePrint )
		{
			// Draw the death notice
			if( !g_iUser1 )
			{
				y = YRES(DEATHNOTICE_TOP) + 2 + (DEATHNOTICE_ROW_HEIGHT * i);  //!!!
			}
			else
			{
				y = ScreenHeight / 5 + 2 + (DEATHNOTICE_ROW_HEIGHT * i);
			}

			int id = (rgDeathNoticeList[i].iId == -1) ? m_HUD_d_skull : rgDeathNoticeList[i].iId;
			x = ScreenWidth - DEATHNOTICE_RIGHT_MARGIN - DeathNoticeTextWidth( rgDeathNoticeList[i].szVictim ) - (gHUD.GetSpriteRect(id).Width());
			if( rgDeathNoticeList[i].iHeadShotId )
				x -= gHUD.GetSpriteRect(m_HUD_d_headshot).Width();

			if ( !rgDeathNoticeList[i].bSuicide )
			{
				x -= (5 + DeathNoticeTextWidth( rgDeathNoticeList[i].szKiller ) );
				x = 5 + DeathNoticeDrawText( x, y, rgDeathNoticeList[i].szKiller, rgDeathNoticeList[i].KillerColor );
			}

			r = 255;  g = 80;	b = 0;
			if ( rgDeathNoticeList[i].bTeamKill )
			{
				r = 10;	g = 240; b = 10;  // display it in sickly green
			}

			// Draw death weapon
			SPR_Set( gHUD.GetSprite(id), r, g, b );
			SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(id) );

			x += (gHUD.GetSpriteRect(id).Width());

			if( rgDeathNoticeList[i].iHeadShotId)
			{
				SPR_Set( gHUD.GetSprite(m_HUD_d_headshot), r, g, b );
				SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect(m_HUD_d_headshot));
				x += (gHUD.GetSpriteRect(m_HUD_d_headshot).Width());
			}

			// Draw victims name (if it was a player that was killed)
			if (!rgDeathNoticeList[i].bNonPlayerKill)
			{
				x = DeathNoticeDrawText( x, y, rgDeathNoticeList[i].szVictim, rgDeathNoticeList[i].VictimColor );
			}
		}
	}

	if( i == 0 )
		m_iFlags &= ~HUD_DRAW; // disable hud item

	return 1;
}

// This message handler may be better off elsewhere
int CHudDeathNotice :: MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_DRAW;

	BufferReader reader( pszName, pbuf, iSize );

	int killer = reader.ReadByte();
	int victim = reader.ReadByte();
	int headshot = reader.ReadByte();

	char killedwith[32];
	strncpy( killedwith, "d_", sizeof(killedwith) );
	strncat( killedwith, reader.ReadString(), sizeof( killedwith ) - 2 );

	//if (gViewPort)
	//	gViewPort->DeathMsg( killer, victim );
	gHUD.m_Scoreboard.DeathMsg( killer, victim );

	gHUD.m_Spectator.DeathMessage(victim);
	int i;
	for ( i = 0; i < MAX_DEATHNOTICES; i++ )
	{
		if ( rgDeathNoticeList[i].iId == 0 )
			break;
	}
	if ( i == MAX_DEATHNOTICES )
	{ // move the rest of the list forward to make room for this item
		memmove( rgDeathNoticeList, rgDeathNoticeList+1, sizeof(DeathNoticeItem) * MAX_DEATHNOTICES );
		i = MAX_DEATHNOTICES - 1;
	}

	//if (gViewPort)
		//gViewPort->GetAllPlayersInfo();
	gHUD.m_Scoreboard.GetAllPlayersInfo();

	// Get the Killer's name
	const char *killer_name = g_PlayerInfoList[ killer ].name;
	if ( !killer_name )
	{
		killer_name = "";
		rgDeathNoticeList[i].szKiller[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].KillerColor = GetClientColor( killer );
		strncpy( rgDeathNoticeList[i].szKiller, killer_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szKiller[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Get the Victim's name
	const char *victim_name = NULL;
	// If victim is -1, the killer killed a specific, non-player object (like a sentrygun)
	if ( ((char)victim) != -1 )
		victim_name = g_PlayerInfoList[ victim ].name;
	if ( !victim_name )
	{
		victim_name = "";
		rgDeathNoticeList[i].szVictim[0] = 0;
	}
	else
	{
		rgDeathNoticeList[i].VictimColor = GetClientColor( victim );
		strncpy( rgDeathNoticeList[i].szVictim, victim_name, MAX_PLAYER_NAME_LENGTH );
		rgDeathNoticeList[i].szVictim[MAX_PLAYER_NAME_LENGTH-1] = 0;
	}

	// Is it a non-player object kill?
	if ( ((char)victim) == -1 )
	{
		rgDeathNoticeList[i].bNonPlayerKill = true;

		// Store the object's name in the Victim slot (skip the d_ bit)
		strncpy( rgDeathNoticeList[i].szVictim, killedwith+2, sizeof(killedwith) );
	}
	else
	{
		if ( killer == victim || killer == 0 )
			rgDeathNoticeList[i].bSuicide = true;

		if ( !strncmp( killedwith, "d_teammate", sizeof(killedwith)  ) )
			rgDeathNoticeList[i].bTeamKill = true;
	}

	rgDeathNoticeList[i].iHeadShotId = headshot;

	// Find the sprite in the list
	int spr = gHUD.GetSpriteIndex( killedwith );

	rgDeathNoticeList[i].iId = spr;

	rgDeathNoticeList[i].flDisplayTime = gHUD.m_flTime + hud_deathnotice_time->value;


	if (rgDeathNoticeList[i].bNonPlayerKill)
	{
		ConsolePrint( rgDeathNoticeList[i].szKiller );
		ConsolePrint( " killed a " );
		ConsolePrint( rgDeathNoticeList[i].szVictim );
		ConsolePrint( "\n" );
	}
	else
	{
		// record the death notice in the console
		if ( rgDeathNoticeList[i].bSuicide )
		{
			ConsolePrint( rgDeathNoticeList[i].szVictim );

			if ( !strncmp( killedwith, "d_world", sizeof(killedwith)  ) )
			{
				ConsolePrint( " died" );
			}
			else
			{
				ConsolePrint( " killed self" );
			}
		}
		else if ( rgDeathNoticeList[i].bTeamKill )
		{
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed his teammate " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}
		else
		{
			if( headshot )
				ConsolePrint( "*** ");
			ConsolePrint( rgDeathNoticeList[i].szKiller );
			ConsolePrint( " killed " );
			ConsolePrint( rgDeathNoticeList[i].szVictim );
		}

		if ( *killedwith && (*killedwith > 13 ) && strncmp( killedwith, "d_world", sizeof(killedwith) ) && !rgDeathNoticeList[i].bTeamKill )
		{
			if ( headshot )
				ConsolePrint(" with a headshot from ");
			else
				ConsolePrint(" with ");

			ConsolePrint( killedwith+2 ); // skip over the "d_" part
		}

		if( headshot ) ConsolePrint( " ***");
		ConsolePrint( "\n" );
	}

	return 1;
}
