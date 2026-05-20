#include "counterstrikeviewport_interface.h"

#include "CHudViewPort.h"
#include "CBaseViewport.h"
#include "game_controls/classmenu.h"
#include "game_controls/buymenu.h"
#include "game_controls/teammenu.h"
#include "hud.h"
#include "keydefs.h"
#include "../../vgui2_support/vgui_controls/controls.h"
#include "vgui2/vgui_key_translation.h"
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>
#include <vgui/KeyCode.h>
#include <vgui/MouseCode.h>

static CBuyMenu *VGUI2_GetActiveBuyMenu()
{
	if (!g_pViewport)
		return nullptr;

	CBuyMenu *buyMenu = dynamic_cast<CBuyMenu *>(g_pViewport->GetActivePanel());

	if (!buyMenu || !buyMenu->IsVisible())
		return nullptr;

	return buyMenu;
}

static vgui2::Panel *VGUI2_GetActiveModalGameMenu()
{
	if (!g_pViewport)
		return nullptr;

	IViewportPanel *activePanel = g_pViewport->GetActivePanel();

	if (!activePanel)
		return nullptr;

	if (CBuyMenu *buyMenu = dynamic_cast<CBuyMenu *>(activePanel))
		return buyMenu->IsVisible() ? buyMenu : nullptr;

	if (CTeamMenu *teamMenu = dynamic_cast<CTeamMenu *>(activePanel))
		return teamMenu->IsVisible() ? teamMenu : nullptr;

	if (CClassMenu *classMenu = dynamic_cast<CClassMenu *>(activePanel))
		return classMenu->IsVisible() ? classMenu : nullptr;

	return nullptr;
}

static vgui2::Panel *VGUI2_GetModalGameMenuFocusPanel(vgui2::Panel *menu)
{
	if (CBuyMenu *buyMenu = dynamic_cast<CBuyMenu *>(menu))
		return buyMenu->GetInputFocusPanel();

	return menu;
}

static bool VGUI2_GetMouseCodeForKey(int keynum, vgui2::MouseCode &mouseCode)
{
	switch (keynum)
	{
	case K_MOUSE1:
		mouseCode = vgui2::MOUSE_LEFT;
		return true;
	case K_MOUSE2:
		mouseCode = vgui2::MOUSE_RIGHT;
		return true;
	case K_MOUSE3:
		mouseCode = vgui2::MOUSE_MIDDLE;
		return true;
	case K_MOUSE4:
		mouseCode = vgui2::MOUSE_4;
		return true;
	case K_MOUSE5:
		mouseCode = vgui2::MOUSE_5;
		return true;
	default:
		return false;
	}
}

static bool VGUI2_GetWheelDeltaForKey(int keynum, int &delta)
{
	if (keynum == K_MWHEELUP)
	{
		delta = 1;
		return true;
	}

	if (keynum == K_MWHEELDOWN)
	{
		delta = -1;
		return true;
	}

	return false;
}

bool VGUI2_HasViewport()
{
	return g_pViewport != nullptr;
}

void *VGUI2_GetViewportPtr()
{
	return g_pViewport;
}

void VGUI2_CreateViewport()
{
}

void VGUI2_DestroyViewport()
{
}

void VGUI2_ShowTeamMenu()
{
	if (g_pViewport)
		static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(MENU_TEAM);
}

void VGUI2_ShowClassMenu(int menuType)
{
	if (g_pViewport)
		static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(menuType);
}

void VGUI2_ShowBuyMenu()
{
	if (g_pViewport)
		static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(MENU_BUY);
}

void VGUI2_ShowBuySubMenu(int category)
{
	if (!g_pViewport)
		return;

	static const int kMenuTypes[] =
	{
		MENU_BUY_PISTOL,
		MENU_BUY_SHOTGUN,
		MENU_BUY_SUBMACHINEGUN,
		MENU_BUY_RIFLE,
		MENU_BUY_MACHINEGUN,
		MENU_BUY_ITEM
	};

	if (category < 0 || category >= static_cast<int>(sizeof(kMenuTypes) / sizeof(kMenuTypes[0])))
		return;

	static_cast<CHudViewport *>(g_pViewport)->ShowVGUIMenu(kMenuTypes[category]);
}

void VGUI2_HideAllGameMenus()
{
	if (g_pViewport)
		g_pViewport->HideAllVGUIMenu();
}

bool VGUI2_ShouldCaptureInput()
{
	return g_pViewport && (g_pViewport->GetActivePanel() != nullptr || g_pViewport->IsBackGroundVisible());
}

bool VGUI2_IsModalBuyInputActive()
{
	return VGUI2_GetActiveBuyMenu() != nullptr;
}

bool VGUI2_IsModalGameMenuInputActive()
{
	return VGUI2_GetActiveModalGameMenu() != nullptr;
}

bool VGUI2_HandleModalGameMenuInput(int down, int keynum)
{
	vgui2::Panel *menu = VGUI2_GetActiveModalGameMenu();

	if (!menu)
		return false;

	vgui2::IInputInternal *input = vgui2::input();

	if (!input)
		return true;

	input->SetAppModalSurface(menu->GetVPanel());

	vgui2::Panel *focusPanel = VGUI2_GetModalGameMenuFocusPanel(menu);

	if (focusPanel)
		focusPanel->RequestFocus();

	vgui2::MouseCode mouseCode = vgui2::MOUSE_LEFT;

	if (VGUI2_GetMouseCodeForKey(keynum, mouseCode))
	{
		if (down)
			input->InternalMousePressed(mouseCode);
		else
			input->InternalMouseReleased(mouseCode);

		return true;
	}

	int wheelDelta = 0;

	if (down && VGUI2_GetWheelDeltaForKey(keynum, wheelDelta))
	{
		input->InternalMouseWheeled(wheelDelta);
		return true;
	}

	vgui2::KeyCode keyCode = KeyCode_EngineKeyToVGUI(keynum);

	if (keyCode != vgui2::KEY_NONE)
	{
		if (down)
		{
			input->InternalKeyCodePressed(keyCode);
			input->InternalKeyCodeTyped(keyCode);
		}
		else
		{
			input->InternalKeyCodeReleased(keyCode);
		}
	}

	return true;
}

int VGUI2_GetLocalPlayerTeam()
{
	return g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber;
}

void VGUI2_RunClientCommand(const char *command)
{
	if (!command || !command[0])
		return;

	gEngfuncs.pfnClientCmd((char *)command);
}
