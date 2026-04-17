#include "buysubmenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include <KeyValues.h>
#include <vgui_controls/WizardPanel.h>

#include "buymouseoverpanelbutton.h"
#include "VGUI/counterstrikeviewport_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui2;

CBuySubMenu::CBuySubMenu(vgui2::Panel *parent, const char *name)
	: BaseClass(parent, name)
	, m_pViewport(NULL)
	, m_pPanel(NULL)
	, m_pFirstButton(NULL)
	, m_NextPanel(NULL)
	, m_Category(CCounterStrikeViewport::CATEGORY_COUNT)
	, m_bIsCT(false)
	, m_bControlSettingsLoaded(false)
{
	SetProportional(true);

	m_pPanel = new EditablePanel(this, "ItemInfo");
	m_pPanel->SetProportional(true);
}

CBuySubMenu::~CBuySubMenu()
{
}

void CBuySubMenu::SetCategory(CCounterStrikeViewport::BuyMenuCategory_t category, bool isCT)
{
	if (m_Category == category && m_bIsCT == isCT)
		return;

	m_Category = category;
	m_bIsCT = isCT;
	m_bControlSettingsLoaded = false;
}

Panel *CBuySubMenu::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp("MouseOverPanelButton", controlName))
	{
		MouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pPanel);
		if (!m_pFirstButton)
			m_pFirstButton = newButton;
		return newButton;
	}

	return BaseClass::CreateControlByName(controlName);
}

MouseOverPanelButton *CBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

CBuySubMenu *CBuySubMenu::CreateNewSubMenu()
{
	return new CBuySubMenu(this, "BuySubMenu");
}

void CBuySubMenu::EnsureControlSettingsLoaded()
{
	if (m_bControlSettingsLoaded)
		return;

	LoadControlSettings(GetResourceName());
	InvalidateLayout(true, true);
	m_bControlSettingsLoaded = true;
}

void CBuySubMenu::ShowPanel(bool bShow)
{
	SetVisible(bShow);
	SetMouseInputEnabled(bShow);
	SetKeyBoardInputEnabled(bShow);

	if (bShow)
	{
		EnsureControlSettingsLoaded();
		InvalidateLayout(true, true);
	}
}

void CBuySubMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	for (int i = 0; i < GetChildCount(); ++i)
	{
		MouseOverPanelButton *buyButton = dynamic_cast<MouseOverPanelButton *>(GetChild(i));
		if (!buyButton)
			continue;

		if (buyButton == m_pFirstButton && state)
			buyButton->ShowPage();
		else
			buyButton->HidePage();

		buyButton->InvalidateLayout();
	}
}

void CBuySubMenu::DeleteSubPanels()
{
	if (m_NextPanel)
	{
		m_NextPanel->SetVisible(false);
		m_NextPanel = NULL;
	}

	m_pFirstButton = NULL;
}

vgui2::WizardSubPanel *CBuySubMenu::GetNextSubPanel()
{
	return m_NextPanel;
}

void CBuySubMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
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

	if (Q_strstr(command, ".res"))
	{
		int i;
		for (i = 0; i < m_SubMenus.Count(); ++i)
		{
			if (!Q_stricmp(m_SubMenus[i].filename, command))
			{
				m_NextPanel = m_SubMenus[i].panel;
				Assert(m_NextPanel);
				m_NextPanel->InvalidateLayout();
				break;
			}
		}

		if (i == m_SubMenus.Count())
		{
			SubMenuEntry_t newEntry;
			memset(&newEntry, 0, sizeof(newEntry));

			CBuySubMenu *newMenu = CreateNewSubMenu();
			newMenu->LoadControlSettings(command);
			m_NextPanel = newMenu;
			Q_strncpy(newEntry.filename, command, sizeof(newEntry.filename));
			newEntry.panel = newMenu;
			m_SubMenus.AddToTail(newEntry);
		}

		GetWizardPanel()->OnNextButton();
		return;
	}

	GetWizardPanel()->Close();
	if (m_pViewport)
		m_pViewport->ShowBackGround(false);

	if (Q_stricmp(command, "vguicancel") != 0)
		VGUI2_RunClientCommand(command);

	BaseClass::OnCommand(command);
}

#endif
