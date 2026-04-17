#include "counterstrikeviewport.h"

#if !defined(VGUI2_STUB_MODE)

#include <tier2/tier2.h>
#include <IEngineVGui.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Frame.h>

typedef float vec_t;
typedef vec_t vec3_t[3];

#include "hud.h"
#include "VGUI/buymenu.h"
#include "VGUI/buysubmenu.h"
#include "VGUI/cstrikeclassmenu.h"
#include "VGUI/cstriketeammenu.h"
#include "VGUI/counterstrikeviewport_interface.h"

namespace
{

class CCSViewportBackground : public vgui2::Frame
{
	DECLARE_CLASS_SIMPLE(CCSViewportBackground, vgui2::Frame);

public:
	explicit CCSViewportBackground(vgui2::Panel *parent)
		: BaseClass(parent, "ViewPortBackground")
	{
		SetScheme("ClientScheme");
		SetTitle("", true);
		SetTitleBarVisible(false);
		SetMoveable(false);
		SetSizeable(false);
		SetProportional(true);
		SetVisible(false);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
	}

	void ApplySchemeSettings(vgui2::IScheme *scheme) override
	{
		BaseClass::ApplySchemeSettings(scheme);
		SetBgColor(scheme->GetColor("ViewportBG", Color(0, 0, 0, 0)));
	}
};

static void CenterPanelInViewport(vgui2::Panel *panel, vgui2::Panel *viewport)
{
	if (!panel || !viewport)
		return;

	int menuW = 0, menuH = 0;
	int wide = 0, tall = 0;
	panel->GetSize(menuW, menuH);
	viewport->GetSize(wide, tall);
	panel->SetPos((wide - menuW) / 2, (tall - menuH) / 2);
}

}

CCounterStrikeViewport::CCounterStrikeViewport(vgui2::VPANEL parent)
	: BaseClass(parent, "CounterStrikeViewport")
	, m_pTeamMenu(NULL)
	, m_pClassMenu(NULL)
	, m_pBuyMenu(NULL)
	, m_pBuyPresetPanel(NULL)
	, m_pBuyPresetListBox(NULL)
	, m_pBackGround(NULL)
	, m_bPanelsCreated(false)
{
	for (int i = 0; i < ARRAYSIZE(m_apBuySubMenus); ++i)
	{
		m_apBuySubMenus[i] = NULL;
	}

	SetBounds(0, 0, gHUD.m_scrinfo.iWidth, gHUD.m_scrinfo.iHeight);

	m_pBackGround = new CCSViewportBackground(this);
	vgui2::ipanel()->MoveToBack(m_pBackGround->GetVPanel());

	ReloadScheme();
}
CCounterStrikeViewport::~CCounterStrikeViewport()
{
	DestroyPanels();
	delete m_pBackGround;
	m_pBackGround = NULL;
}

void CCounterStrikeViewport::CreatePanels()
{
	if (m_bPanelsCreated)
		return;
	m_pTeamMenu = new CTeamMenu(this, "TeamMenu");
	if (m_pTeamMenu)
		m_pTeamMenu->SetViewport(this);

	m_pClassMenu = new CClassMenu(this, "ClassMenu");
	if (m_pClassMenu)
		m_pClassMenu->SetViewport(this);

	m_pBuyMenu = new CBuyMenu(this, "BuyMenu");
	if (m_pBuyMenu)
		m_pBuyMenu->SetViewport(this);

	for (int category = 0; category < CATEGORY_COUNT; ++category)
	{
		for (int side = 0; side < SUBMENU_COUNT; ++side)
		{
			const bool isCT = side == SUBMENU_CT;
			const int index = GetSubMenuIndex((BuyMenuCategory_t)category, isCT);
			m_apBuySubMenus[index] = new CBuySubMenu(this, "BuySubMenu");
			if (m_apBuySubMenus[index])
			{
				m_apBuySubMenus[index]->SetViewport(this);
				m_apBuySubMenus[index]->SetCategory((BuyMenuCategory_t)category, isCT);
			}
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
	ShowPanel(true);

	if (m_pTeamMenu)
	{
		m_pTeamMenu->EnsureControlSettingsLoaded();
		m_pTeamMenu->SetSpectateVisible(gHUD.m_Menu.m_bAllowSpec);
		m_pTeamMenu->InvalidateLayout(true, true);
		m_pTeamMenu->MoveToCenterOfScreen();
		m_pTeamMenu->Repaint();
		m_pTeamMenu->ShowPanel(true);
		m_pTeamMenu->MoveToFront();
		m_pTeamMenu->RequestFocus();
	}
}

void CCounterStrikeViewport::ShowClassMenu(int menuType)
{
	if (!m_bPanelsCreated)
		CreatePanels();

	HideAllGameMenus();
	ShowPanel(true);

	if (m_pClassMenu)
	{
		m_pClassMenu->SetMenuType(menuType);
		m_pClassMenu->EnsureControlSettingsLoaded();
		m_pClassMenu->InvalidateLayout(true, true);
		CenterPanelInViewport(m_pClassMenu, this);
		m_pClassMenu->ShowPanel(true);
		m_pClassMenu->MoveToFront();
		m_pClassMenu->RequestFocus();
	}
}

void CCounterStrikeViewport::ShowBuyMenu()
{
	if (!m_bPanelsCreated)
		CreatePanels();

	HideAllGameMenus();
	ShowPanel(true);

	if (m_pBuyMenu)
	{
		m_pBuyMenu->EnsureControlSettingsLoaded();
		CenterPanelInViewport(m_pBuyMenu, this);
		m_pBuyMenu->ShowPanel(true);
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
	ShowPanel(true);

	m_apBuySubMenus[index]->SetCategory(category, bIsCT);
	m_apBuySubMenus[index]->EnsureControlSettingsLoaded();
	m_apBuySubMenus[index]->InvalidateLayout(true, true);
	CenterPanelInViewport(m_apBuySubMenus[index], this);
	m_apBuySubMenus[index]->ShowPanel(true);
	m_apBuySubMenus[index]->MoveToFront();
	m_apBuySubMenus[index]->RequestFocus();
}

void CCounterStrikeViewport::HideAllGameMenus()
{
	ShowPanel(false);
	ShowBackGround(false);

	if (m_pTeamMenu)
		m_pTeamMenu->ShowPanel(false);

	if (m_pClassMenu)
		m_pClassMenu->ShowPanel(false);

	if (m_pBuyMenu)
		m_pBuyMenu->ShowPanel(false);

	for (int i = 0; i < ARRAYSIZE(m_apBuySubMenus); ++i)
	{
		if (m_apBuySubMenus[i])
			m_apBuySubMenus[i]->ShowPanel(false);
	}
}

void CCounterStrikeViewport::ShowBackGround(bool bShow)
{
	if (m_pBackGround)
		m_pBackGround->SetVisible(bShow);
}

void CCounterStrikeViewport::ReloadScheme(const char *fromFile)
{
	BaseClass::ReloadScheme(fromFile);
	if (m_pBackGround)
		m_pBackGround->SetScheme("ClientScheme");
}

void CCounterStrikeViewport::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
}

void CCounterStrikeViewport::PerformLayout()
{
	int wide = gHUD.m_scrinfo.iWidth;
	int tall = gHUD.m_scrinfo.iHeight;

	if (GetVParent())
		vgui2::ipanel()->GetSize(GetVParent(), wide, tall);

	SetBounds(0, 0, wide, tall);
	if (m_pBackGround)
		m_pBackGround->SetBounds(0, 0, wide, tall);

	BaseClass::PerformLayout();
}

void CCounterStrikeViewport::Paint()
{
	BaseClass::Paint();
}

#endif
