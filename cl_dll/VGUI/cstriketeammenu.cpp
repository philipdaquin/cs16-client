#include "VGUI/cstriketeammenu.h"
#include "VGUI/counterstrikeviewport.h"
#include "VGUI/vgui_legacy_api.h"
#include "cdll_dll.h"

#include <stdio.h>
#include <string.h>
#include <vgui/IInputInternal.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/HTML.h>
#include <vgui_controls/Label.h>

CTeamMenuPanel::CTeamMenuPanel(vgui2::Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	m_iCurrentTeam = 0;

	SetTitleBarVisible(false);
	SetMoveable(false);
	SetSizeable(false);
	SetCloseButtonVisible(false);
	SetMenuButtonVisible(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetVisible(false);
	SetScheme(g_pViewport ? g_pViewport->GetSchemeHandle() : 0);

	LoadControlSettings("resource/UI/Teammenu.res");
	SetSize(640, 480);
}

const char *CTeamMenuPanel::GetName()
{
	return BaseClass::GetName();
}

void CTeamMenuPanel::SetData(KeyValues *data)
{
	(void)data;
}

void CTeamMenuPanel::Reset()
{
	m_iCurrentTeam = 0;
	ShowPanel(false);
}

void CTeamMenuPanel::Update()
{
	m_iCurrentTeam = VGuiLegacy_GetTeamNumber();
	UpdateMapName();
	UpdateButtonStates();
	CenterInViewport();
}

bool CTeamMenuPanel::NeedsUpdate()
{
	return false;
}

bool CTeamMenuPanel::HasInputElements()
{
	return true;
}

void CTeamMenuPanel::ShowPanel(bool state)
{
	if (state)
	{
		Update();
		SetVisible(true);
		SetMouseInputEnabled(true);
		SetKeyBoardInputEnabled(true);
		MoveToFront();
		RequestFocus();
		Activate();
		vgui2::input()->SetAppModalSurface(GetVPanel());
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
		if (vgui2::input()->GetAppModalSurface() == GetVPanel())
			vgui2::input()->SetAppModalSurface(NULL);
	}
}

vgui2::VPANEL CTeamMenuPanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CTeamMenuPanel::IsVisible()
{
	return BaseClass::IsVisible();
}

void CTeamMenuPanel::SetParent(vgui2::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CTeamMenuPanel::OnCommand(const char *command)
{
	if (!stricmp(command, "vguicancel"))
	{
		if (g_pViewport)
			g_pViewport->ShowPanel(PANEL_TEAM, false);
		return;
	}

	if (!strnicmp(command, "jointeam ", 9))
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%s\n", command);
		VGuiLegacy_ClientCmd(buffer);

		if (g_pViewport)
			g_pViewport->ShowPanel(PANEL_TEAM, false);
		return;
	}

	BaseClass::OnCommand(command);
}

void CTeamMenuPanel::OnKeyCodePressed(vgui2::KeyCode code)
{
	if (code == vgui2::KEY_ESCAPE)
	{
		if (g_pViewport)
			g_pViewport->ShowPanel(PANEL_TEAM, false);
		return;
	}

	BaseClass::OnKeyCodePressed(code);
}

void CTeamMenuPanel::UpdateMapName()
{
	vgui2::Label *mapLabel = dynamic_cast<vgui2::Label *>(FindChildByName("mapname", true));
	if (!mapLabel)
		return;

	const char *levelName = VGuiLegacy_GetLevelName();
	if (!levelName || !levelName[0])
		return;

	const char *mapName = levelName;
	if (!strnicmp(mapName, "maps/", 5))
		mapName += 5;

	char buffer[128];
	strncpy(buffer, mapName, sizeof(buffer) - 1);
	buffer[sizeof(buffer) - 1] = '\0';

	char *extension = strrchr(buffer, '.');
	if (extension)
		*extension = '\0';

	mapLabel->SetText(buffer);
	mapLabel->SetVisible(true);
}

void CTeamMenuPanel::UpdateButtonStates()
{
	vgui2::Button *ter = dynamic_cast<vgui2::Button *>(FindChildByName("terbutton", true));
	vgui2::Button *ct = dynamic_cast<vgui2::Button *>(FindChildByName("ctbutton", true));
	vgui2::Button *spec = dynamic_cast<vgui2::Button *>(FindChildByName("specbutton", true));
	vgui2::Button *vip = dynamic_cast<vgui2::Button *>(FindChildByName("vipbutton", true));

	if (ter)
		ter->SetEnabled(m_iCurrentTeam != TEAM_TERRORIST);
	if (ct)
		ct->SetEnabled(m_iCurrentTeam != TEAM_CT);
	if (spec)
		spec->SetEnabled(VGuiLegacy_GetAllowSpectators());
	if (vip)
		vip->SetEnabled(false);
}

void CTeamMenuPanel::CenterInViewport()
{
	if (!g_pViewport)
		return;

	int x = (g_pViewport->GetWide() - GetWide()) / 2;
	int y = (g_pViewport->GetTall() - GetTall()) / 2;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	SetPos(x, y);
}
