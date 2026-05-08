#ifndef GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H
#define GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H

#include "CBaseViewport.h"

#include <string>


class CClientMOTD;
class CCSBaseBuyMenu;
class CCSBuyMenu_CT;
class CCSBuyMenu_TER;
class CCSBuyEquipMenu_CT;
class CCSBuyEquipMenu_TER;
class CCSTeamMenu;
class CCSClassMenu;
class CClassMenu_CT;
class CClassMenu_TER;
class CZb2ZombieKeeper;

/**
*	Viewport for the Hud.
*/
class CHudViewport : public CBaseViewport
{
public:
	DECLARE_CLASS_SIMPLE(CHudViewport, CBaseViewport);

public:
	CHudViewport() = default;

	void ApplySchemeSettings(vgui2::IScheme* pScheme) override;
	void Paint() override;

	void Start() override;

	void HideScoreBoard() override;

	void ActivateClientUI() override;

	void HideClientUI() override;

	void CreateDefaultPanels() override;

	IViewportPanel* CreatePanelByName(const char* pszName) override;
	IGameUIPanel* CreateGameUIPanelByName(const char* pszName) override;

    bool ShowVGUIMenu(int iMenu) override;
	bool ShowVGUIMenuByName(const char* szName) override;
    bool HideVGUIMenu(int iMenu) override;
    void UpdateGameMode() override;

	void OpenPanelWithCheck(const char *panelToOpen, const char *panelToCheck);
	void OpenBuyMenuForLocalTeam(bool equipmentMenu);

	int MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf);

	// MOTD
	bool m_bGotAllMOTD = false;
	std::string	m_szMOTD;
	
	CClientMOTD *m_pMOTD = nullptr;
	CCSTeamMenu *m_pTeamMenu = nullptr;
	CCSClassMenu *m_pClassMenu = nullptr;
	CClassMenu_CT *m_pClassMenuCT = nullptr;
	CClassMenu_TER *m_pClassMenuTER = nullptr;
    CCSBaseBuyMenu *m_pBuyMenu = nullptr;
	CCSBuyMenu_CT *m_pBuyMenuCT = nullptr;
	CCSBuyMenu_TER *m_pBuyMenuTER = nullptr;
	CCSBuyEquipMenu_CT *m_pBuyEquipMenuCT = nullptr;
	CCSBuyEquipMenu_TER *m_pBuyEquipMenuTER = nullptr;
	CZb2ZombieKeeper* m_pZombieKeeperMenu = nullptr;
};

#endif //GAME_CLIENT_UI_HUD_CHUDVIEWPORT_H
