#include <cstdio>

#include <KeyValues.h>
#include <vgui/IInputInternal.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <IEngineVGui.h>
#include <vgui/ISurface.h>

#include "CHudViewPort.h"
#include "CClientVGUI.h"
#include "CClientMOTD.h"
#include "CGameUITestPanel.h"
#include "csmoe/BuyMenu/cstrikebuymenu.h"
#include "csmoe/BuyMenu/cstrikebuyequipmenu.h"
#include "csmoe/BuyMenu/buy_presets.h"
#include "csmoe/cstriketeammenu.h"
#include "csmoe/cstrikeclassmenu.h"
#include "hud.h"
#include "parsemsg.h"

static CHudViewport *s_pHudViewPort = nullptr;

static void Cmd_ShowBuyMenu()
{
	if (s_pHudViewPort)
		s_pHudViewPort->OpenBuyMenuForLocalTeam(false);
}

static void Cmd_ShowBuyEquipMenu()
{
	if (s_pHudViewPort)
		s_pHudViewPort->OpenBuyMenuForLocalTeam(true);
}

static void Cmd_ChooseTeam()
{
	if (s_pHudViewPort)
		s_pHudViewPort->ShowPanel(PANEL_TEAM, true);
}

void CHudViewport::ApplySchemeSettings(vgui2::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetPaintBackgroundEnabled(false);

	//extern vgui2::HFont g_HudTextVgui_TextFont;
	//g_HudTextVgui_TextFont = pScheme->GetFont("Default");
}

void CHudViewport::Paint()
{
	BaseClass::Paint();

	static vgui2::HFont s_ProbeFont = vgui2::INVALID_FONT;
	static bool s_ProbeFontReady = false;
	if (s_ProbeFont == vgui2::INVALID_FONT)
	{
		s_ProbeFont = vgui2::surface()->CreateFont();
	}
	if (!s_ProbeFontReady && s_ProbeFont != vgui2::INVALID_FONT)
	{
		s_ProbeFontReady = vgui2::surface()->AddGlyphSetToFont(s_ProbeFont, "Default", 28, 700, 0, 0, 0, 0x0, 0xFFFF);
	}

	// Temporary probe text disabled now that font rendering is confirmed.
	// if (s_ProbeFontReady && s_ProbeFont != vgui2::INVALID_FONT)
	// {
	// 	const wchar_t probeText[] = L"VIEWPORT OVERLAY PROBE";
	// 	vgui2::surface()->DrawSetTextFont(s_ProbeFont);
	// 	vgui2::surface()->DrawSetTextColor(0, 0, 0, 255);
	// 	vgui2::surface()->DrawSetTextPos(36, 48);
	// 	vgui2::surface()->DrawPrintText(probeText, (int)(sizeof(probeText) / sizeof(probeText[0]) - 1));
	// 	vgui2::surface()->DrawFlushText();
	// }
}

void CHudViewport::Start()
{
	BaseClass::Start();

	s_pHudViewPort = this;

    gEngfuncs.pfnAddCommand("motd_open", []() { s_pHudViewPort->m_pMOTD->Activate(gHUD.m_szServerName, "wow"); });
	gEngfuncs.pfnAddCommand("buymenu", Cmd_ShowBuyMenu);
	gEngfuncs.pfnAddCommand("buyequip", Cmd_ShowBuyEquipMenu);
	gEngfuncs.pfnAddCommand("chooseteam", Cmd_ChooseTeam);
	RegisterBuyPresetCommands();
}

int CHudViewport::MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (m_bGotAllMOTD)
	{
		m_szMOTD.clear();
		m_bMOTDActive = false;
	}

	BufferReader buf(pszName, pbuf, iSize);

	m_bGotAllMOTD = buf.ReadByte();

	m_szMOTD += buf.ReadString();
	CClientMOTD *panel = m_pMOTD;
	if (panel)
	{
		m_bMOTDActive = true;
		panel->Activate(gHUD.m_szServerName, m_szMOTD.c_str());
	}
	else
		gEngfuncs.Con_Printf("MsgFunc_MOTD() : Error! CClientMOTD is nullptr\n");

	if (m_bGotAllMOTD)
		m_bMOTDActive = false;

	return 1;
}

bool CHudViewport::IsMOTDVisible() const
{
	return m_pMOTD && m_pMOTD->IsVisible();
}

bool CHudViewport::IsMOTDActive() const
{
	return m_bMOTDActive || IsMOTDVisible();
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
	gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::CreateDefaultPanels resetting cached panel pointers this=%p\n",
		this);
	m_pMOTD = nullptr;
	m_pTeamMenu = nullptr;
	m_pClassMenu = nullptr;
	m_pClassMenuCT = nullptr;
	m_pClassMenuTER = nullptr;
	m_pBuyMenu = nullptr;
	m_pBuyMenuCT = nullptr;
	m_pBuyMenuTER = nullptr;
	m_pBuyEquipMenuCT = nullptr;
	m_pBuyEquipMenuTER = nullptr;
	m_pZombieKeeperMenu = nullptr;

	AddNewPanel(CreatePanelByName("ClientMOTD"));
    AddNewPanel(CreatePanelByName(PANEL_TEAM));
    AddNewPanel(CreatePanelByName(PANEL_CLASS_CT));
    AddNewPanel(CreatePanelByName(PANEL_CLASS_TER));

    AddNewPanel(CreatePanelByName(PANEL_BUY_CT));
    AddNewPanel(CreatePanelByName(PANEL_BUY_TER));
    AddNewPanel(CreatePanelByName(PANEL_BUY_EQUIP_CT));
    AddNewPanel(CreatePanelByName(PANEL_BUY_EQUIP_TER));
    //AddNewPanel(CreatePanelByName(VIEWPORT_PANEL_SCORE));
	gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::CreateDefaultPanels done this=%p buyCT=%p buyTER=%p equipCT=%p equipTER=%p\n",
		this, (void *)m_pBuyMenuCT, (void *)m_pBuyMenuTER, (void *)m_pBuyEquipMenuCT, (void *)m_pBuyEquipMenuTER);

	// Temporarily disabled for startup isolation.
	// AddNewGameUIPanel(CreateGameUIPanelByName("GameUITestPanel"));
}

IViewportPanel* CHudViewport::CreatePanelByName(const char* pszName)
{
	IViewportPanel* pPanel = nullptr;
	gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::CreatePanelByName entry this=%p name='%s' team=%d\n",
		this, pszName ? pszName : "<null>", cl::g_iTeamNumber);
	
	if (Q_strcmp("ClientMOTD", pszName) == 0)
	{
		if(!m_pMOTD)
			m_pMOTD = new CClientMOTD(this);
		pPanel = m_pMOTD;
	}
	else if (Q_strcmp(PANEL_TEAM, pszName) == 0)
	{
        if (!m_pTeamMenu)
        {
            m_pTeamMenu = new CCSTeamMenu(this);
            m_pTeamMenu->UpdateGameMode();
        }
        pPanel = m_pTeamMenu;
    }
    else if (Q_strcmp(PANEL_CLASS_CT, pszName) == 0)
    {
        if (!m_pClassMenuCT)
        {
			gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::CreatePanelByName creating PANEL_CLASS_CT this=%p\n",
				this);
            m_pClassMenuCT = new CClassMenu_CT(this);
            m_pClassMenuCT->UpdateGameMode();
        }
        pPanel = m_pClassMenuCT;
    }
    else if (Q_strcmp(PANEL_CLASS_TER, pszName) == 0)
    {
        if (!m_pClassMenuTER)
        {
			gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::CreatePanelByName creating PANEL_CLASS_TER this=%p\n",
				this);
            m_pClassMenuTER = new CClassMenu_TER(this);
            m_pClassMenuTER->UpdateGameMode();
        }
        pPanel = m_pClassMenuTER;
    }
    // else if (Q_strcmp(PANEL_CLASS, pszName) == 0)
    // {
    //     pPanel = (cl::g_iTeamNumber == TEAM_TERRORIST) ? CreatePanelByName(PANEL_CLASS_TER) : CreatePanelByName(PANEL_CLASS_CT);
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
	else if (Q_strcmp(PANEL_BUY_CT, pszName) == 0)
	{
		// if (!m_pBuyMenuCT)
		// {
			gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::CreatePanelByName creating PANEL_BUY_CT this=%p\n",
				this);
			m_pBuyMenuCT = new CCSBuyMenu_CT(this);
		// }
		pPanel = m_pBuyMenuCT;
	}
	else if (Q_strcmp(PANEL_BUY_TER, pszName) == 0)
	{
		// if (!m_pBuyMenuTER)
		// {
			gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::CreatePanelByName creating PANEL_BUY_TER this=%p\n",
				this);
			m_pBuyMenuTER = new CCSBuyMenu_TER(this);
		// }
		pPanel = m_pBuyMenuTER;
	}
	else if (Q_strcmp(PANEL_BUY_EQUIP_CT, pszName) == 0)
	{
		if (!m_pBuyEquipMenuCT)
			m_pBuyEquipMenuCT = new CCSBuyEquipMenu_CT(this);
		pPanel = m_pBuyEquipMenuCT;
	}
	else if (Q_strcmp(PANEL_BUY_EQUIP_TER, pszName) == 0)
	{
		if (!m_pBuyEquipMenuTER)
			m_pBuyEquipMenuTER = new CCSBuyEquipMenu_TER(this);
		pPanel = m_pBuyEquipMenuTER;
	}
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
	/*else if (Q_strcmp(VIEWPORT_PANEL_SCORE, pszName) == 0)
	{
		pPanel = new CScorePanel(this);
	}
	*/
	return pPanel;
}

IGameUIPanel *CHudViewport::CreateGameUIPanelByName(const char *pszName)
{
	IGameUIPanel *pPanel = nullptr;
	
	if (Q_strcmp("GameUITestPanel", pszName) == 0)
	{
		IEngineVGui *enginevgui = engineVgui();
		if (!enginevgui)
		{
			std::fprintf(stderr, "[phase3][VGUI2-TRACE] CreateGameUIPanelByName skipped GameUITestPanel: engineVgui is null\n");
			return nullptr;
		}

		vgui2::VPANEL gameUIParent = enginevgui->GetPanel(PANEL_GAMEUIDLL);
		if (!gameUIParent)
		{
			std::fprintf(stderr, "[phase3][VGUI2-TRACE] CreateGameUIPanelByName skipped GameUITestPanel: PANEL_GAMEUIDLL is null\n");
			return nullptr;
		}

		std::fprintf(stderr, "[phase3][VGUI2-TRACE] CreateGameUIPanelByName creating GameUITestPanel parent=%p\n", (void *)gameUIParent);
		pPanel = new CGameUITestPanel(gameUIParent);
	}
	
	return pPanel;
}

bool CHudViewport::ShowVGUIMenu(int iMenu)
{
    gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::ShowVGUIMenu menu=%d team=%d viewport=%p classCT=%p classTER=%p\n",
        iMenu, cl::g_iTeamNumber, (void *)this, (void *)m_pClassMenuCT, (void *)m_pClassMenuTER);

    IViewportPanel *panel = NULL;

    switch (iMenu)
    {
        case MENU_TEAM:
        {
            panel = m_pTeamMenu;
            break;
        }
        case MENU_CLASS_T:
        {
            gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::ShowVGUIMenu selecting TER class menu team=%d\n",
                cl::g_iTeamNumber);
            panel = m_pClassMenuTER ? m_pClassMenuTER : CreatePanelByName(PANEL_CLASS_TER);
            break;
        }
        case MENU_CLASS_CT:
        {
            gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::ShowVGUIMenu selecting CT class menu team=%d\n",
                cl::g_iTeamNumber);
            panel = m_pClassMenuCT ? m_pClassMenuCT : CreatePanelByName(PANEL_CLASS_CT);
			break;
        }
        case MENU_BUY:
        case MENU_BUY_PISTOL:
        case MENU_BUY_SHOTGUN:
        case MENU_BUY_RIFLE:
        case MENU_BUY_SUBMACHINEGUN:
        case MENU_BUY_MACHINEGUN:
        case MENU_BUY_ITEM:
        {
			if (iMenu == MENU_BUY_ITEM)
			{
				OpenBuyMenuForLocalTeam(true);
				return true;
			}

			if (iMenu == MENU_BUY)
			{
				OpenBuyMenuForLocalTeam(false);
				return true;
			}

			CCSBaseBuyMenu *buyMenu = nullptr;
			if (cl::g_iTeamNumber == TEAM_CT)
			{
				gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::ShowVGUIMenu selecting PANEL_BUY_CT team=%d buyCT=%p\n",
					cl::g_iTeamNumber, (void *)m_pBuyMenuCT);

				buyMenu = m_pBuyMenuCT ? m_pBuyMenuCT : static_cast<CCSBaseBuyMenu *>(CreatePanelByName(PANEL_BUY_CT));			}
			else if (cl::g_iTeamNumber == TEAM_TERRORIST)
			{
				gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::ShowVGUIMenu selecting PANEL_BUY_TER team=%d buyTER=%p\n",
					cl::g_iTeamNumber, (void *)m_pBuyMenuTER);
				buyMenu = m_pBuyMenuTER ? m_pBuyMenuTER : static_cast<CCSBaseBuyMenu *>(CreatePanelByName(PANEL_BUY_TER));			}
			else
			{
				buyMenu = m_pBuyMenu;
			}

			if (!buyMenu)
				break;

			buyMenu->ActivateMenu(iMenu);
            return true;
        }
    }

    if (panel)
    {
        ShowPanel(panel, true);
        return true;
    }

    return false;
}

bool CHudViewport::HideVGUIMenu(int iMenu)
{
    IViewportPanel *panel = NULL;

    switch (iMenu)
    {
    case MENU_TEAM:
    {
        panel = m_pTeamMenu;
        break;
    }
        case MENU_CLASS_T:
        {
            panel = m_pClassMenuTER;
            break;
        }
        case MENU_CLASS_CT:
        {
            panel = m_pClassMenuCT;
            break;
        }

        case MENU_BUY:
        case MENU_BUY_PISTOL:
        case MENU_BUY_SHOTGUN:
        case MENU_BUY_RIFLE:
        case MENU_BUY_SUBMACHINEGUN:
        case MENU_BUY_MACHINEGUN:
        case MENU_BUY_ITEM:
        {
			panel = m_pBuyMenu;
			if (m_pBuyMenuCT)
				ShowPanel(m_pBuyMenuCT, false);
			if (m_pBuyMenuTER)
				ShowPanel(m_pBuyMenuTER, false);
			if (m_pBuyEquipMenuCT)
				ShowPanel(m_pBuyEquipMenuCT, false);
			if (m_pBuyEquipMenuTER)
				ShowPanel(m_pBuyEquipMenuTER, false);
			return true;
        }
    }

    if (panel)
    {
        ShowPanel(panel, false);
        return true;
    }

    return false;
}

void CHudViewport::UpdateGameMode()
{
    if(m_pBuyMenu)
        m_pBuyMenu->UpdateGameMode();
	if (m_pBuyMenuCT)
		m_pBuyMenuCT->UpdateGameMode();
	if (m_pBuyMenuTER)
		m_pBuyMenuTER->UpdateGameMode();
    if (m_pTeamMenu)
        m_pTeamMenu->UpdateGameMode();
}

void CHudViewport::OpenPanelWithCheck(const char *panelToOpen, const char *panelToCheck)
{
	IViewportPanel *targetPanel = FindPanelByName(panelToOpen);
	const bool targetFullyVisible = targetPanel ? vgui2::ipanel()->IsFullyVisible(targetPanel->GetVPanel()) : false;
	if (targetPanel && targetFullyVisible && GetActivePanel() == targetPanel)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CHudViewport::OpenPanelWithCheck skip active target='%s' panel=%p active=%p fullyVisible=%d\n",
			panelToOpen ? panelToOpen : "<null>", (void *)targetPanel, (void *)GetActivePanel(), targetFullyVisible ? 1 : 0);
		return;
	}

	IViewportPanel *checkPanel = FindPanelByName(panelToCheck);
	if (!checkPanel || !checkPanel->IsVisible())
		ShowPanel(panelToOpen, true);
}

void CHudViewport::OpenBuyMenuForLocalTeam(bool equipmentMenu)
{
	gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::OpenBuyMenuForLocalTeam entry this=%p team=%d equipment=%d\n",
		this, cl::g_iTeamNumber, equipmentMenu ? 1 : 0);
	if (cl::g_iTeamNumber == TEAM_CT)
	{
		gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::OpenBuyMenuForLocalTeam CT open='%s'\n",
			equipmentMenu ? PANEL_BUY_EQUIP_CT : PANEL_BUY_CT);
		IViewportPanel *buyPanel = equipmentMenu
			? (m_pBuyEquipMenuCT ? m_pBuyEquipMenuCT : CreatePanelByName(PANEL_BUY_EQUIP_CT))
			: (m_pBuyMenuCT ? m_pBuyMenuCT : CreatePanelByName(PANEL_BUY_CT));
		if (buyPanel)
			ShowPanel(buyPanel, true);
	}
	else if (cl::g_iTeamNumber == TEAM_TERRORIST)
	{
		gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::OpenBuyMenuForLocalTeam TER open='%s'\n",
			equipmentMenu ? PANEL_BUY_EQUIP_TER : PANEL_BUY_TER);
		IViewportPanel *buyPanel = equipmentMenu
			? (m_pBuyEquipMenuTER ? m_pBuyEquipMenuTER : CreatePanelByName(PANEL_BUY_EQUIP_TER))
			: (m_pBuyMenuTER ? m_pBuyMenuTER : CreatePanelByName(PANEL_BUY_TER));
		if (buyPanel)
			ShowPanel(buyPanel, true);
	}
}

bool CHudViewport::ShowVGUIMenuByName(const char* szName)
{
	gEngfuncs.Con_Printf("[PANEL_BUY] CHudViewport::ShowVGUIMenuByName entry this=%p name='%s' team=%d\n",
		this, szName ? szName : "<null>", cl::g_iTeamNumber);
    auto pPanel = FindPanelByName(szName);
    if (pPanel == nullptr)
        pPanel = CreatePanelByName(szName);

    if (pPanel == nullptr)
        return false;

    ShowPanel(pPanel, true);
    return true;
}
