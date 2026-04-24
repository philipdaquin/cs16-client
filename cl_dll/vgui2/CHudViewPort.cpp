#include <cstdio>

#include <KeyValues.h>
#include <IEngineVGui.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>

#include "CHudViewPort.h"
#include "CClientVGUI.h"
#include "CClientMOTD.h"
#include "CGameUITestPanel.h"
#include "game_controls/buymenu.h"
#include "game_controls/classmenu.h"
// #include "game_controls/teammenu.h" // legacy generic team menu
#include "csmoe/cstriketeammenu.h"
#include "hud.h"
#include "parsemsg.h"

void CHudViewport::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}

void CHudViewport::Start()
{
	std::fprintf(stderr, "[phase3][VGUI2-TRACE] CHudViewport::Start entry this=%p viewport=%p\n", this, (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CHudViewport::Start this=%p viewport=%p\n", this, (void *)g_pViewport);
	BaseClass::Start();
	std::fprintf(stderr, "[phase3][VGUI2-TRACE] CHudViewport::Start exit this=%p viewport=%p\n", this, (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CHudViewport::Start after BaseClass::Start this=%p viewport=%p\n", this, (void *)g_pViewport);

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
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::MsgFunc_MOTD this=%p size=%d viewport=%p\n",
		this, iSize, (void *)g_pViewport);
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
	std::fprintf(stderr, "[phase3][VGUI2-TRACE] CHudViewport::CreateDefaultPanels entry this=%p viewport=%p\n", this, (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CHudViewport::CreateDefaultPanels this=%p viewport=%p\n", this, (void *)g_pViewport);
	AddNewPanel(CreatePanelByName("ClientMOTD"));
	AddNewPanel(CreatePanelByName(PANEL_TEAM));
	AddNewPanel(CreatePanelByName(PANEL_CLASS));
	AddNewPanel(CreatePanelByName(PANEL_BUY));

	AddNewGameUIPanel(CreateGameUIPanelByName("GameUITestPanel"));
}

IViewportPanel *CHudViewport::CreatePanelByName(const char *pszName)
{
	std::fprintf(stderr, "[phase3][VGUI2-TRACE] CHudViewport::CreatePanelByName entry this=%p name='%s' viewport=%p\n",
		this, pszName ? pszName : "<null>", (void *)g_pViewport);
	gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CHudViewport::CreatePanelByName this=%p name='%s' viewport=%p\n",
		this, pszName ? pszName : "<null>", (void *)g_pViewport);
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
		{
			// Old generic path:
			// m_pTeamMenu = new CTeamMenu(this);
			std::fprintf(stderr, "[phase3][VGUI2-TRACE] CHudViewport::CreatePanelByName creating TEAM panel this=%p viewport=%p\n",
				this, (void *)g_pViewport);
			gEngfuncs.Con_Printf("[phase3][VGUI2-CLIENT] CHudViewport::CreatePanelByName creating TEAM panel this=%p viewport=%p\n",
				this, (void *)g_pViewport);
			m_pTeamMenu = new CCSTeamMenu(this);
			static_cast<CCSTeamMenu *>(m_pTeamMenu)->UpdateGameMode();
		}
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

	// Older implemenetationm remove so we can use the vanilla implentation
	// if (Q_strcmp("ClientMOTD", pszName) == 0)
	// {
	// 	if(!m_pMOTD)
	// 		m_pMOTD = new CClientMOTD(this);
	// 	pPanel = m_pMOTD;
	// }
    // else if (Q_strcmp(PANEL_TEAM, pszName) == 0)
    // {
    //     if (!m_pTeamMenu)
    //     {
    //         m_pTeamMenu = new CCSTeamMenu(this);
    //         m_pTeamMenu->UpdateGameMode();
    //     }
    //     pPanel = m_pTeamMenu;
    // }
    // else if (Q_strcmp(PANEL_CLASS, pszName) == 0)
    // {
    //     if (!m_pClassMenu)
    //     {
    //         m_pClassMenu = new CCSClassMenu(this);
    //         m_pClassMenu->UpdateGameMode();
    //     }
    //     pPanel = m_pClassMenu;
    // }
	// else if (Q_strcmp(PANEL_BUY, pszName) == 0)
	// {
	// 	if(!m_pBuyMenu)
    //     {
    //         m_pBuyMenu = new CCSBaseBuyMenu(this);
    //         m_pBuyMenu->UpdateGameMode();
    //     }
	// 	pPanel = m_pBuyMenu;
	// }
    // else if (Q_strcmp(CZSHELTERTEAMHOUSINGDLG_NAME, pszName) == 0)
    // {
    //     pPanel = new ZShelterTeamHousingDialog();
    // }
    // else if (Q_strcmp(PANEL_ZOMBIEKEEPER, pszName) == 0)
    // {
    //     if (!m_pZombieKeeperMenu)
    //     {
    //         m_pZombieKeeperMenu = new CZb2ZombieKeeper();
    //         m_pZombieKeeperMenu->SetupPage(0);
    //     }
    //     pPanel = m_pZombieKeeperMenu;

    // }



	return pPanel;
}

IGameUIPanel *CHudViewport::CreateGameUIPanelByName(const char *pszName)
{
	if (Q_strcmp("GameUITestPanel", pszName) == 0)
	{
		IEngineVGui *enginevgui = engineVgui();
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::CreateGameUIPanelByName before GetPanel this=%p name='%s' enginevgui=%p viewport=%p\n",
			this, pszName ? pszName : "<null>", (void *)enginevgui, (void *)g_pViewport);
		// vgui2::VPANEL gameUIParent = engineVgui()->GetPanel(PANEL_GAMEUIDLL);
		if (!enginevgui)
		{
			gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::CreateGameUIPanelByName aborting because enginevgui is null this=%p name='%s'\n",
				this, pszName ? pszName : "<null>");
			return nullptr;
		}

		vgui2::VPANEL gameUIParent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::CreateGameUIPanelByName this=%p name='%s' gameUIParent=%p viewport=%p\n",
			this, pszName ? pszName : "<null>", (void *)gameUIParent, (void *)g_pViewport);
		auto *pPanel = new CGameUITestPanel(gameUIParent);
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::CreateGameUIPanelByName created panel=%p name='%s' gameUIParent=%p\n",
			(void *)pPanel, pszName ? pszName : "<null>", (void *)gameUIParent);
		return pPanel;
	}

	return nullptr;
}

bool CHudViewport::ShowVGUIMenu(int iMenu)
{
	std::fprintf(stderr, "[phase4][VGUI2-TRACE] CHudViewport::ShowVGUIMenu entry this=%p menu=%d viewport=%p team=%p class=%p buy=%p\n",
		this, iMenu, (void *)g_pViewport, (void *)m_pTeamMenu, (void *)m_pClassMenu, (void *)m_pBuyMenu);
	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CHudViewport::ShowVGUIMenu this=%p menu=%d viewport=%p team=%p class=%p buy=%p\n",
		this, iMenu, (void *)g_pViewport, (void *)m_pTeamMenu, (void *)m_pClassMenu, (void *)m_pBuyMenu);
	IViewportPanel *panel = nullptr;

	switch (iMenu)
	{
	case MENU_TEAM:
		panel = m_pTeamMenu;
		gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CHudViewport::ShowVGUIMenu selecting TEAM panel=%p type=%d\n",
			(void *)panel, iMenu);
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
	{
		gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CHudViewport::ShowVGUIMenu no panel for menu=%d\n", iMenu);
		return false;
	}

	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CHudViewport::ShowVGUIMenu before ShowPanel menu=%d panel=%p visible=%d active=%p\n",
		iMenu, (void *)panel, panel->IsVisible() ? 1 : 0, (void *)GetActivePanel());
	ShowPanel(panel, true);
	gEngfuncs.Con_Printf("[phase4][VGUI2-CLIENT] CHudViewport::ShowVGUIMenu shown menu=%d panel=%p visible=%d active=%p\n",
		iMenu, (void *)panel, panel->IsVisible() ? 1 : 0, (void *)GetActivePanel());
	std::fprintf(stderr, "[phase4][VGUI2-TRACE] CHudViewport::ShowVGUIMenu exit this=%p menu=%d panel=%p visible=%d active=%p\n",
		this, iMenu, (void *)panel, panel->IsVisible() ? 1 : 0, (void *)GetActivePanel());
	return true;
}

bool CHudViewport::HideVGUIMenu(int iMenu)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::HideVGUIMenu this=%p menu=%d viewport=%p\n",
		this, iMenu, (void *)g_pViewport);
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
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::HideVGUIMenu no panel for menu=%d\n", iMenu);
		return false;
	}

	ShowPanel(panel, false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::HideVGUIMenu hidden menu=%d panel=%p\n", iMenu, (void *)panel);
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
