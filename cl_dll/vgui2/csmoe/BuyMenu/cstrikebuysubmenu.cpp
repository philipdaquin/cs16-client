#include "hud.h"
#include "../../CBaseViewport.h"
#include "../../CBackGroundPanel.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "buy_presets.h"
#include "cstrikebuymenu.h"
#include "cstrikebuysubmenu.h"
#include "../vgui_int.h"
#include "../../vgui_resource_paths.h"
#include "shared_util.h"

using namespace vgui2;

static const char *ResolveTeamBuyResource(const char *command)
{
	if (!command || !g_pViewport)
		return command;

	const CCSBaseBuyMenu *buyMenu = dynamic_cast<const CCSBaseBuyMenu *>(g_pViewport->GetActivePanel());
	const int team = buyMenu ? buyMenu->GetTeam() : TEAM_TERRORIST;
	const bool isTerrorist = (team == TEAM_TERRORIST);

	if (!Q_stricmp(command, "Resource/UI/BuyPistols.res"))
		return isTerrorist ? vgui2::resource_paths::kMenuBuyPistolsTER : vgui2::resource_paths::kMenuBuyPistolsCT;
	if (!Q_stricmp(command, "Resource/UI/BuyShotguns.res"))
		return isTerrorist ? vgui2::resource_paths::kMenuBuyShotgunsTER : vgui2::resource_paths::kMenuBuyShotgunsCT;
	if (!Q_stricmp(command, "Resource/UI/BuySubMachineGuns.res"))
		return isTerrorist ? vgui2::resource_paths::kMenuBuySubMachinegunsTER : vgui2::resource_paths::kMenuBuySubMachinegunsCT;
	if (!Q_stricmp(command, "Resource/UI/BuyRifles.res"))
		return isTerrorist ? vgui2::resource_paths::kMenuBuyRiflesTER : vgui2::resource_paths::kMenuBuyRiflesCT;
	if (!Q_stricmp(command, "Resource/UI/BuyMachineGuns.res"))
		return isTerrorist ? vgui2::resource_paths::kMenuBuyMachinegunsTER : vgui2::resource_paths::kMenuBuyMachinegunsCT;
	if (!Q_stricmp(command, "Resource/UI/BuyEquipment.res"))
		return isTerrorist ? vgui2::resource_paths::kMenuBuyEquipmentTER : vgui2::resource_paths::kMenuBuyEquipmentCT;

	return command;
}

CCSBuySubMenu::CCSBuySubMenu(vgui2::Panel *parent, const char *name)
	: CBuySubMenu(parent, name)
{
	m_backgroundLayoutFinished = false;
}

void CCSBuySubMenu::OnCommand(const char *command)
{
	if (!command)
		return;

	const char *buyPresetSetString = "cl_buy_favorite_query_set ";
	if (!Q_strnicmp(command, buyPresetSetString, Q_strlen(buyPresetSetString)))
	{
		char setCommand[64];
		Q_snprintf(setCommand, sizeof(setCommand), "cl_buy_favorite_set %d", Q_atoi(command + Q_strlen(buyPresetSetString)));
		BaseClass::OnCommand(setCommand);
		return;
	}

	if (!Q_stricmp(command, "buy_unavailable"))
	{
		BaseClass::OnCommand("vguicancel");
		return;
	}

	if (Q_strstr(command, ".res"))
	{
		const char *resolvedCommand = ResolveTeamBuyResource(command);
		if (resolvedCommand != command)
		{
			gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSBuySubMenu::OnCommand resolved resource='%s' -> '%s'\n",
				command, resolvedCommand);
		}

		BaseClass::OnCommand(resolvedCommand);
		return;
	}

	BaseClass::OnCommand(command);
}

void CCSBuySubMenu::OnSizeChanged(int newWide, int newTall)
{
	m_backgroundLayoutFinished = false;
	BaseClass::OnSizeChanged(newWide, newTall);
}

void CCSBuySubMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	int screenW, screenH;
	GetHudSize(screenW, screenH);

	int fullW = scheme()->GetProportionalScaledValueEx(GetScheme(), 640);
	int fullH = scheme()->GetProportionalScaledValueEx(GetScheme(), 480);

	fullW = GetAlternateProportionalValueFromScaled(GetScheme(), fullW);
	fullH = GetAlternateProportionalValueFromScaled(GetScheme(), fullH);

	const int offsetX = (screenW - fullW) / 2;
	const int offsetY = (screenH - fullH) / 2;

	if (!m_backgroundLayoutFinished)
		ResizeWindowControls(this, GetTall(), GetWide(), offsetX, offsetY);
	m_backgroundLayoutFinished = true;
}

MouseOverPanelButton *CCSBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new BuyMouseOverPanelButton(this, nullptr, panel);
}

CBuySubMenu *CCSBuySubMenu::CreateNewSubMenu(const char *name)
{
	return new CCSBuySubMenu(this, name);
}
