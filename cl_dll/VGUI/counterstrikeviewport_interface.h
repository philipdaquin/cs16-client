#pragma once

void VGUI2_CreateViewport();
void VGUI2_DestroyViewport();
bool VGUI2_HasViewport();
void VGUI2_ShowTeamMenu();
void VGUI2_ShowClassMenu(int menuType);
void VGUI2_ShowBuyMenu();
void VGUI2_ShowBuySubMenu(int category);
void VGUI2_HideAllGameMenus();
int VGUI2_GetLocalPlayerTeam();
void VGUI2_RunClientCommand(const char *command);

enum VGUI2BuyMenuCategory
{
	VGUI2_BUYMENU_PISTOL = 0,
	VGUI2_BUYMENU_SHOTGUN,
	VGUI2_BUYMENU_SUBMACHINEGUN,
	VGUI2_BUYMENU_RIFLE,
	VGUI2_BUYMENU_MACHINEGUN,
	VGUI2_BUYMENU_ITEM
};
