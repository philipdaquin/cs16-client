#include "hud.h"
#include "cstrikebuyequipmenu.h"
#include "cstrikebuysubmenu.h"
#include "../../CBackGroundPanel.h"
#include "../../vgui_resource_paths.h"

CCSBuyEquipMenu_CT::CCSBuyEquipMenu_CT(IViewport *pViewPort) : CBuyMenu(pViewPort)
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);

	if (m_pMainMenu)
		m_pMainMenu->DeletePanel();

	m_pMainMenu = new CCSBuySubMenu(this, "BuySubMenu");
	m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyEquipmentCT, "GAME");
	m_pMainMenu->SetVisible(false);

	m_iTeam = TEAM_CT;

	// CreateBackground(this);
	m_backgroundLayoutFinished = false;
}

void CCSBuyEquipMenu_CT::PaintBackground(void)
{
}

void CCSBuyEquipMenu_CT::PerformLayout(void)
{
	BaseClass::PerformLayout();

	// if (!m_backgroundLayoutFinished)
	// {
	// 	LayoutBackgroundPanel(this);
	// 	m_backgroundLayoutFinished = true;
	// }
}

void CCSBuyEquipMenu_CT::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	// ApplyBackgroundSchemeSettings(this, pScheme);
	// m_backgroundLayoutFinished = false;
}

CCSBuyEquipMenu_TER::CCSBuyEquipMenu_TER(IViewport *pViewPort) : CBuyMenu(pViewPort)
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);

	if (m_pMainMenu)
		m_pMainMenu->DeletePanel();

	m_pMainMenu = new CCSBuySubMenu(this, "BuySubMenu");
	m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyEquipmentTER, "GAME");
	m_pMainMenu->SetVisible(false);

	m_iTeam = TEAM_TERRORIST;

	// CreateBackground(this);
	m_backgroundLayoutFinished = false;
}

void CCSBuyEquipMenu_TER::PaintBackground(void)
{
}

void CCSBuyEquipMenu_TER::PerformLayout(void)
{
	BaseClass::PerformLayout();

	// if (!m_backgroundLayoutFinished)
	// {
	// 	LayoutBackgroundPanel(this);
	// 	m_backgroundLayoutFinished = true;
	// }
}

void CCSBuyEquipMenu_TER::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	ApplyBackgroundSchemeSettings(this, pScheme);
	m_backgroundLayoutFinished = false;
}
