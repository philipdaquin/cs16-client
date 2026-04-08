#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include "counterstrikeviewport.h"

void VGUI2_CreateViewport();
void VGUI2_DestroyViewport();
CCounterStrikeViewport *VGUI2_GetViewport();
void VGUI2_ShowTeamMenu();
void VGUI2_ShowClassMenu(int menuType);
void VGUI2_ShowBuyMenu();
void VGUI2_ShowBuySubMenu(CCounterStrikeViewport::BuyMenuCategory_t category);
void VGUI2_HideAllGameMenus();
int VGUI2_GetLocalPlayerTeam();

#endif