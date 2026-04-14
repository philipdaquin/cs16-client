#include "counterstrikeviewport.h"

#if !defined(VGUI2_STUB_MODE)

typedef float vec_t;
typedef vec_t vec3_t[3];

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
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCounterStrikeViewport ctor ENTRY this=%p parent=%u\n",
		this, parent);

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
	SetBounds(0, 0, gHUD.m_scrinfo.iWidth, gHUD.m_scrinfo.iHeight);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCounterStrikeViewport ctor EXIT this=%p\n", this);
}
CCounterStrikeViewport::~CCounterStrikeViewport()
{
	DestroyPanels();
}

void CCounterStrikeViewport::CreatePanels()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCounterStrikeViewport::CreatePanels ENTRY this=%p created=%d\n",
		this, m_bPanelsCreated ? 1 : 0);

	if (m_bPanelsCreated)
		return;


	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels creating TeamMenu\n");
	m_pTeamMenu = new CTeamMenu(this, "TeamMenu");
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels created TeamMenu=%p\n", m_pTeamMenu);
	if (m_pTeamMenu)
		gEngfuncs.Con_Printf("SETTING m_pTeamMenu=%p to the VIEWPORT", m_pTeamMenu);

	if (m_pTeamMenu)
		m_pTeamMenu->SetViewport(this);

	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels creating ClassMenu\n");
	m_pClassMenu = new CClassMenu(this, "ClassMenu");
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels created ClassMenu=%p\n", m_pClassMenu);
	if (m_pClassMenu)
		m_pClassMenu->SetViewport(this);

	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels creating BuyMenu\n");
	m_pBuyMenu = new CBuyMenu(this, "BuyMenu");
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels created BuyMenu=%p\n", m_pBuyMenu);
	if (m_pBuyMenu)
		m_pBuyMenu->SetViewport(this);

	for (int category = 0; category < CATEGORY_COUNT; ++category)
	{
		for (int side = 0; side < SUBMENU_COUNT; ++side)
		{
			const bool isCT = side == SUBMENU_CT;
			const int index = GetSubMenuIndex((BuyMenuCategory_t)category, isCT);
			gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels creating BuySubMenu category=%d isCT=%d\n",
				category, isCT ? 1 : 0);
			m_apBuySubMenus[index] = new CBuySubMenu(this, "BuySubMenu");
			gEngfuncs.Con_Printf("[VGUI2-CLIENT] CreatePanels created BuySubMenu=%p category=%d isCT=%d\n",
				m_apBuySubMenus[index], category, isCT ? 1 : 0);
			if (m_apBuySubMenus[index])
			{
				m_apBuySubMenus[index]->SetViewport(this);
				m_apBuySubMenus[index]->SetCategory((BuyMenuCategory_t)category, isCT);
			}
		}
	}

	m_bPanelsCreated = true;
	HideAllGameMenus();
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCounterStrikeViewport::CreatePanels EXIT this=%p\n", this);
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
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCounterStrikeViewport::ShowTeamMenu this=%p panels=%d team=%p wide=%d tall=%d\n",
		this, m_bPanelsCreated ? 1 : 0, m_pTeamMenu, GetWide(), GetTall());
	if (!m_bPanelsCreated) {
		CreatePanels();
	}

	HideAllGameMenus();
	SetVisible(true);
	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);

	if (m_pTeamMenu)
	{
		m_pTeamMenu->EnsureControlSettingsLoaded();
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
		m_pClassMenu->EnsureControlSettingsLoaded();
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
		m_pBuyMenu->EnsureControlSettingsLoaded();
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
	m_apBuySubMenus[index]->EnsureControlSettingsLoaded();
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

void CCounterStrikeViewport::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
}

void CCounterStrikeViewport::Paint()
{
	BaseClass::Paint();
}

#endif
