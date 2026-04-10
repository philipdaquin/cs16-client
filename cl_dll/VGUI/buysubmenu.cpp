#include "buysubmenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include "VGUI/counterstrikeviewport.h"

CBuySubMenu::CBuySubMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
	{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	m_Category = CCounterStrikeViewport::CATEGORY_COUNT;
	m_bIsCT = false;
}

void CBuySubMenu::SetCategory(CCounterStrikeViewport::BuyMenuCategory_t category, bool isCT)
{
	if (m_Category == category && m_bIsCT == isCT)
		return;

	m_Category = category;
	m_bIsCT = isCT;
	LoadControlSettings(GetResourceName());
}

void CBuySubMenu::ApplySchemeSettings(vgui2::HScheme scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
}

void CBuySubMenu::Paint()
{
	BaseClass::Paint();
}

const char *CBuySubMenu::GetResourceName() const
{
	static const char *const kResources[CCounterStrikeViewport::CATEGORY_COUNT][CCounterStrikeViewport::SUBMENU_COUNT] =
	{
		{ "Resource/UI/BuyPistols_TER.res", "Resource/UI/BuyPistols_CT.res" },
		{ "Resource/UI/BuyShotguns_TER.res", "Resource/UI/BuyShotguns_CT.res" },
		{ "Resource/UI/BuySubMachineguns_TER.res", "Resource/UI/BuySubMachineguns_CT.res" },
		{ "Resource/UI/BuyRifles_TER.res", "Resource/UI/BuyRifles_CT.res" },
		{ "Resource/UI/BuyMachineguns_TER.res", "Resource/UI/BuyMachineguns_CT.res" },
		{ "Resource/UI/BuyEquipment_TER.res", "Resource/UI/BuyEquipment_CT.res" }
	};

	if (m_Category < 0 || m_Category >= CCounterStrikeViewport::CATEGORY_COUNT)
		return "Resource/UI/MainBuyMenu.res";

	return kResources[m_Category][m_bIsCT ? CCounterStrikeViewport::SUBMENU_CT : CCounterStrikeViewport::SUBMENU_TER];
}

void CBuySubMenu::OnCommand(const char *command)
{
	if (!command || !command[0])
	{
		BaseClass::OnCommand(command);
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		ClientCmd("cancelselect\n");
		return;
	}

	if (m_pViewport)
		m_pViewport->HideAllGameMenus();

	char szCommand[64];
	snprintf(szCommand, sizeof(szCommand), "%s\n", command);
	ClientCmd(szCommand);
}

#endif
