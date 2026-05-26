#include "hud.h"
#include "../../CBaseViewport.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "cstrikebuymenu.h"
#include "cstrikebuysubmenu.h"
#include "buy_presets.h"
#include "buypresetbutton.h"
#include "buypreset_weaponsetlabel.h"
#include "shared_util.h"
#include "../vgui_int.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include "buymouseoverpanelbutton.h"
#include "../../CBackGroundPanel.h"
#include "../../vgui_resource_paths.h"

using namespace vgui2;

static void SetMainBuyButtonCommand(Panel *panel, const char *buttonName, const char *command)
{
	Button *button = panel ? dynamic_cast<Button *>(panel->FindChildByName(buttonName)) : NULL;
	if (button)
		button->SetCommand(command);
}

CCSBuyMenu_CT::CCSBuyMenu_CT(IViewport *pViewPort)
	: CCSBaseBuyMenu(pViewPort, TEAM_CT)
{
		m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");
}

CCSBuyMenu_TER::CCSBuyMenu_TER(IViewport *pViewPort)
	: CCSBaseBuyMenu(pViewPort, TEAM_TERRORIST)
{
		m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");
}

CCSBaseBuyMenu::CCSBaseBuyMenu(IViewport *pViewPort)
	: CBuyMenu(pViewPort)
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);
	// CreateBackground(this);
	m_backgroundLayoutFinished = false;
	m_pMoney = NULL;
	m_pMainBackground = NULL;
	m_pLoadout = NULL;
	m_lastMoney = -1;
	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
		m_pBuyPresetButtons[i] = NULL;
	UpdateGameMode();
}

CCSBaseBuyMenu::CCSBaseBuyMenu(IViewport *pViewPort, int team)
	: CBuyMenu(pViewPort)
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);
	// CreateBackground(this);
	m_backgroundLayoutFinished = false;
	m_pMoney = NULL;
	m_pMainBackground = NULL;
	m_pLoadout = NULL;
	m_lastMoney = -1;
	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
		m_pBuyPresetButtons[i] = NULL;
	LoadTeamResource(team);
}

void CCSBaseBuyMenu::LoadTeamResource(int team)
{
	m_iTeam = team;

	ResetHistory();
	ResetCurrentSubPanel();

	if (m_pMainMenu)
	{
		m_pMainMenu->DeletePanel();
		m_pMainMenu = nullptr;
	}

	m_pMainMenu = new CCSBuySubMenu(this, "BuySubMenu");
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::LoadTeamResource this=%p team=%d loading main='%s'\n",
		this, m_iTeam, vgui2::resource_paths::kMenuBuyMain);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::LoadTeamResource LoadControlSettings main='%s' submenu=%p\n",
		vgui2::resource_paths::kMenuBuyMain, (void *)m_pMainMenu);
	m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");
	ConfigureMainBuyMenuCommands();
	SetupBuyPresetControls();
	m_pMainMenu->SetVisible(false);
}

void CCSBaseBuyMenu::ConfigureMainBuyMenuCommands()
{
	if (!m_pMainMenu)
		return;

	const bool isTerrorist = (m_iTeam == TEAM_TERRORIST);

	SetMainBuyButtonCommand(m_pMainMenu, "pistols",
		isTerrorist ? vgui2::resource_paths::kMenuBuyPistolsTER : vgui2::resource_paths::kMenuBuyPistolsCT);
	SetMainBuyButtonCommand(m_pMainMenu, "shotguns",
		isTerrorist ? vgui2::resource_paths::kMenuBuyShotgunsTER : vgui2::resource_paths::kMenuBuyShotgunsCT);
	SetMainBuyButtonCommand(m_pMainMenu, "submachineguns",
		isTerrorist ? vgui2::resource_paths::kMenuBuySubMachinegunsTER : vgui2::resource_paths::kMenuBuySubMachinegunsCT);
	SetMainBuyButtonCommand(m_pMainMenu, "rifles",
		isTerrorist ? vgui2::resource_paths::kMenuBuyRiflesTER : vgui2::resource_paths::kMenuBuyRiflesCT);
	SetMainBuyButtonCommand(m_pMainMenu, "machineguns",
		isTerrorist ? vgui2::resource_paths::kMenuBuyMachinegunsTER : vgui2::resource_paths::kMenuBuyMachinegunsCT);
	SetMainBuyButtonCommand(m_pMainMenu, "equipment",
		isTerrorist ? vgui2::resource_paths::kMenuBuyEquipmentTER : vgui2::resource_paths::kMenuBuyEquipmentCT);

	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::ConfigureMainBuyMenuCommands this=%p team=%d targetTeam=%s\n",
		this, m_iTeam, isTerrorist ? "TER" : "CT");
}

void CCSBaseBuyMenu::SetupBuyPresetControls()
{
	m_pMainBackground = m_pMainMenu ? dynamic_cast<Panel *>(m_pMainMenu->FindChildByName("mainBackground")) : NULL;
	if (!m_pMainBackground && m_pMainMenu)
		m_pMainBackground = new Panel(m_pMainMenu, "mainBackground");

	m_pMoney = m_pMainMenu ? dynamic_cast<Label *>(m_pMainMenu->FindChildByName("money")) : NULL;
	if (!m_pMoney && m_pMainMenu)
		m_pMoney = new Label(m_pMainMenu, "money", "");

	m_pLoadout = m_pMainMenu ? dynamic_cast<BuyPresetEditPanel *>(m_pMainMenu->FindChildByName("loadoutPanel")) : NULL;
	if (!m_pLoadout)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::SetupBuyPresetControls no loadoutPanel in '%s'; skipping preset editor\n",
			vgui2::resource_paths::kMenuBuyMain);
	}

	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
	{
		char name[32];
		Q_snprintf(name, sizeof(name), "BuyPresetButton%c", 'A' + i);
		m_pBuyPresetButtons[i] = m_pMainMenu ? dynamic_cast<BuyPresetButton *>(m_pMainMenu->FindChildByName(name)) : NULL;
		if (!m_pBuyPresetButtons[i] && m_pMainMenu)
			m_pBuyPresetButtons[i] = new BuyPresetButton(m_pMainMenu, name);
	}
}

void CCSBaseBuyMenu::SetupControlSettings()
{

	// LoadControlSettings(vgui2::resource_paths::kMenuBuy, "GAME");


	if (m_iTeam == TEAM_CT)
		LoadTeamResource(TEAM_CT);
	else
		LoadTeamResource(TEAM_TERRORIST);

	// if (m_pMainMenu) { 
	// 	m_pMainMenu->SetVisible(false);
	// }

}

void CCSBaseBuyMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *defaultButton = FindChildByName("CancelButton");
		if (!defaultButton && m_pMainMenu)
			defaultButton = m_pMainMenu->FindChildByName("CancelButton");

		if (defaultButton)
			defaultButton->RequestFocus();

		SetMouseInputEnabled(true);
		if (m_pMainMenu)
			m_pMainMenu->SetMouseInputEnabled(true);
	}
}

void CCSBaseBuyMenu::Init(void)
{
}

void CCSBaseBuyMenu::VidInit(void)
{
	SetVisible(false);
}

void CCSBaseBuyMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		// if (cl::gHUD.m_iIntermission || cl::gEngfuncs.IsSpectateOnly())
		// 	return;

		// int wide = 0;
		// int tall = 0;
		// GetHudSize(wide, tall);
		// SetPos(0, 0);
		// SetSize(wide, tall);

		// const int team = (cl::g_iTeamNumber == TEAM_CT) ? TEAM_CT : TEAM_TERRORIST;
		// SetTeam(team);
		// ResetHistory();
		// ResetCurrentSubPanel();
		// if (m_pMainMenu)
		// {
		// 	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::ShowPanel prepare this=%p team=%d main='%s' mainPanel=%p current=%p\n",
		// 		this, m_iTeam, vgui2::resource_paths::kMenuBuyMain, (void *)m_pMainMenu, (void *)GetCurrentSubPanel());
		// 	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::ShowPanel LoadControlSettings main='%s' submenu=%p\n",
		// 		vgui2::resource_paths::kMenuBuyMain, (void *)m_pMainMenu);
		// 	m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");
		// 	ConfigureMainBuyMenuCommands();
		// 	SetupBuyPresetControls();
		// 	m_pMainMenu->InvalidateLayout();
		// }
	}

	BaseClass::ShowPanel(bShow);

	// if (bShow)
	// 	UpdateBuyPresets(true);
}

void CCSBaseBuyMenu::Update(void)
{
	ConfigureMainBuyMenuCommands();
	SetupBuyPresetControls();
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::Update this=%p team=%d mainPanel=%p current=%p\n",
		this, m_iTeam, (void *)m_pMainMenu, (void *)GetCurrentSubPanel());
}

void CCSBaseBuyMenu::Paint(void)
{
	if (m_pMoney && m_lastMoney != cl::gHUD.m_Money.m_iMoneyCount)
	{
		m_lastMoney = cl::gHUD.m_Money.m_iMoneyCount;
		char money[64];
		Q_snprintf(money, sizeof(money), "$%d", m_lastMoney);
		m_pMoney->SetText(money);
	}

	BaseClass::Paint();
}

void CCSBaseBuyMenu::PaintBackground(void)
{
}

void CCSBaseBuyMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();

	// if (!m_backgroundLayoutFinished)
	// {
	// 	LayoutBackgroundPanel(this);
	// 	if (m_pMainMenu)
	// 	{
	// 		const int startX = scheme()->GetProportionalScaledValueEx(GetScheme(), 70);
	// 		const int startY = scheme()->GetProportionalScaledValueEx(GetScheme(), 320);
	// 		const int wide = scheme()->GetProportionalScaledValueEx(GetScheme(), 120);
	// 		const int tall = scheme()->GetProportionalScaledValueEx(GetScheme(), 28);
	// 		const int gap = scheme()->GetProportionalScaledValueEx(GetScheme(), 8);
	// 		for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
	// 		{
	// 			if (m_pBuyPresetButtons[i])
	// 				m_pBuyPresetButtons[i]->SetBounds(startX + i * (wide + gap), startY, wide, tall);
	// 		}
	// 		if (m_pMoney)
	// 			m_pMoney->SetBounds(startX, startY - tall - gap, wide * 2, tall);
	// 	}
	// 	m_backgroundLayoutFinished = true;
	// }
}

void CCSBaseBuyMenu::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	// ApplyBackgroundSchemeSettings(this, pScheme);
	// if (m_pMainBackground)
	// {
	// 	m_pMainBackground->SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
	// 	m_pMainBackground->SetBgColor(pScheme->GetColor("Button.BgColor", GetBgColor()));
	// }
	// m_backgroundLayoutFinished = false;
	// UpdateBuyPresets(true);
}

void CCSBaseBuyMenu::UpdateBuyPresets(bool)
{
	if (!TheBuyPresets)
		TheBuyPresets = new BuyPresetManager();

	const int presetCount = TheBuyPresets->GetNumPresets();
	const int numPresets = (presetCount < (int)NUM_BUY_PRESET_BUTTONS) ? presetCount : (int)NUM_BUY_PRESET_BUTTONS;
	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
	{
		BuyPresetButton *button = m_pBuyPresetButtons[i];
		if (!button)
			continue;

		if (i >= numPresets)
		{
			button->SetVisible(false);
			button->SetEnabled(false);
			continue;
		}

		const BuyPreset *preset = TheBuyPresets->GetPreset(i);
		int currentCost = -1;
		WeaponSet currentSet;
		const WeaponSet *fullSet = preset ? preset->GetSet(0) : NULL;
		if (fullSet)
			fullSet->GetCurrent(currentCost, currentSet);

		button->ClearWeapons();
		if (fullSet)
		{
			button->SetPrimaryWeapon(ImageFnameFromWeaponID(fullSet->GetPrimaryWeapon().GetWeaponID(), true));
			button->SetSecondaryWeapon(ImageFnameFromWeaponID(fullSet->GetSecondaryWeapon().GetWeaponID(), false));
			if (i == 0 && m_pLoadout)
				m_pLoadout->SetWeaponSet(fullSet, true);
		}

		char text[32];
		Q_snprintf(text, sizeof(text), "#Cstrike_BuyMenuPreset%d", i + 1);
		button->SetText(text);
		char command[64];
		Q_snprintf(command, sizeof(command), "cl_buy_favorite %d", i + 1);
		button->SetCommand(command);
		button->SetAvailable(currentCost >= 0);
		button->SetVisible(true);
		button->SetEnabled(true);
	}

	Run(m_pMainMenu);
}

void CCSBaseBuyMenu::GotoMenu(int iMenu)
{
	if (!m_pMainMenu)
		return;

	const char *resource = nullptr;
	switch (iMenu)
	{
	case MENU_BUY_PISTOL:
		resource = (m_iTeam == TEAM_TERRORIST) ? vgui2::resource_paths::kMenuBuyPistolsTER : vgui2::resource_paths::kMenuBuyPistolsCT;
		break;
	case MENU_BUY_SHOTGUN:
		resource = (m_iTeam == TEAM_TERRORIST) ? vgui2::resource_paths::kMenuBuyShotgunsTER : vgui2::resource_paths::kMenuBuyShotgunsCT;
		break;
	case MENU_BUY_RIFLE:
		resource = (m_iTeam == TEAM_TERRORIST) ? vgui2::resource_paths::kMenuBuyRiflesTER : vgui2::resource_paths::kMenuBuyRiflesCT;
		break;
	case MENU_BUY_SUBMACHINEGUN:
		resource = (m_iTeam == TEAM_TERRORIST) ? vgui2::resource_paths::kMenuBuySubMachinegunsTER : vgui2::resource_paths::kMenuBuySubMachinegunsCT;
		break;
	case MENU_BUY_MACHINEGUN:
		resource = (m_iTeam == TEAM_TERRORIST) ? vgui2::resource_paths::kMenuBuyMachinegunsTER : vgui2::resource_paths::kMenuBuyMachinegunsCT;
		break;
	case MENU_BUY_ITEM:
		resource = (m_iTeam == TEAM_TERRORIST) ? vgui2::resource_paths::kMenuBuyEquipmentTER : vgui2::resource_paths::kMenuBuyEquipmentCT;
		break;
	case MENU_BUY:
	default:
		resource = nullptr;
		break;
	}

	if (resource)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::GotoMenu this=%p menu=%d team=%d resource='%s' mainPanel=%p current=%p\n",
			this, iMenu, m_iTeam, resource, (void *)m_pMainMenu, (void *)GetCurrentSubPanel());
		m_pMainMenu->SetupNextSubPanel(resource);
		m_pMainMenu->GotoNextSubPanel();
	}
}

void CCSBaseBuyMenu::ActivateMenu(int iMenu)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::ActivateMenu entry this=%p menu=%d team=%d visible=%d mainPanel=%p current=%p\n",
		this, iMenu, m_iTeam, IsVisible() ? 1 : 0, (void *)m_pMainMenu, (void *)GetCurrentSubPanel());
	g_pViewport->ShowPanel(this, true);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::ActivateMenu after-show this=%p menu=%d visible=%d mainPanel=%p current=%p\n",
		this, iMenu, IsVisible() ? 1 : 0, (void *)m_pMainMenu, (void *)GetCurrentSubPanel());

	if (iMenu == MENU_BUY)
	{
		if (m_pMainMenu && GetCurrentSubPanel() != m_pMainMenu)
		{
			ResetHistory();
			Run(m_pMainMenu);
		}
		return;
	}

	if (m_pMainMenu && GetCurrentSubPanel() != m_pMainMenu)
		Run(m_pMainMenu);

	GotoMenu(iMenu);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBaseBuyMenu::ActivateMenu after-goto this=%p menu=%d visible=%d mainPanel=%p current=%p\n",
		this, iMenu, IsVisible() ? 1 : 0, (void *)m_pMainMenu, (void *)GetCurrentSubPanel());
}

void CCSBaseBuyMenu::SetTeam(int iTeam)
{
	const int newTeam = (iTeam == TEAM_CT) ? TEAM_CT : TEAM_TERRORIST;
	if (newTeam == m_iTeam && m_pMainMenu)
		return;

	if (newTeam == TEAM_CT)
		LoadTeamResource(TEAM_CT);
	else
		LoadTeamResource(TEAM_TERRORIST);
}

void CCSBaseBuyMenu::UpdateGameMode()
{
	const int team = (cl::g_iTeamNumber == TEAM_CT) ? TEAM_CT : TEAM_TERRORIST;
	SetTeam(team);
}