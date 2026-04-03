#include "VGUI/counterstrikeviewport.h"
#include "VGUI/cstrikeclassmenu.h"
#include "VGUI/cstriketeammenu.h"
#include "VGUI/buymenu.h"
#include "VGUI/cstrikespectatorgui.h"
#include "VGUI/vgui_legacy_api.h"
#include "cdll_dll.h"
#include "VGUI/vgui_int.h"

#include <string.h>
#include <vgui/IInputInternal.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

CCounterStrikeViewport::CCounterStrikeViewport() : BaseClass(NULL, "CSViewport")
{
	m_pActivePanel = NULL;
	m_hScheme = 0;

	SetPaintBackgroundEnabled(false);
	SetPaintEnabled(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetVisible(true);

	SetToFullScreen();

	if (g_pVGuiSchemeManager)
		m_hScheme = vgui2::scheme()->GetScheme("ClientScheme");

	if (m_hScheme)
		SetScheme(m_hScheme);
}

CCounterStrikeViewport::~CCounterStrikeViewport()
{
	for (int i = 0; i < m_Panels.Count(); ++i)
		delete m_Panels[i];
}

void CCounterStrikeViewport::SetToFullScreen()
{
	int wide = VGuiLegacy_GetScreenWidth();
	int tall = VGuiLegacy_GetScreenHeight();

	if (g_pVGuiSurface)
		g_pVGuiSurface->GetScreenSize(wide, tall);

	SetPos(0, 0);
	SetSize(wide, tall);
}

void CCounterStrikeViewport::Start()
{
	CreateDefaultPanels();
}

void CCounterStrikeViewport::CreateDefaultPanels()
{
	AddNewPanel(new CTeamMenuPanel(this, PANEL_TEAM));
	AddNewPanel(new CClassMenuPanel(this, PANEL_CLASS_T, false));
	AddNewPanel(new CClassMenuPanel(this, PANEL_CLASS_CT, true));
	AddNewPanel(new CBuyMenu(this, PANEL_BUY));
	AddNewPanel(new CSpectatorGUI(this, PANEL_SPECGUI));
}

void CCounterStrikeViewport::AddNewPanel(IViewportPanel *panel)
{
	if (!panel)
		return;

	panel->SetParent(GetVPanel());
	panel->ShowPanel(false);
	m_Panels.AddToTail(panel);
}

IViewportPanel *CCounterStrikeViewport::FindPanelByName(const char *name)
{
	for (int i = 0; i < m_Panels.Count(); ++i)
	{
		if (!strcmp(m_Panels[i]->GetName(), name))
			return m_Panels[i];
	}

	return NULL;
}

IViewportPanel *CCounterStrikeViewport::GetActivePanel() const
{
	return m_pActivePanel;
}

void CCounterStrikeViewport::ShowPanel(const char *name, bool state)
{
	IViewportPanel *panel = FindPanelByName(name);
	if (!panel)
		return;

	if (state)
	{
		if (m_pActivePanel && m_pActivePanel != panel)
			m_pActivePanel->ShowPanel(false);

		panel->Update();
		panel->ShowPanel(true);
		m_pActivePanel = panel;
	}
	else
	{
		panel->ShowPanel(false);
		if (m_pActivePanel == panel)
			m_pActivePanel = NULL;
	}
}

void CCounterStrikeViewport::ShowPanel(int menuType, bool state)
{
	switch (menuType)
	{
	case MENU_TEAM:
		ShowPanel(PANEL_TEAM, state);
		break;
	case MENU_CLASS_T:
		ShowPanel(PANEL_CLASS_T, state);
		break;
	case MENU_CLASS_CT:
		ShowPanel(PANEL_CLASS_CT, state);
		break;
	case MENU_BUY:
		ShowPanel(PANEL_BUY, state);
		break;
	case MENU_BUY_PISTOL:
	case MENU_BUY_SHOTGUN:
	case MENU_BUY_RIFLE:
	case MENU_BUY_SUBMACHINEGUN:
	case MENU_BUY_MACHINEGUN:
	case MENU_BUY_ITEM:
	{
		CBuyMenu *buyMenu = static_cast<CBuyMenu *>(FindPanelByName(PANEL_BUY));
		if (buyMenu)
		{
			buyMenu->SetActiveSubMenu(menuType);
			ShowPanel(PANEL_BUY, state);
		}
		break;
	}
	default:
		break;
	}
}

void CCounterStrikeViewport::HideAllPanels()
{
	for (int i = 0; i < m_Panels.Count(); ++i)
		m_Panels[i]->ShowPanel(false);

	m_pActivePanel = NULL;
	vgui2::input()->SetAppModalSurface(NULL);
}

void CCounterStrikeViewport::Reset()
{
	HideAllPanels();

	for (int i = 0; i < m_Panels.Count(); ++i)
		m_Panels[i]->Reset();
}

void CCounterStrikeViewport::UpdateAllPanels()
{
	for (int i = 0; i < m_Panels.Count(); ++i)
	{
		if (m_Panels[i]->IsVisible() || m_Panels[i]->NeedsUpdate())
			m_Panels[i]->Update();
	}
}

vgui2::HScheme CCounterStrikeViewport::GetSchemeHandle() const
{
	return m_hScheme;
}

void CCounterStrikeViewport::Paint()
{
}

void CCounterStrikeViewport::OnThink()
{
	BaseClass::OnThink();
	SetToFullScreen();

	CSpectatorGUI *spec = static_cast<CSpectatorGUI *>(FindPanelByName(PANEL_SPECGUI));
	if (spec)
		spec->ShowPanel(VGuiLegacy_GetObserverMode() != 0);

	UpdateAllPanels();
}

void CCounterStrikeViewport::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}
