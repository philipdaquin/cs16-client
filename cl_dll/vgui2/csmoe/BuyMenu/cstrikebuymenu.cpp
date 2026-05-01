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
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include "buymouseoverpanelbutton.h"
#include "../../CBackGroundPanel.h"
#include "../../vgui_resource_paths.h"

using namespace vgui2;

CCSBuyMenu_CT::CCSBuyMenu_CT(IViewport *pViewPort)
	: CCSBaseBuyMenu(pViewPort, "BuySubMenu_CT", vgui2::resource_paths::kMenuBuyCT, TEAM_CT)
{
}

CCSBuyMenu_TER::CCSBuyMenu_TER(IViewport *pViewPort)
	: CCSBaseBuyMenu(pViewPort, "BuySubMenu_TER", vgui2::resource_paths::kMenuBuyTER, TEAM_TERRORIST)
{
}

CCSBaseBuyMenu::CCSBaseBuyMenu(IViewport *pViewPort)
	: CBuyMenu(pViewPort)
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);
	CreateBackground(this);
	m_backgroundLayoutFinished = false;
	m_pMoney = NULL;
	m_pMainBackground = NULL;
	m_pLoadout = NULL;
	m_lastMoney = -1;
	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
		m_pBuyPresetButtons[i] = NULL;
	UpdateGameMode();
}

CCSBaseBuyMenu::CCSBaseBuyMenu(IViewport *pViewPort, const char *subPanelName, const char *resourceName, int team)
	: CBuyMenu(pViewPort)
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);
	CreateBackground(this);
	m_backgroundLayoutFinished = false;
	m_pMoney = NULL;
	m_pMainBackground = NULL;
	m_pLoadout = NULL;
	m_lastMoney = -1;
	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
		m_pBuyPresetButtons[i] = NULL;
	LoadTeamResource(subPanelName, resourceName, team);
}

void CCSBaseBuyMenu::LoadTeamResource(const char *subPanelName, const char *resourceName, int team)
{
	m_iTeam = team;

	ResetHistory();
	ResetCurrentSubPanel();

	if (m_pMainMenu)
	{
		m_pMainMenu->DeletePanel();
		m_pMainMenu = nullptr;
	}

	m_pMainMenu = new CCSBuySubMenu(this, subPanelName);
	m_pMainMenu->LoadControlSettings(resourceName, "GAME");
	SetupBuyPresetControls();
	m_pMainMenu->SetVisible(false);
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
	if (!m_pLoadout && m_pMainMenu)
		m_pLoadout = new BuyPresetEditPanel(m_pMainMenu, "loadoutPanel", "Resource/UI/Loadout.res", 0, false);

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
	if (m_iTeam == TEAM_CT)
		LoadTeamResource("BuySubMenu_CT", vgui2::resource_paths::kMenuBuyCT, TEAM_CT);
	else
		LoadTeamResource("BuySubMenu_TER", vgui2::resource_paths::kMenuBuyTER, TEAM_TERRORIST);
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
		if (cl::gHUD.m_iIntermission || cl::gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);

	if (bShow)
		UpdateBuyPresets(true);
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

	if (!m_backgroundLayoutFinished)
	{
		LayoutBackgroundPanel(this);
		if (m_pMainMenu)
		{
			const int startX = scheme()->GetProportionalScaledValueEx(GetScheme(), 70);
			const int startY = scheme()->GetProportionalScaledValueEx(GetScheme(), 320);
			const int wide = scheme()->GetProportionalScaledValueEx(GetScheme(), 120);
			const int tall = scheme()->GetProportionalScaledValueEx(GetScheme(), 28);
			const int gap = scheme()->GetProportionalScaledValueEx(GetScheme(), 8);
			for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
			{
				if (m_pBuyPresetButtons[i])
					m_pBuyPresetButtons[i]->SetBounds(startX + i * (wide + gap), startY, wide, tall);
			}
			if (m_pMoney)
				m_pMoney->SetBounds(startX, startY - tall - gap, wide * 2, tall);
		}
		m_backgroundLayoutFinished = true;
	}
}

void CCSBaseBuyMenu::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	ApplyBackgroundSchemeSettings(this, pScheme);
	if (m_pMainBackground)
	{
		m_pMainBackground->SetBorder(pScheme->GetBorder("ButtonDepressedBorder"));
		m_pMainBackground->SetBgColor(pScheme->GetColor("Button.BgColor", GetBgColor()));
	}
	m_backgroundLayoutFinished = false;
	UpdateBuyPresets(true);
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
	case MENU_BUY:
	default:
		resource = nullptr;
		break;
	}

	if (resource)
	{
		m_pMainMenu->SetupNextSubPanel(resource);
		m_pMainMenu->GotoNextSubPanel();
	}

	Run(m_pMainMenu);
}

void CCSBaseBuyMenu::ActivateMenu(int iMenu)
{
	GotoMenu(iMenu);
	g_pViewport->ShowPanel(this, true);
}

void CCSBaseBuyMenu::SetTeam(int iTeam)
{
	const int newTeam = (iTeam == TEAM_CT) ? TEAM_CT : TEAM_TERRORIST;
	if (newTeam == m_iTeam && m_pMainMenu)
		return;

	if (newTeam == TEAM_CT)
		LoadTeamResource("BuySubMenu_CT", vgui2::resource_paths::kMenuBuyCT, TEAM_CT);
	else
		LoadTeamResource("BuySubMenu_TER", vgui2::resource_paths::kMenuBuyTER, TEAM_TERRORIST);
}

void CCSBaseBuyMenu::UpdateGameMode()
{
	const int team = (cl::g_iTeamNumber == TEAM_CT) ? TEAM_CT : TEAM_TERRORIST;
	SetTeam(team);
}
