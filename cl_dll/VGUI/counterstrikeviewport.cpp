#include "counterstrikeviewport.h"

#if !defined(VGUI2_STUB_MODE)

#include "hud.h"
#include "VGUI/buymenu.h"
#include "VGUI/buysubmenu.h"
#include "VGUI/cstrikeclassmenu.h"
#include "VGUI/cstriketeammenu.h"
#include "VGUI/counterstrikeviewport_interface.h"

CCounterStrikeViewport::CCounterStrikeViewport(vgui2::VPANEL parent)
	: BaseClass(NULL, "CounterStrikeViewport")
	, m_pTeamMenu(NULL)
	, m_pClassMenu(NULL)
	, m_pBuyMenu(NULL)
	, m_pBuyPresetPanel(NULL)
	, m_pBuyPresetListBox(NULL)
	, m_bPanelsCreated(false)
{
	for (int i = 0; i < ARRAYSIZE(m_apBuySubMenus); ++i)
	{
		m_apBuySubMenus[i] = NULL;
	}

	SetParent(parent);
	SetProportional(false);
	SetVisible(false);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
	SetBounds(0, 0, ScreenWidth, ScreenHeight);

	CreatePanels();
}

CCounterStrikeViewport::~CCounterStrikeViewport()
{
	DestroyPanels();
}

void CCounterStrikeViewport::CreatePanels()
{
	if (m_bPanelsCreated)
		return;

	m_pTeamMenu = new CTeamMenu(this, "TeamMenu");
	m_pTeamMenu->SetViewport(this);

	m_pClassMenu = new CClassMenu(this, "ClassMenu");
	m_pClassMenu->SetViewport(this);

	m_pBuyMenu = new CBuyMenu(this, "BuyMenu");
	m_pBuyMenu->SetViewport(this);

	for (int category = 0; category < CATEGORY_COUNT; ++category)
	{
		for (int side = 0; side < SUBMENU_COUNT; ++side)
		{
			const int index = GetSubMenuIndex((BuyMenuCategory_t)category, side == SUBMENU_CT);
			m_apBuySubMenus[index] = new CBuySubMenu(this, "BuySubMenu");
			m_apBuySubMenus[index]->SetViewport(this);
			m_apBuySubMenus[index]->SetCategory((BuyMenuCategory_t)category, side == SUBMENU_CT);
		}
	}

	m_bPanelsCreated = true;
	HideAllGameMenus();
}

void CCounterStrikeViewport::DestroyPanels()
{
	if (!m_bPanelsCreated)
		return;

	delete m_pTeamMenu;
	m_pTeamMenu = NULL;

	delete m_pClassMenu;
	m_pClassMenu = NULL;

	delete m_pBuyMenu;
	m_pBuyMenu = NULL;

	for (int i = 0; i < ARRAYSIZE(m_apBuySubMenus); ++i)
	{
		delete m_apBuySubMenus[i];
		m_apBuySubMenus[i] = NULL;
	}

	m_bPanelsCreated = false;
}

void CCounterStrikeViewport::ShowTeamMenu()
{
	if (!m_bPanelsCreated)
		CreatePanels();

	HideAllGameMenus();
	SetVisible(true);
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	if (m_pTeamMenu)
	{
		m_pTeamMenu->SetBounds(0, 0, GetWide(), GetTall());
		m_pTeamMenu->SetSpectateVisible(gHUD.m_Menu.m_bAllowSpec);
		m_pTeamMenu->SetVisible(true);
		m_pTeamMenu->MoveToFront();
		m_pTeamMenu->RequestFocus();
	}
}

void CCounterStrikeViewport::ShowClassMenu(int menuType)
{
	if (!m_bPanelsCreated)
		CreatePanels();

	HideAllGameMenus();
	SetVisible(true);
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	if (m_pClassMenu)
	{
		m_pClassMenu->SetMenuType(menuType);
		m_pClassMenu->SetBounds(0, 0, GetWide(), GetTall());
		m_pClassMenu->SetVisible(true);
		m_pClassMenu->MoveToFront();
		m_pClassMenu->RequestFocus();
	}
}

void CCounterStrikeViewport::ShowBuyMenu()
{
	if (!m_bPanelsCreated)
		CreatePanels();

	HideAllGameMenus();
	SetVisible(true);
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	if (m_pBuyMenu)
	{
		m_pBuyMenu->SetBounds(0, 0, GetWide(), GetTall());
		m_pBuyMenu->SetVisible(true);
		m_pBuyMenu->MoveToFront();
		m_pBuyMenu->RequestFocus();
	}
}

void CCounterStrikeViewport::ShowBuySubMenu(BuyMenuCategory_t category)
{
	if (!m_bPanelsCreated)
		CreatePanels();

	const bool bIsCT = VGUI2_GetLocalPlayerTeam() == TEAM_CT;
	const int index = GetSubMenuIndex(category, bIsCT);

	if (index < 0 || index >= ARRAYSIZE(m_apBuySubMenus) || !m_apBuySubMenus[index])
		return;

	HideAllGameMenus();
	SetVisible(true);
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	m_apBuySubMenus[index]->SetCategory(category, bIsCT);
	m_apBuySubMenus[index]->SetBounds(0, 0, GetWide(), GetTall());
	m_apBuySubMenus[index]->SetVisible(true);
	m_apBuySubMenus[index]->MoveToFront();
	m_apBuySubMenus[index]->RequestFocus();
}

void CCounterStrikeViewport::HideAllGameMenus()
{
	SetVisible(false);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);

	if (m_pTeamMenu)
		m_pTeamMenu->SetVisible(false);

	if (m_pClassMenu)
		m_pClassMenu->SetVisible(false);

	if (m_pBuyMenu)
		m_pBuyMenu->SetVisible(false);

	for (int i = 0; i < ARRAYSIZE(m_apBuySubMenus); ++i)
	{
		if (m_apBuySubMenus[i])
			m_apBuySubMenus[i]->SetVisible(false);
	}
}

void CCounterStrikeViewport::ApplySchemeSettings(vgui2::HScheme scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
}

void CCounterStrikeViewport::Paint()
{
	BaseClass::Paint();
}

#endif
