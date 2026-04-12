#include "buymenu.h"
#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

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
	, m_bControlSettingsLoaded(false)
{
	printf("[VGUI2-CLIENT] CBuyMenu ctor ENTRY this=%p parent=%p name='%s'\n",
		this, parent, panelName ? panelName : "<null>");
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	m_pViewport = NULL;
	printf("[VGUI2-CLIENT] CBuyMenu ctor EXIT this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);
}

void CBuyMenu::EnsureControlSettingsLoaded()
{
	if (m_bControlSettingsLoaded)
		return;

	printf("[VGUI2-CLIENT] CBuyMenu loading control settings\n");
	ReloadControlSettings();
	m_bControlSettingsLoaded = true;
}

void CBuyMenu::ReloadControlSettings()
{
	LoadControlSettings("Resource/UI/BuyMenu.res");
	LoadControlSettings("Resource/UI/MainBuyMenu.res");
}

void CBuyMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
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
