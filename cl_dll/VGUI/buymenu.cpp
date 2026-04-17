#include "buymenu.h"
#include "buysubmenu.h"
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
	, m_pMainMenu(NULL)
	, m_bControlSettingsLoaded(false)
{
	SetScheme("ClientScheme");
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);
	SetVisible(false);
	SetPaintBackgroundEnabled(false);
	SetMoveable(false);
	SetSizeable(false);
	SetTitleBarVisible(false);
	SetAutoDelete(false);
	m_pViewport = NULL;
	ShowButtons(false);

	m_pMainMenu = new CBuySubMenu(this, "mainmenu");
	if (m_pMainMenu)
	{
		m_pMainMenu->LoadControlSettings("Resource/UI/MainBuyMenu.res");
		m_pMainMenu->SetVisible(false);
	}
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
	if (m_pMainMenu)
		m_pMainMenu->LoadControlSettings("Resource/UI/MainBuyMenu.res");
	InvalidateLayout(true, true);
}

void CBuyMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	SetVisible(bShow);
	SetMouseInputEnabled(bShow);

	if (bShow)
	{
		Update();
		if (m_pMainMenu)
			Run(m_pMainMenu);
		VGUI2_RunClientCommand("gameui_preventescapetoshow\n");
	}
	else
	{
		VGUI2_RunClientCommand("gameui_allowescapetoshow\n");
		ResetHistory();
	}

	if (m_pViewport)
		m_pViewport->ShowBackGround(bShow);
}

void CBuyMenu::Update()
{
}

void CBuyMenu::OnClose()
{
	VGUI2_RunClientCommand("gameui_allowescapetoshow\n");
	BaseClass::OnClose();
	ResetHistory();
}

void CBuyMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
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
