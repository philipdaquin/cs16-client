#include <KeyValues.h>
#include <IEngineVGui.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>

#include "CHudViewPort.h"
#include "CClientMOTD.h"
#include "CGameUITestPanel.h"
#include "game_controls/buymenu.h"
#include "game_controls/classmenu.h"
#include "game_controls/teammenu.h"
#include "hud.h"
#include "parsemsg.h"

void CHudViewport::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}

void CHudViewport::Start()
{
	BaseClass::Start();

	static CHudViewport *const s_pHudViewPort = this;

	gEngfuncs.pfnHookUserMsg("VGUIMenu", [](const char *pszName, int iSize, void *pbuf)
	{
		return s_pHudViewPort->MsgFunc_MOTD(pszName, iSize, pbuf);
	});

	gEngfuncs.pfnAddCommand("motd_open", []()
	{
		s_pHudViewPort->m_pMOTD->Activate(gHUD.m_szServerName, "wow");
	});
}

int CHudViewport::MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (m_bGotAllMOTD)
		m_szMOTD.clear();

	BufferReader buf(pszName, pbuf, iSize);

	m_bGotAllMOTD = buf.ReadByte();
	m_szMOTD += buf.ReadString();

	return 1;
}

void CHudViewport::HideScoreBoard()
{
	BaseClass::HideScoreBoard();
	gHUD.m_Scoreboard.UserCmd_HideScores();
}

void CHudViewport::ActivateClientUI()
{
	BaseClass::ActivateClientUI();

	if (gHUD.m_iIntermission)
		gHUD.m_Scoreboard.UserCmd_ShowScores();
}

void CHudViewport::HideClientUI()
{
	BaseClass::HideClientUI();
}

void CHudViewport::CreateDefaultPanels()
{
	AddNewPanel(CreatePanelByName("ClientMOTD"));
	AddNewPanel(CreatePanelByName(PANEL_TEAM));
	AddNewPanel(CreatePanelByName(PANEL_CLASS));
	AddNewPanel(CreatePanelByName(PANEL_BUY));

	AddNewGameUIPanel(CreateGameUIPanelByName("GameUITestPanel"));
}

IViewportPanel *CHudViewport::CreatePanelByName(const char *pszName)
{
	IViewportPanel *pPanel = nullptr;

	if (Q_strcmp("ClientMOTD", pszName) == 0)
	{
		if (!m_pMOTD)
			m_pMOTD = new CClientMOTD(this);
		pPanel = m_pMOTD;
	}
	else if (Q_strcmp(PANEL_TEAM, pszName) == 0)
	{
		if (!m_pTeamMenu)
			m_pTeamMenu = new CTeamMenu(this);
		pPanel = m_pTeamMenu;
	}
	else if (Q_strcmp(PANEL_CLASS, pszName) == 0)
	{
		if (!m_pClassMenu)
			m_pClassMenu = new CClassMenu(this);
		pPanel = m_pClassMenu;
	}
	else if (Q_strcmp(PANEL_BUY, pszName) == 0)
	{
		if (!m_pBuyMenu)
			m_pBuyMenu = new CBuyMenu(this);
		pPanel = m_pBuyMenu;
	}

	return pPanel;
}

IGameUIPanel *CHudViewport::CreateGameUIPanelByName(const char *pszName)
{
	if (Q_strcmp("GameUITestPanel", pszName) == 0)
		return new CGameUITestPanel(engineVgui()->GetPanel(PANEL_GAMEUIDLL));

	return nullptr;
}

bool CHudViewport::ShowVGUIMenu(int iMenu)
{
	IViewportPanel *panel = nullptr;

	switch (iMenu)
	{
	case MENU_TEAM:
		panel = m_pTeamMenu;
		break;

	case MENU_CLASS_T:
	case MENU_CLASS_CT:
		panel = m_pClassMenu;
		if (m_pClassMenu)
		{
			KeyValues::AutoDelete data(new KeyValues("ClassMenu"));
			data->SetInt("team", iMenu == MENU_CLASS_CT ? TEAM_CT : TEAM_TERRORIST);
			m_pClassMenu->SetData(data);
		}
		break;

	case MENU_BUY:
	case MENU_BUY_PISTOL:
	case MENU_BUY_SHOTGUN:
	case MENU_BUY_RIFLE:
	case MENU_BUY_SUBMACHINEGUN:
	case MENU_BUY_MACHINEGUN:
	case MENU_BUY_ITEM:
		panel = m_pBuyMenu;
		break;

	default:
		break;
	}

	if (!panel)
		return false;

	ShowPanel(panel, true);
	return true;
}

bool CHudViewport::HideVGUIMenu(int iMenu)
{
	IViewportPanel *panel = nullptr;

	switch (iMenu)
	{
	case MENU_TEAM:
		panel = m_pTeamMenu;
		break;

	case MENU_CLASS_T:
	case MENU_CLASS_CT:
		panel = m_pClassMenu;
		break;

	case MENU_BUY:
	case MENU_BUY_PISTOL:
	case MENU_BUY_SHOTGUN:
	case MENU_BUY_RIFLE:
	case MENU_BUY_SUBMACHINEGUN:
	case MENU_BUY_MACHINEGUN:
	case MENU_BUY_ITEM:
		panel = m_pBuyMenu;
		break;

	default:
		break;
	}

	if (!panel)
		return false;

	ShowPanel(panel, false);
	return true;
}

void CHudViewport::UpdateGameMode()
{
}

bool CHudViewport::ShowVGUIMenuByName(const char *szName)
{
	IViewportPanel *pPanel = FindPanelByName(szName);
	if (!pPanel)
		pPanel = CreatePanelByName(szName);

	if (!pPanel)
		return false;

	ShowPanel(pPanel, true);
	return true;
}
