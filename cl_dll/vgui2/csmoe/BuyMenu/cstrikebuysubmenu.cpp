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
#include "vgui_controls/ImagePanel.h"

using namespace vgui2;

static bool EndsWith(const char *text, const char *suffix)
{
	if (!text || !suffix)
		return false;

	const int textLen = Q_strlen(text);
	const int suffixLen = Q_strlen(suffix);
	if (suffixLen > textLen)
		return false;

	return !Q_stricmp(text + textLen - suffixLen, suffix);
}

static void LowerString(char *text)
{
	if (!text)
		return;

	for (char *p = text; *p; ++p)
	{
		if (*p >= 'A' && *p <= 'Z')
			*p = static_cast<char>(*p - 'A' + 'a');
	}
}

static Panel *FindWeaponCardAnchorForBase(Panel *panel, const char *baseName)
{
	if (!panel || !baseName || !baseName[0])
		return NULL;

	const char *suffixes[] = { "-bg", "-img", "-bgwpn" };
	for (int i = 0; i < ARRAYSIZE(suffixes); ++i)
	{
		char anchorName[160];
		Q_snprintf(anchorName, sizeof(anchorName), "%s%s", baseName, suffixes[i]);

		Panel *anchor = panel->FindChildByName(anchorName);
		if (anchor)
			return anchor;
	}

	return NULL;
}

static Panel *FindWeaponCardAnchor(Panel *panel, const char *buttonName)
{
	if (!panel || !buttonName || !buttonName[0])
		return NULL;

	char baseName[128];
	Q_strncpy(baseName, buttonName, sizeof(baseName));

	if (EndsWith(baseName, "-bottom"))
		baseName[Q_strlen(baseName) - Q_strlen("-bottom")] = 0;
	else if (EndsWith(baseName, "-auto-bottom"))
		baseName[Q_strlen(baseName) - Q_strlen("-auto-bottom")] = 0;
	else
		return NULL;

	return FindWeaponCardAnchorForBase(panel, baseName);
}

static const char *GetButtonCommandString(Button *button)
{
	if (!button)
		return "";

	KeyValues *command = button->GetCommand();
	if (!command)
		return "";

	return command->GetString("command", "");
}

static const char *BaseNameFromPath(const char *path)
{
	if (!path)
		return "";

	const char *baseName = Q_strrchr(path, '/');
	return baseName ? baseName + 1 : path;
}

static bool IsRealWeaponImage(const char *imageName)
{
	if (!imageName || !imageName[0])
		return false;

	return Q_strnicmp(imageName, "gfx/vgui/", Q_strlen("gfx/vgui/")) == 0
		&& Q_stricmp(BaseNameFromPath(imageName), "noimg");
}

static const char *GetBuiltInBuyCommand(const char *weaponName)
{
	struct BuiltInBuyCommand_t
	{
		const char *weaponName;
		const char *command;
	};

	static const BuiltInBuyCommand_t commands[] =
	{
		{ "glock18", "glock" },
		{ "usp45", "usp" },
		{ "p228", "p228" },
		{ "deserteagle", "deagle" },
		{ "fiveseven", "fn57" },
		{ "elite", "elites" },
		{ "elites", "elites" },
		{ "m3", "m3" },
		{ "xm1014", "xm1014" },
		{ "tmp", "tmp" },
		{ "mac10", "mac10" },
		{ "mp5", "mp5" },
		{ "ump45", "ump45" },
		{ "p90", "p90" },
		{ "galil", "galil" },
		{ "famas", "famas" },
		{ "ak47", "ak47" },
		{ "scout", "scout" },
		{ "m4a1", "m4a1" },
		{ "sg552", "sg552" },
		{ "aug", "aug" },
		{ "sg550", "sg550" },
		{ "awp", "awp" },
		{ "g3sg1", "g3sg1" },
		{ "m249", "m249" },
	};

	for (int i = 0; i < ARRAYSIZE(commands); ++i)
	{
		if (!Q_stricmp(weaponName, commands[i].weaponName))
			return commands[i].command;
	}

	return NULL;
}

static void BuildBuyCommandFromImage(const char *imageName, char *command, int commandSize)
{
	if (!command || commandSize <= 0)
		return;

	command[0] = 0;

	if (!IsRealWeaponImage(imageName))
		return;

	char weaponName[64];
	Q_strncpy(weaponName, BaseNameFromPath(imageName), sizeof(weaponName));
	LowerString(weaponName);

	const int weaponNameLen = Q_strlen(weaponName);
	if (weaponNameLen > 2 && !Q_stricmp(weaponName + weaponNameLen - 2, "_1"))
		weaponName[weaponNameLen - 2] = 0;

	const char *builtInCommand = GetBuiltInBuyCommand(weaponName);
	if (builtInCommand)
	{
		Q_strncpy(command, builtInCommand, commandSize);
		return;
	}

	Q_snprintf(command, commandSize, "nst_buy_mywpn %s", weaponName);
}

static Button *FindOrCreateWeaponCardButton(CCSBuySubMenu *owner, Panel *panel, const char *baseName)
{
	if (!owner || !panel || !baseName || !baseName[0])
		return NULL;

	char buttonName[160];
	Q_snprintf(buttonName, sizeof(buttonName), "%s-bottom", baseName);

	Button *button = dynamic_cast<Button *>(panel->FindChildByName(buttonName));
	if (button)
		return button;

	char autoButtonName[192];
	Q_snprintf(autoButtonName, sizeof(autoButtonName), "%s-auto-bottom", baseName);
	button = new Button(panel, autoButtonName, "", owner);
	button->SetPaintBackgroundEnabled(false);
	button->SetPaintBorderEnabled(false);
	button->SetPaintEnabled(false);
	return button;
}

static void EnsureWeaponCardButtons(CCSBuySubMenu *owner, Panel *panel)
{
	if (!owner || !panel)
		return;

	for (int i = 0; i < panel->GetChildCount(); ++i)
	{
		Panel *child = panel->GetChild(i);
		ImagePanel *imagePanel = dynamic_cast<ImagePanel *>(child);
		if (!imagePanel || !EndsWith(imagePanel->GetName(), "-img"))
			continue;

		const char *imageName = imagePanel->GetImageName();
		if (!IsRealWeaponImage(imageName))
			continue;

		char baseName[128];
		Q_strncpy(baseName, imagePanel->GetName(), sizeof(baseName));
		baseName[Q_strlen(baseName) - Q_strlen("-img")] = 0;

		Button *button = FindOrCreateWeaponCardButton(owner, panel, baseName);
		if (!button)
			continue;

		if (!GetButtonCommandString(button)[0])
		{
			char command[96];
			BuildBuyCommandFromImage(imageName, command, sizeof(command));
			if (command[0])
				button->SetCommand(command);
		}

		Panel *anchor = FindWeaponCardAnchorForBase(panel, baseName);
		if (!anchor)
			anchor = imagePanel;

		int x, y, wide, tall;
		anchor->GetBounds(x, y, wide, tall);
		button->SetBounds(x, y, wide, tall);
		button->SetVisible(true);
		button->SetEnabled(true);
		button->SetMouseInputEnabled(true);
		button->SetKeyBoardInputEnabled(true);
		button->MoveToFront();
	}

	for (int i = 0; i < panel->GetChildCount(); ++i)
	{
		Panel *child = panel->GetChild(i);
		if (child)
			EnsureWeaponCardButtons(owner, child);
	}
}

static void AlignBuySubMenuButtonsToCards(Panel *panel)
{
	if (!panel)
		return;

	for (int i = 0; i < panel->GetChildCount(); ++i)
	{
		Panel *child = panel->GetChild(i);
		Button *button = dynamic_cast<Button *>(child);
		if (!button)
			continue;

		Panel *anchor = FindWeaponCardAnchor(panel, button->GetName());
		if (!anchor)
			continue;

		if (!GetButtonCommandString(button)[0])
		{
			button->SetMouseInputEnabled(false);
			button->SetKeyBoardInputEnabled(false);
			continue;
		}

		int x, y, wide, tall;
		anchor->GetBounds(x, y, wide, tall);
		button->SetBounds(x, y, wide, tall);
		button->SetVisible(true);
		button->SetEnabled(true);
		button->SetMouseInputEnabled(true);
		button->SetKeyBoardInputEnabled(true);
		button->MoveToFront();
	}

	for (int i = 0; i < panel->GetChildCount(); ++i)
	{
		Panel *child = panel->GetChild(i);
		if (child)
			AlignBuySubMenuButtonsToCards(child);
	}
}

static void AlignBuySubMenuCancelButton(Panel *panel)
{
	if (!panel)
		return;

	Button *button = dynamic_cast<Button *>(panel->FindChildByName("CancelButton"));
	if (!button)
		return;

	Panel *anchor = panel->FindChildByName("CancelButton-bg");
	if (!anchor)
		anchor = panel->FindChildByName("CancelButton-key");
	if (!anchor)
		return;

	int x, y, wide, tall;
	anchor->GetBounds(x, y, wide, tall);
	button->SetBounds(x, y, wide, tall);
	button->SetVisible(true);
	button->SetEnabled(true);
	button->SetMouseInputEnabled(true);
	button->SetKeyBoardInputEnabled(true);
}

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

	BaseClass::PerformLayout();
	EnsureWeaponCardButtons(this, this);
	AlignBuySubMenuButtonsToCards(this);
	AlignBuySubMenuCancelButton(this);

	// Keep the shared viewport background visible whenever this submenu lays out.
	if (g_pViewport)
		g_pViewport->ShowBackGround(true);
}

MouseOverPanelButton *CCSBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new BuyMouseOverPanelButton(this, nullptr, panel);
}

CBuySubMenu *CCSBuySubMenu::CreateNewSubMenu(const char *name)
{
	return new CCSBuySubMenu(this, name);
}
