#include "buysubmenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"
#include "mouseoverpanelbutton.h"

CBuySubMenu::CBuySubMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
	, m_Category(CCounterStrikeViewport::CATEGORY_COUNT)
	, m_bIsCT(false)
	, m_bControlSettingsLoaded(false)
	, m_pPanel(NULL)
	, m_pFirstButton(NULL)
	, m_NextPanel(NULL)
{
	SetScheme("ClientScheme");
	SetProportional(true);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);

	m_pPanel = new vgui2::EditablePanel(this, "ItemInfo");
	m_pPanel->SetProportional(true);
}

CBuySubMenu::~CBuySubMenu()
{
}

vgui2::Panel *CBuySubMenu::CreateControlByName(const char *controlName)
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

void CBuySubMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	for (int i = 0; i < GetChildCount(); i++)
	{
		MouseOverPanelButton *buyButton = dynamic_cast<MouseOverPanelButton *>(GetChild(i));
		if (buyButton)
		{
			if (buyButton == m_pFirstButton && state)
				buyButton->ShowPage();
			else
				buyButton->HidePage();

			buyButton->InvalidateLayout();
		}
	}
}

CBuySubMenu *CBuySubMenu::CreateNewSubMenu()
{
	return new CBuySubMenu(this);
}

MouseOverPanelButton *CBuySubMenu::CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

void CBuySubMenu::SetCategory(CCounterStrikeViewport::BuyMenuCategory_t category, bool isCT)
{
	if (m_Category == category && m_bIsCT == isCT)
		return;

	m_Category = category;
	m_bIsCT = isCT;
	m_bControlSettingsLoaded = false;
}

void CBuySubMenu::EnsureControlSettingsLoaded()
{
	if (m_bControlSettingsLoaded)
		return;

	LoadControlSettings(GetResourceName());
	InvalidateLayout(true, true);
	m_bControlSettingsLoaded = true;
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

void CBuySubMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		SetVisible(true);
		SetMouseInputEnabled(true);
		Activate();
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	if (m_pViewport)
		m_pViewport->ShowBackGround(bShow);
}

vgui2::WizardSubPanel *CBuySubMenu::GetNextSubPanel()
{
	return m_NextPanel;
}

void CBuySubMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetBgColor(scheme->GetColor("BgColor", Color(0, 0, 0, 0)));
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

	if (Q_strstr(command, ".res"))
	{
		int i;
		for (i = 0; i < m_SubMenus.Count(); i++)
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
			memset(&newEntry, 0x0, sizeof(newEntry));

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

	if (Q_stricmp(command, "vguicancel") != 0)
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();
		VGUI2_RunClientCommand(command);
	}
	else if (m_pViewport)
	{
		m_pViewport->HideAllGameMenus();
		VGUI2_RunClientCommand("cancelselect\n");
	}

	BaseClass::OnCommand(command);
}

#endif
