#include "buymenu.h"
#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"

#if !defined(VGUI2_STUB_MODE)

extern cl_enginefunc_t gEngfuncs;

CBuyMenu::CBuyMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	m_pViewport = NULL;
}

void CBuyMenu::ApplySchemeSettings(vgui2::HScheme scheme)
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
	gEngfuncs.Con_Printf("[VGUI2-BUYMENU] OnCommand: %s\n", command);

	if (!command || !command[0])
	{
		BaseClass::OnCommand(command);
		return;
	}

	if (!strnicmp(command, "Resource/UI/", 12))
	{
		gEngfuncs.Con_Printf("[VGUI2-BUYMENU] Opening submenu: %s\n", command);
		if (m_pViewport)
		{
			m_pViewport->HideAllGameMenus();
			bool isCT = VGUI2_GetLocalPlayerTeam() == 2;
			if (!strnicmp(command + 12, "BuyPistols", 10))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::PISTOL);
			else if (!strnicmp(command + 12, "BuyShotguns", 11))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::SHOTGUN);
			else if (!strnicmp(command + 12, "BuySubMachineguns", 16))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::SUBMACHINEGUN);
			else if (!strnicmp(command + 12, "BuyRifles", 9))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::RIFLE);
			else if (!strnicmp(command + 12, "BuyMachineguns", 14))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::MACHINEGUN);
			else if (!strnicmp(command + 12, "BuyEquipment", 12))
				m_pViewport->ShowBuySubMenu(CCounterStrikeViewport::ITEM);
		}
	}
	else if (!strnicmp(command, "vguicancel", 10))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();
		ClientCmd("cancelselect\n");
	}
	else if (!strnicmp(command, "autobuy", 7))
	{
		ClientCmd("autobuy\n");
	}
	else if (!strnicmp(command, "rebuy", 5))
	{
		ClientCmd("rebuy\n");
	}
	else
	{
		ClientCmd(command);
	}

	BaseClass::OnCommand(command);
}

#endif