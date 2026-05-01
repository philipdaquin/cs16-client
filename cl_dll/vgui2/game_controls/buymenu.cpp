#include "hud.h"
#include "../CBaseViewport.h"
#include "buymenu.h"
#include "buysubmenu.h"
#include "mouseoverpanelbutton.h"
#include "../vgui_resource_paths.h"

using namespace vgui2;

static bool IsLocalPlayerTerrorist()
{
	return g_PlayerExtraInfo[gHUD.m_Scoreboard.m_iPlayerNum].teamnumber == TEAM_TERRORIST;
}

static const char *GetBuyMenuResourceForMenu(int iMenu)
{
	const bool isTerrorist = IsLocalPlayerTerrorist();

	switch (iMenu)
	{
	case MENU_BUY_PISTOL:
		return vgui2::resource_paths::BuyCategoryForTeam(vgui2::resource_paths::kMenuBuyPistolsCT, vgui2::resource_paths::kMenuBuyPistolsTER, isTerrorist);
	case MENU_BUY_SHOTGUN:
		return vgui2::resource_paths::BuyCategoryForTeam(vgui2::resource_paths::kMenuBuyShotgunsCT, vgui2::resource_paths::kMenuBuyShotgunsTER, isTerrorist);
	case MENU_BUY_RIFLE:
		return vgui2::resource_paths::BuyCategoryForTeam(vgui2::resource_paths::kMenuBuyRiflesCT, vgui2::resource_paths::kMenuBuyRiflesTER, isTerrorist);
	case MENU_BUY_SUBMACHINEGUN:
		return vgui2::resource_paths::BuyCategoryForTeam(vgui2::resource_paths::kMenuBuySubMachinegunsCT, vgui2::resource_paths::kMenuBuySubMachinegunsTER, isTerrorist);
	case MENU_BUY_MACHINEGUN:
		return vgui2::resource_paths::BuyCategoryForTeam(vgui2::resource_paths::kMenuBuyMachinegunsCT, vgui2::resource_paths::kMenuBuyMachinegunsTER, isTerrorist);
	case MENU_BUY_ITEM:
		return vgui2::resource_paths::kMenuBuyEquipment;
	default:
		return nullptr;
	}
}

CBuyMenu::CBuyMenu(IViewport *pViewPort) : WizardPanel(NULL, PANEL_BUY), m_pViewPort(pViewPort)
{
	SetScheme("ClientScheme");
	SetTitle("#Cstrike_Buy_Menu", true);

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	SetTitleBarVisible(false);
	SetAutoDelete(false);

	LoadControlSettings(vgui2::resource_paths::kMenuBuy, "GAME");

	m_pMainMenu = new CBuySubMenu(this, "mainmenu");

	m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");
	m_pMainMenu->SetVisible(false);

	ShowButtons(false);
}

CBuyMenu::~CBuyMenu(void)
{
	if (m_pMainMenu)
		m_pMainMenu->DeleteSubPanels();
}

void CBuyMenu::Init(void)
{
}

void CBuyMenu::VidInit(void)
{
	SetVisible(false);
}

void CBuyMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Update();

		Run(m_pMainMenu);

		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	m_pViewPort->ShowBackGround( bShow );
}

void CBuyMenu::Update(void)
{
	NULL;
}

void CBuyMenu::OnClose(void)
{
	BaseClass::OnClose();

	ResetHistory();
}

void CBuyMenu::OnKeyCodeTyped(KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		OnClose();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}
