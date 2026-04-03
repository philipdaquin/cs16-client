#include "VGUI/cstrikeclassmenu.h"
#include "VGUI/counterstrikeviewport.h"
#include "VGUI/vgui_legacy_api.h"

#include <stdio.h>
#include <string.h>

#include <vgui/IInputInternal.h>
#include <vgui_controls/Controls.h>

CClassMenuPanel::CClassMenuPanel(vgui2::Panel *parent, const char *panelName, bool isCt) : BaseClass(parent, panelName), m_bIsCt(isCt)
{
	SetTitleBarVisible(false);
	SetMoveable(false);
	SetSizeable(false);
	SetCloseButtonVisible(false);
	SetMenuButtonVisible(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetVisible(false);
	SetScheme(g_pViewport ? g_pViewport->GetSchemeHandle() : 0);

	LoadControlSettings(m_bIsCt ? "resource/UI/Classmenu_CT.res" : "resource/UI/Classmenu_TER.res");
	SetSize(640, 480);
}

const char *CClassMenuPanel::GetName()
{
	return BaseClass::GetName();
}

void CClassMenuPanel::SetData(KeyValues *data)
{
	(void)data;
}

void CClassMenuPanel::Reset()
{
	ShowPanel(false);
}

void CClassMenuPanel::Update()
{
	CenterInViewport();
}

bool CClassMenuPanel::NeedsUpdate()
{
	return false;
}

bool CClassMenuPanel::HasInputElements()
{
	return true;
}

void CClassMenuPanel::ShowPanel(bool state)
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

vgui2::VPANEL CClassMenuPanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CClassMenuPanel::IsVisible()
{
	return BaseClass::IsVisible();
}

void CClassMenuPanel::SetParent(vgui2::VPANEL parent)
{
	BaseClass::SetParent(parent);
}

void CClassMenuPanel::OnCommand(const char *command)
{
	if (!stricmp(command, "vguicancel"))
	{
		if (g_pViewport)
			g_pViewport->ShowPanel(m_bIsCt ? PANEL_CLASS_CT : PANEL_CLASS_T, false);
		return;
	}

	if (!strnicmp(command, "joinclass ", 10))
	{
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%s\n", command);
		VGuiLegacy_ClientCmd(buffer);

		if (g_pViewport)
			g_pViewport->ShowPanel(m_bIsCt ? PANEL_CLASS_CT : PANEL_CLASS_T, false);
		return;
	}

	BaseClass::OnCommand(command);
}

void CClassMenuPanel::OnKeyCodePressed(vgui2::KeyCode code)
{
	if (code == vgui2::KEY_ESCAPE)
	{
		if (g_pViewport)
			g_pViewport->ShowPanel(m_bIsCt ? PANEL_CLASS_CT : PANEL_CLASS_T, false);
		return;
	}

	BaseClass::OnKeyCodePressed(code);
}

void CClassMenuPanel::CenterInViewport()
{
	if (!g_pViewport)
		return;

	int x = (g_pViewport->GetWide() - GetWide()) / 2;
	int y = (g_pViewport->GetTall() - GetTall()) / 2;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	SetPos(x, y);
}
