#include "VGUI/buymenu.h"
#include "VGUI/buysubmenu.h"
#include "VGUI/counterstrikeviewport.h"
#include "VGUI/vgui_legacy_api.h"
#include "cdll_dll.h"

#include <stdio.h>
#include <string.h>
#include <vgui/IInputInternal.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Label.h>

using namespace vgui2;

CBuyMenu::CBuyMenu(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_pMainPage = NULL;
	m_pCurrentPage = NULL;
	m_pPageHost = NULL;
	m_pMoneyLabel = NULL;
	m_iRequestedMenuType = 0;
	m_iLastKnownMoney = 0;

	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetVisible(false);
	SetScheme(g_pViewport ? g_pViewport->GetSchemeHandle() : 0);

	LoadControlSettings("resource/UI/BuyMenu.res");
	SetSize(640, 480);

	m_pPageHost = FindChildByName("BuySubMenu", true);
	if (!m_pPageHost)
		m_pPageHost = this;

	m_pMoneyLabel = new Label(this, "CurrentMoney", "");
	m_pMoneyLabel->SetBounds(420, 24, 180, 24);
	m_pMoneyLabel->SetContentAlignment(Label::a_east);

	m_pMainPage = GetOrCreatePage("resource/UI/MainBuyMenu.res");
}

CBuyMenu::~CBuyMenu()
{
}

const char *CBuyMenu::GetName()
{
	return BaseClass::GetName();
}

void CBuyMenu::SetData(KeyValues *data)
{
	(void)data;
}

void CBuyMenu::Reset()
{
	m_iRequestedMenuType = 0;
	CloseSubMenu();
	ShowPanel(false);
}

void CBuyMenu::Update()
{
	CenterInViewport();
	UpdateMoneyLabel();
	UpdateAffordability();

	if (!m_pCurrentPage)
		ShowPage(m_pMainPage);

	ShowRequestedSubMenu();
}

bool CBuyMenu::NeedsUpdate()
{
	return false;
}

bool CBuyMenu::HasInputElements()
{
	return true;
}

void CBuyMenu::ShowPanel(bool state)
{
	if (state)
	{
		Update();
		SetVisible(true);
		SetMouseInputEnabled(true);
		SetKeyBoardInputEnabled(true);
		MoveToFront();
		RequestFocus();
		vgui2::input()->SetAppModalSurface(GetVPanel());
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
		if (vgui2::input()->GetAppModalSurface() == GetVPanel())
			vgui2::input()->SetAppModalSurface(NULL);
	}
}

VPANEL CBuyMenu::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CBuyMenu::IsVisible()
{
	return BaseClass::IsVisible();
}

void CBuyMenu::SetParent(VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CBuyMenu::OnCommand(const char *command)
{
	if (!command || !command[0])
		return;

	if (!stricmp(command, "vguicancel"))
	{
		if (m_pCurrentPage && m_pCurrentPage != m_pMainPage)
			ShowPage(m_pMainPage);
		else if (g_pViewport)
			g_pViewport->ShowPanel(PANEL_BUY, false);
		return;
	}

	const char *resourceFile = ResolveCommandResource(command);
	if (resourceFile)
	{
		ShowPage(GetOrCreatePage(resourceFile));
		return;
	}

	char buffer[256];
	snprintf(buffer, sizeof(buffer), "%s\n", command);
	VGuiLegacy_ClientCmd(buffer);
}

void CBuyMenu::OnKeyCodePressed(KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		if (m_pCurrentPage && m_pCurrentPage != m_pMainPage)
			ShowPage(m_pMainPage);
		else if (g_pViewport)
			g_pViewport->ShowPanel(PANEL_BUY, false);
		return;
	}

	BaseClass::OnKeyCodePressed(code);
}

void CBuyMenu::PerformLayout()
{
	BaseClass::PerformLayout();
	CenterInViewport();

	if (m_pPageHost && m_pMoneyLabel)
	{
		m_pMoneyLabel->SetPos(GetWide() - 200, 24);
	}

	for (int i = 0; i < m_Pages.Count(); ++i)
	{
		if (!m_Pages[i])
			continue;

		m_Pages[i]->SetPos(0, 0);
		m_Pages[i]->SetSize(m_pPageHost->GetWide(), m_pPageHost->GetTall());
	}
}

void CBuyMenu::SetActiveSubMenu(int menuType)
{
	m_iRequestedMenuType = menuType;
}

void CBuyMenu::ShowRequestedSubMenu()
{
	if (!m_iRequestedMenuType)
		return;

	const char *resourceFile = GetMenuTypeResource(m_iRequestedMenuType);
	m_iRequestedMenuType = 0;

	if (resourceFile)
		ShowPage(GetOrCreatePage(resourceFile));
}

void CBuyMenu::CloseSubMenu()
{
	ShowPage(m_pMainPage);
}

int CBuyMenu::GetCurrentMoney() const
{
	const int money = VGuiLegacy_GetCurrentMoney();
	return money > 0 ? money : m_iLastKnownMoney;
}

const char *CBuyMenu::GetTeamSuffix() const
{
	return VGuiLegacy_GetTeamNumber() == TEAM_TERRORIST ? "TER" : "CT";
}

const char *CBuyMenu::GetMenuTypeResource(int menuType) const
{
	switch (menuType)
	{
	case MENU_BUY_PISTOL:
		return VGuiLegacy_GetTeamNumber() == TEAM_TERRORIST ? "resource/UI/BuyPistols_TER.res" : "resource/UI/BuyPistols_CT.res";
	case MENU_BUY_SHOTGUN:
		return VGuiLegacy_GetTeamNumber() == TEAM_TERRORIST ? "resource/UI/BuyShotguns_TER.res" : "resource/UI/BuyShotguns_CT.res";
	case MENU_BUY_SUBMACHINEGUN:
		return VGuiLegacy_GetTeamNumber() == TEAM_TERRORIST ? "resource/UI/BuySubMachineguns_TER.res" : "resource/UI/BuySubMachineguns_CT.res";
	case MENU_BUY_RIFLE:
		return VGuiLegacy_GetTeamNumber() == TEAM_TERRORIST ? "resource/UI/BuyRifles_TER.res" : "resource/UI/BuyRifles_CT.res";
	case MENU_BUY_MACHINEGUN:
		return VGuiLegacy_GetTeamNumber() == TEAM_TERRORIST ? "resource/UI/BuyMachineguns_TER.res" : "resource/UI/BuyMachineguns_CT.res";
	case MENU_BUY_ITEM:
		return VGuiLegacy_GetTeamNumber() == TEAM_TERRORIST ? "resource/UI/BuyEquipment_TER.res" : "resource/UI/BuyEquipment_CT.res";
	default:
		return NULL;
	}
}

const char *CBuyMenu::ResolveCommandResource(const char *command) const
{
	if (!stricmp(command, "Resource/UI/BuyPistols.res"))
		return GetMenuTypeResource(MENU_BUY_PISTOL);
	if (!stricmp(command, "Resource/UI/BuyShotguns.res"))
		return GetMenuTypeResource(MENU_BUY_SHOTGUN);
	if (!stricmp(command, "Resource/UI/BuySubMachineGuns.res"))
		return GetMenuTypeResource(MENU_BUY_SUBMACHINEGUN);
	if (!stricmp(command, "Resource/UI/BuyRifles.res"))
		return GetMenuTypeResource(MENU_BUY_RIFLE);
	if (!stricmp(command, "Resource/UI/BuyMachineGuns.res"))
		return GetMenuTypeResource(MENU_BUY_MACHINEGUN);
	if (!stricmp(command, "Resource/UI/BuyEquipment.res"))
		return GetMenuTypeResource(MENU_BUY_ITEM);

	return NULL;
}

CBuySubMenu *CBuyMenu::GetOrCreatePage(const char *resourceFile)
{
	if (!resourceFile)
		return NULL;

	for (int i = 0; i < m_Pages.Count(); ++i)
	{
		if (!stricmp(m_Pages[i]->GetResourceFile(), resourceFile))
			return m_Pages[i];
	}

	char pageName[64];
	snprintf(pageName, sizeof(pageName), "BuyPage%d", m_Pages.Count());

	CBuySubMenu *page = new CBuySubMenu(m_pPageHost, pageName, this);
	page->LoadFromResource(resourceFile);
	page->SetVisible(false);
	m_Pages.AddToTail(page);
	InvalidateLayout();
	return page;
}

void CBuyMenu::ShowPage(CBuySubMenu *page)
{
	if (!page)
		return;

	for (int i = 0; i < m_Pages.Count(); ++i)
		m_Pages[i]->SetVisible(false);

	page->SetVisible(true);
	page->MoveToFront();
	m_pCurrentPage = page;
	UpdateAffordability();
}

void CBuyMenu::UpdateMoneyLabel()
{
	m_iLastKnownMoney = GetCurrentMoney();

	if (!m_pMoneyLabel)
		return;

	char buffer[64];
	snprintf(buffer, sizeof(buffer), "$%d", m_iLastKnownMoney);
	m_pMoneyLabel->SetText(buffer);
}

void CBuyMenu::UpdateAffordability()
{
	const int money = GetCurrentMoney();

	for (int i = 0; i < m_Pages.Count(); ++i)
		m_Pages[i]->UpdateAffordability(money);
}

void CBuyMenu::CenterInViewport()
{
	if (!g_pViewport)
		return;

	int x = (g_pViewport->GetWide() - GetWide()) / 2;
	int y = (g_pViewport->GetTall() - GetTall()) / 2;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	SetPos(x, y);
}
