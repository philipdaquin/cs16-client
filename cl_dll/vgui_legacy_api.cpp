#include "hud.h"
#include "cl_util.h"
#include "VGUI/vgui_legacy_api.h"

#include <string.h>

extern float g_flRoundTime;

void VGuiLegacy_ClientCmd(const char *command)
{
	if (command && command[0])
	{
		ClientCmd(command);
	}
}

void VGuiLegacy_ConsolePrint(const char *text)
{
	if (text)
	{
		ConsolePrint(text);
	}
}

int VGuiLegacy_GetScreenWidth()
{
	return ScreenWidth;
}

int VGuiLegacy_GetScreenHeight()
{
	return ScreenHeight;
}

int VGuiLegacy_GetCurrentMoney()
{
	if (gHUD.cscl_currentmoney)
	{
		return (int)gHUD.cscl_currentmoney->value;
	}

	return 0;
}

int VGuiLegacy_GetTeamNumber()
{
	return g_iTeamNumber;
}

int VGuiLegacy_GetObserverMode()
{
	return g_iUser1;
}

int VGuiLegacy_GetObserverTarget()
{
	return g_iUser2;
}

int VGuiLegacy_GetMaxPlayers()
{
	return gEngfuncs.GetMaxClients();
}

int VGuiLegacy_GetTeamFrags(int teamIndex)
{
	return g_TeamInfo[teamIndex].frags;
}

int VGuiLegacy_GetPlayerHealth(int playerIndex)
{
	return g_PlayerExtraInfo[playerIndex].health;
}

bool VGuiLegacy_GetAllowSpectators()
{
	return gHUD.m_Menu.m_bAllowSpec != false;
}

bool VGuiLegacy_GetPlayerName(int playerIndex, char *buffer, size_t bufferSize)
{
	if (!buffer || bufferSize == 0)
	{
		return false;
	}

	buffer[0] = '\0';

	hud_player_info_t info;
	memset(&info, 0, sizeof(info));
	GetPlayerInfo(playerIndex, &info);

	if (!info.name)
	{
		return false;
	}

	strncpy(buffer, info.name, bufferSize - 1);
	buffer[bufferSize - 1] = '\0';
	return true;
}

const char *VGuiLegacy_GetLevelName()
{
	return gEngfuncs.pfnGetLevelName();
}

float VGuiLegacy_GetRoundTime()
{
	return g_flRoundTime;
}

float VGuiLegacy_GetClientTime()
{
	return gEngfuncs.GetClientTime();
}
