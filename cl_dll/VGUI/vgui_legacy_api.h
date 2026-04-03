#pragma once

#include <stddef.h>

void VGuiLegacy_ClientCmd(const char *command);
void VGuiLegacy_ConsolePrint(const char *text);

int VGuiLegacy_GetScreenWidth();
int VGuiLegacy_GetScreenHeight();
int VGuiLegacy_GetCurrentMoney();
int VGuiLegacy_GetTeamNumber();
int VGuiLegacy_GetObserverMode();
int VGuiLegacy_GetObserverTarget();
int VGuiLegacy_GetMaxPlayers();
int VGuiLegacy_GetTeamFrags(int teamIndex);
int VGuiLegacy_GetPlayerHealth(int playerIndex);
bool VGuiLegacy_GetAllowSpectators();
bool VGuiLegacy_GetPlayerName(int playerIndex, char *buffer, size_t bufferSize);

const char *VGuiLegacy_GetLevelName();
float VGuiLegacy_GetRoundTime();
float VGuiLegacy_GetClientTime();
