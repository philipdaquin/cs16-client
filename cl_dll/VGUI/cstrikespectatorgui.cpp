#include "VGUI/cstrikespectatorgui.h"
#include "VGUI/vgui_legacy_api.h"
#include "cdll_dll.h"

#include <stdio.h>
#include <string.h>
#include <vgui_controls/Label.h>

#define OBS_NONE 0
#define OBS_CHASE_LOCKED 1
#define OBS_CHASE_FREE 2
#define OBS_ROAMING 3
#define OBS_IN_EYE 4
#define OBS_MAP_FREE 5
#define OBS_MAP_CHASE 6

CSpectatorGUI::CSpectatorGUI(vgui2::Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	SetVisible(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetScheme(g_pViewport ? g_pViewport->GetSchemeHandle() : 0);

	LoadControlSettings("resource/UI/Spectator.res");
}

const char *CSpectatorGUI::GetName()
{
	return BaseClass::GetName();
}

void CSpectatorGUI::SetData(KeyValues *data)
{
	(void)data;
}

void CSpectatorGUI::Reset()
{
	SetVisible(false);
}

void CSpectatorGUI::Update()
{
	SetBounds(0, 0, VGuiLegacy_GetScreenWidth(), VGuiLegacy_GetScreenHeight());
	UpdateScoreLabels();
	UpdateTimerLabel();

	const int observerTarget = VGuiLegacy_GetObserverTarget();
	if (observerTarget > 0 && observerTarget <= VGuiLegacy_GetMaxPlayers())
	{
		char buffer[256];
		char playerName[128];
		playerName[0] = '\0';
		VGuiLegacy_GetPlayerName(observerTarget, playerName, sizeof(playerName));
		snprintf(buffer, sizeof(buffer), "%s (%d)", playerName, VGuiLegacy_GetPlayerHealth(observerTarget));
		UpdateLabelText("playerlabel", buffer);
	}
	else
	{
		UpdateLabelText("playerlabel", "");
	}

	switch (VGuiLegacy_GetObserverMode())
	{
	case OBS_CHASE_LOCKED: UpdateLabelText("extrainfo", "Chase Locked"); break;
	case OBS_CHASE_FREE:   UpdateLabelText("extrainfo", "Chase Free"); break;
	case OBS_IN_EYE:       UpdateLabelText("extrainfo", "In Eye"); break;
	case OBS_ROAMING:      UpdateLabelText("extrainfo", "Roaming"); break;
	case OBS_MAP_FREE:     UpdateLabelText("extrainfo", "Map Free"); break;
	case OBS_MAP_CHASE:    UpdateLabelText("extrainfo", "Map Chase"); break;
	default:               UpdateLabelText("extrainfo", ""); break;
	}
}

bool CSpectatorGUI::NeedsUpdate()
{
	return IsVisible();
}

bool CSpectatorGUI::HasInputElements()
{
	return false;
}

void CSpectatorGUI::ShowPanel(bool state)
{
	SetVisible(state);
}

vgui2::VPANEL CSpectatorGUI::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CSpectatorGUI::IsVisible()
{
	return BaseClass::IsVisible();
}

void CSpectatorGUI::SetParent(vgui2::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CSpectatorGUI::UpdateLabelText(const char *controlName, const char *text)
{
	vgui2::Label *label = dynamic_cast<vgui2::Label *>(FindChildByName(controlName, true));
	if (label)
		label->SetText(text);
}

void CSpectatorGUI::UpdateScoreLabels()
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%d", VGuiLegacy_GetTeamFrags(TEAM_CT));
	UpdateLabelText("CTScoreValue", buffer);

	snprintf(buffer, sizeof(buffer), "%d", VGuiLegacy_GetTeamFrags(TEAM_TERRORIST));
	UpdateLabelText("TERScoreValue", buffer);
}

void CSpectatorGUI::UpdateTimerLabel()
{
	int totalSeconds = (int)(VGuiLegacy_GetRoundTime() - VGuiLegacy_GetClientTime());
	if (totalSeconds < 0)
		totalSeconds = 0;

	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%02d:%02d", totalSeconds / 60, totalSeconds % 60);
	UpdateLabelText("timerlabel", buffer);
}
