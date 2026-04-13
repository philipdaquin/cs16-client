#include "cstriketeammenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"

namespace
{

void DumpPanelTree(vgui2::Panel *panel, int depth)
{
	if (!panel || depth > 3)
		return;

	int x = 0, y = 0, w = 0, h = 0;
	panel->GetBounds(x, y, w, h);
	printf("[VGUI2-CLIENT] TeamMenuTree depth=%d panel=%p name='%s' visible=%d enabled=%d bounds=(%d,%d %dx%d) children=%d\n",
		depth,
		panel,
		panel->GetName() ? panel->GetName() : "<null>",
		panel->IsVisible() ? 1 : 0,
		panel->IsEnabled() ? 1 : 0,
		x, y, w, h,
		panel->GetChildCount());

	for (int i = 0; i < panel->GetChildCount(); ++i)
	{
		DumpPanelTree(panel->GetChild(i), depth + 1);
	}
}

}

CTeamMenu::CTeamMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
	, m_bControlSettingsLoaded(false)
{
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu ENTRY this=%p parent=%p name='%s'\n",
		this, parent, panelName ? panelName : "<null>");

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-proportional begin this=%p\n", this);
	SetProportional(false);
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-proportional end this=%p\n", this);

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-visible begin this=%p\n", this);
	SetVisible(false);
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-visible end this=%p visible=%d\n",
		this, IsVisible() ? 1 : 0);

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-paint-background begin this=%p\n", this);
	// TEMPORARY ISOLATION:
	// Disable TeamMenu background fill so we can confirm the fullscreen green overlay
	// is coming from panel background painting rather than parser/control creation.
	SetPaintBackgroundEnabled(false);
	// SetPaintBackgroundEnabled(false);
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-paint-background end this=%p\n", this);
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP constructor-final-state this=%p vpanel=%u parent=%p\n",
		this, (unsigned int)GetVPanel(), GetParent());
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu EXIT this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);
}

void CTeamMenu::EnsureControlSettingsLoaded()
{
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded ENTRY this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);

	if (m_bControlSettingsLoaded)
	{
		printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded EARLY-RETURN already-loaded this=%p\n",
			this);
		return;
	}

	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded STEP before-LoadControlSettings this=%p\n",
		this);

	printf("[TEAMTRACE] calling EditablePanel::LoadControlSettings this=%p resource='%s'\n",
		this, "Resource/UI/Teammenu.res");
	BaseClass::LoadControlSettings("Resource/UI/Teammenu.res");
	printf("[TEAMTRACE] returned EditablePanel::LoadControlSettings this=%p\n", this);
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded STEP after-LoadControlSettings this=%p\n",
		this);
	vgui2::Panel *pRootFrame = FindChildByName("TeamMenu");
	if (pRootFrame)
	{
		printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded disabling root frame paint root=%p\n",
			pRootFrame);
		pRootFrame->SetPaintBackgroundEnabled(false);
		pRootFrame->SetPaintBorderEnabled(false);
	}
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded dumping panel tree this=%p\n", this);
	DumpPanelTree(this, 0);
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded STEP child lookup rootframe=%p terbutton=%p ctbutton=%p specbutton=%p mapinfo=%p sysmenu=%p\n",
		pRootFrame,
		FindChildByName("terbutton"),
		FindChildByName("ctbutton"),
		FindChildByName("specbutton"),
		FindChildByName("MapInfo"),
		FindChildByName("SysMenu"));
	m_bControlSettingsLoaded = true;
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded EXIT this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);
}

void CTeamMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	printf("[VGUI2-CLIENT] CTeamMenu::ApplySchemeSettings ENTRY this=%p scheme=%p\n", this, scheme);
	BaseClass::ApplySchemeSettings(scheme);
	// TEMPORARY ISOLATION:
	// Keep background painting disabled after scheme application too, otherwise the
	// scheme pass can re-enable the same fullscreen fill we are isolating.
	// SetPaintBackgroundEnabled(true);
	SetPaintBackgroundEnabled(false);
	printf("[VGUI2-CLIENT] CTeamMenu::ApplySchemeSettings EXIT this=%p\n", this);
}

void CTeamMenu::Paint()
{
	BaseClass::Paint();
}

void CTeamMenu::SetSpectateVisible(bool bVisible)
{
	printf("[VGUI2-CLIENT] CTeamMenu::SetSpectateVisible ENTRY this=%p visible=%d\n",
		this, bVisible ? 1 : 0);
	vgui2::Panel *pSpectateButton = FindChildByName("specbutton");
	if (!pSpectateButton)
	{
		printf("[VGUI2-CLIENT] CTeamMenu::SetSpectateVisible missing specbutton this=%p\n", this);
		return;
	}

	pSpectateButton->SetVisible(bVisible);
	pSpectateButton->SetEnabled(bVisible);
	printf("[VGUI2-CLIENT] CTeamMenu::SetSpectateVisible EXIT this=%p button=%p\n",
		this, pSpectateButton);
}

void CTeamMenu::OnCommand(const char *command)
{
	printf("[VGUI2-CLIENT] CTeamMenu::OnCommand ENTRY this=%p command='%s' viewport=%p\n",
		this, command ? command : "<null>", m_pViewport);
	if (!command || !command[0])
	{
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand empty-command forwarding to base this=%p\n", this);
		BaseClass::OnCommand(command);
		return;
	}

	if (!strnicmp(command, "jointeam", 8))
	{
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand handling jointeam this=%p command='%s'\n",
			this, command);
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		char szCommand[64];
		snprintf(szCommand, sizeof(szCommand), "%s\n", command);
		VGUI2_RunClientCommand(szCommand);
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand issued jointeam command='%s'\n", szCommand);
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand handling vguicancel this=%p\n", this);
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		VGUI2_RunClientCommand("cancelselect\n");
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand issued cancelselect\n");
		return;
	}

	printf("[VGUI2-CLIENT] CTeamMenu::OnCommand forwarding to base command='%s'\n", command);
	BaseClass::OnCommand(command);
}

#endif
