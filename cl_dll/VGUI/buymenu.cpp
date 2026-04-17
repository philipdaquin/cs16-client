#include "buymenu.h"
#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include <vgui_controls/WizardSubPanel.h>

namespace
{

void RunClientCommand(const char *command)
{
	if (!command || !command[0])
		return;

	char szCommand[64];
	snprintf(szCommand, sizeof(szCommand), "%s%s", command, strchr(command, '\n') ? "" : "\n");
	VGUI2_RunClientCommand(szCommand);
}

bool CommandStartsWith(const char *command, const char *prefix)
{
	return !strnicmp(command, prefix, strlen(prefix));
}

}

CBuyMenu::CBuyMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
	, m_pMainMenu(NULL)
	, m_bControlSettingsLoaded(false)
{
	SetScheme("ClientScheme");
	SetProportional(true);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	SetMoveable(false);
	SetSizeable(false);
	SetTitleBarVisible(false);
	SetAutoDelete(false);
	ShowButtons(false);

	LoadControlSettings("Resource/UI/BuyMenu.res");
	CreateMainMenu();
	m_bControlSettingsLoaded = true;
}

CBuyMenu::~CBuyMenu()
{
	if (m_pMainMenu)
		m_pMainMenu->DeleteSubPanels();
}

void CBuyMenu::CreateMainMenu()
{
	if (m_pMainMenu)
		return;

	m_pMainMenu = new CBuySubMenu(this, "mainmenu");
	m_pMainMenu->LoadControlSettings("Resource/UI/MainBuyMenu.res");
	m_pMainMenu->SetVisible(false);
}

void CBuyMenu::EnsureControlSettingsLoaded()
{
	if (m_bControlSettingsLoaded)
		return;

	ReloadControlSettings();
	m_bControlSettingsLoaded = true;
}

void CBuyMenu::ReloadControlSettings()
{
	LoadControlSettings("Resource/UI/BuyMenu.res");
	InvalidateLayout(true, true);
	CreateMainMenu();
	if (m_pMainMenu)
	{
		m_pMainMenu->LoadControlSettings("Resource/UI/MainBuyMenu.res");
		m_pMainMenu->SetVisible(false);
	}
}

void CBuyMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		Update();
		if (m_pMainMenu)
			Run(m_pMainMenu);

		SetMouseInputEnabled(true);
		engine->ClientCmd_Unrestricted("gameui_preventescapetoshow\n");
	}
	else
	{
		engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	if (m_pViewport)
		m_pViewport->ShowBackGround(bShow);
}

void CBuyMenu::Update()
{
}

void CBuyMenu::OnClose()
{
	engine->ClientCmd_Unrestricted("gameui_allowescapetoshow\n");
	BaseClass::OnClose();
	ResetHistory();
}

void CBuyMenu::OnKeyCodePressed(vgui2::KeyCode code)
{
	int nDir = 0;

	switch (code)
	{
	case KEY_XBUTTON_UP:
	case KEY_XSTICK1_UP:
	case KEY_XSTICK2_UP:
	case KEY_UP:
	case STEAMCONTROLLER_DPAD_UP:
		nDir = -1;
		break;

	case KEY_XBUTTON_DOWN:
	case KEY_XSTICK1_DOWN:
	case KEY_XSTICK2_DOWN:
	case KEY_DOWN:
	case STEAMCONTROLLER_DPAD_DOWN:
		nDir = 1;
		break;
	}

	if (nDir != 0)
	{
		Panel *pSubPanel = (GetCurrentSubPanel() ? GetCurrentSubPanel() : m_pMainMenu);

		CUtlSortVector< SortedPanel_t, CSortedPanelYLess > vecSortedButtons;
		VguiPanelGetSortedChildButtonList(pSubPanel, (void*)&vecSortedButtons, "&", 0);

		if (VguiPanelNavigateSortedChildButtonList((void*)&vecSortedButtons, nDir) != -1)
		{
			return;
		}
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}

void CBuyMenu::OnKeyCodeTyped(vgui2::KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		OnClose();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void CBuyMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetBgColor(scheme->GetColor("BgColor", Color(0, 0, 0, 0)));
	SetPaintBackgroundEnabled(true);
}

void CBuyMenu::Paint()
{
	BaseClass::Paint();
}

void CBuyMenu::OnCommand(const char *command)
{
	if (!command || !command[0])
	{
		BaseClass::OnCommand(command);
		return;
	}

	if (CommandStartsWith(command, "Resource/UI/"))
	{
		if (m_pViewport)
		{
			if (strstr(command, "BuyPistols"))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::PISTOL);
			else if (strstr(command, "BuyShotguns"))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::SHOTGUN);
			else if (strstr(command, "BuySubMachineguns") || strstr(command, "BuySubMachineGuns"))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::SUBMACHINEGUN);
			else if (strstr(command, "BuyRifles"))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::RIFLE);
			else if (strstr(command, "BuyMachineguns") || strstr(command, "BuyMachineGuns"))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::MACHINEGUN);
			else if (strstr(command, "BuyEquipment"))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::ITEM);
		}
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();
		VGUI2_RunClientCommand("cancelselect\n");
		return;
	}

	if (CommandStartsWith(command, "autobuy") || CommandStartsWith(command, "rebuy") ||
		CommandStartsWith(command, "primammo") || CommandStartsWith(command, "secammo"))
	{
		RunClientCommand(command);
		return;
	}

	RunClientCommand(command);
}

#endif
