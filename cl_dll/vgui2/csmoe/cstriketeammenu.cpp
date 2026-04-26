#include <cstdio>

#include "hud.h"
#include "cstriketeammenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <cl_entity.h>
#include <cdll_dll.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/RichText.h>

#include "../CBaseViewport.h"
#include "shared_util.h"
#include "igbutton.h"

using namespace vgui2;

using cl::gEngfuncs;
using cl::g_PlayerExtraInfo;
using cl::gHUD;
using cl::g_iTeamNumber;

CCSTeamMenu::CCSTeamMenu(IViewport* pViewPort) : CTeamMenu(pViewPort)
{
	std::fprintf(stderr, "[phase3][VGUI2-TRACE] CCSTeamMenu::CCSTeamMenu entry this=%p viewport=%p\n",
		this, (void *)pViewPort);
	gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CCSTeamMenu::CCSTeamMenu this=%p viewport=%p\n",
		this, (void *)pViewPort);
}

void CCSTeamMenu::SetupControlSettings()
{
	std::fprintf(stderr, "[phase3][VGUI2-TRACE] CCSTeamMenu::SetupControlSettings entry this=%p\n", this);
	gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CCSTeamMenu::SetupControlSettings this=%p loading resource/UI/Teammenu.res pathID=GAME\n",
		this);
	// Old CSO-specific layout:
	// LoadControlSettings("Resource/UI/CSO_TeamMenu.res", "GAME");
	LoadControlSettings("resource/UI/Teammenu.res", "GAME");
	gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CCSTeamMenu::SetupControlSettings complete this=%p\n", this);
	std::fprintf(stderr, "[phase3][VGUI2-TRACE] CCSTeamMenu::SetupControlSettings exit this=%p\n", this);
	InvalidateLayout();
}

CCSTeamMenu::~CCSTeamMenu(void)
{
}

void CCSTeamMenu::ShowPanel(bool bShow)
{
	std::fprintf(stderr, "[phase4][VGUI2-TRACE] CCSTeamMenu::ShowPanel entry this=%p show=%d\n",
		this, bShow ? 1 : 0);
	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CCSTeamMenu::ShowPanel this=%p show=%d\n",
		this, bShow ? 1 : 0);
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		{
			gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CCSTeamMenu::ShowPanel blocked this=%p because intermission/spectate\n",
				this);
			return;
		}
	}

	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CCSTeamMenu::ShowPanel forwarding to CTeamMenu this=%p show=%d visible=%d\n",
		this, bShow ? 1 : 0, BaseClass::IsVisible() ? 1 : 0);
	BaseClass::ShowPanel(bShow);

	// if (bShow)
	// {
	// 	// Debug visibility probe: make the TeamMenu frame impossible to miss while
	// 	// we verify the actual paint path on the client.
	// 	SetBgColor(Color(0, 160, 255, 255));
	// 	SetPaintBackgroundEnabled(true);
	// 	SetPaintEnabled(true);
	// 	SetPaintBorderEnabled(true);
	// 	SetTitleBarVisible(true);
	// }

	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CCSTeamMenu::ShowPanel done this=%p show=%d visible=%d\n",
		this, bShow ? 1 : 0, BaseClass::IsVisible() ? 1 : 0);
	std::fprintf(stderr, "[phase4][VGUI2-TRACE] CCSTeamMenu::ShowPanel exit this=%p show=%d visible=%d\n",
		this, bShow ? 1 : 0, BaseClass::IsVisible() ? 1 : 0);
}

void CCSTeamMenu::Update(void)
{
	// gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::Update this=%p\n", this);
	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CCSTeamMenu::Update entry this=%p visible=%d\n",
	// 	this, BaseClass::IsVisible() ? 1 : 0);
	BaseClass::Update();

	if (g_pViewport->GetAllowSpectators())
	{
		if (g_iTeamNumber == TEAM_UNASSIGNED || (g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].dead))
			SetVisibleButton("specbutton", true);
		else
			SetVisibleButton("specbutton", false);
	}
	else
		SetVisibleButton("specbutton", false);

	m_bVIPMap = false;

	char mapName[32];
	Q_FileBase(gEngfuncs.pfnGetLevelName(), mapName, sizeof(mapName));

	if (!Q_strncmp(mapName, "maps/as_", 8))
		m_bVIPMap = true;

	if (!m_bVIPMap || (g_iTeamNumber != TEAM_CT))
		SetVisibleButton("vipbutton", false);
	else
		SetVisibleButton("vipbutton", true);

	if (g_iTeamNumber == TEAM_UNASSIGNED)
		SetVisibleButton("CancelButton", false);
	else
		SetVisibleButton("CancelButton", true);

	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CCSTeamMenu::Update exit this=%p visible=%d team=%d\n",
	// 	this, BaseClass::IsVisible() ? 1 : 0, g_iTeamNumber);
}

void CCSTeamMenu::SetVisible(bool state)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::SetVisible this=%p state=%d\n",
		this, state ? 1 : 0);
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *pAutoButton = FindChildByName("autobutton");

		if (pAutoButton)
			pAutoButton->RequestFocus();
	}
}

void CCSTeamMenu::OnCommand(const char *command)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::OnCommand this=%p command='%s'\n",
		this, command ? command : "<null>");
	if (Q_stricmp(command, "vguicancel"))
	{
		cl::gEngfuncs.pfnClientCmd(const_cast<char *>(command));
	}

	Close();

	m_pViewPort->ShowBackGround(false);

	BaseClass::OnCommand(command);
}

void CCSTeamMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	// gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::SetVisibleButton this=%p entry='%s' state=%d\n",
	// 	this, textEntryName ? textEntryName : "<null>", state ? 1 : 0);
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetVisible(state);
}

void CCSTeamMenu::PaintBackground(void)
{
	// gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::PaintBackground enter this=%p visible=%d\n",
	// 	this, IsVisible() ? 1 : 0);
	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CCSTeamMenu::PaintBackground entry this=%p visible=%d\n",
	// 	this, IsVisible() ? 1 : 0);
	// cl::gEngfuncs.Con_Printf("[phase5][VGUI2-CLIENT] CCSTeamMenu::PaintBackground probe start this=%p visible=%d\n",
	// 	this, IsVisible() ? 1 : 0);

	// Temporary paint probe removed.
	// The old bright magenta test rectangle is left here commented out in case
	// we need to re-enable it for debugging later.
	// vgui2::surface()->DrawSetColor(255, 0, 255, 255);
	// vgui2::surface()->DrawFilledRect(24, 24, 320, 160);
	// vgui2::surface()->DrawSetColor(255, 255, 255, 255);
	// vgui2::surface()->DrawOutlinedRect(24, 24, 320, 160);
	// cl::gEngfuncs.pfnFillRGBA(24, 24, 296, 136, 255, 0, 255, 255);
	// cl::gEngfuncs.pfnFillRGBA(24, 24, 296, 1, 255, 255, 255, 255);
	// cl::gEngfuncs.pfnFillRGBA(24, 159, 296, 1, 255, 255, 255, 255);
	// cl::gEngfuncs.pfnFillRGBA(24, 24, 1, 136, 255, 255, 255, 255);
	// cl::gEngfuncs.pfnFillRGBA(319, 24, 1, 136, 255, 255, 255, 255);

	BaseClass::PaintBackground();
	// gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::PaintBackground exit this=%p visible=%d\n",
	// 	this, IsVisible() ? 1 : 0);
	// std::fprintf(stderr, "[phase5][VGUI2-TRACE] CCSTeamMenu::PaintBackground exit this=%p visible=%d\n",
	// 	this, IsVisible() ? 1 : 0);
}

void CCSTeamMenu::PerformLayout(void)
{
	// gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::PerformLayout this=%p visible=%d\n",
	// 	this, IsVisible() ? 1 : 0);
	BaseClass::PerformLayout();
}

void CCSTeamMenu::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::ApplySchemeSettings this=%p scheme=%p\n",
		this, (void *)pScheme);
	BaseClass::ApplySchemeSettings(pScheme);
}

vgui2::Panel* CCSTeamMenu::CreateControlByName(const char* controlName)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::CreateControlByName this=%p control='%s'\n",
		this, controlName ? controlName : "<null>");
	if (!Q_stricmp("IGButton", controlName))
	{
		return new IGButton(this, controlName);
	}
	else
		return BaseClass::CreateControlByName(controlName);
}

void CCSTeamMenu::UpdateGameMode()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CCSTeamMenu::UpdateGameMode this=%p\n", this);
	SetupControlSettings();
}
