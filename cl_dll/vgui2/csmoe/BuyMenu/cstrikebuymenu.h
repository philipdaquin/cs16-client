#ifndef CSTRIKEBUYMENU_H
#define CSTRIKEBUYMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls/buymenu.h"
#include "IViewport.h"

#define PANEL_BUY_CT "BuyMenu_CT"
#define PANEL_BUY_TER "BuyMenu_TER"

namespace vgui
{
	class Panel;
	class Button;
	class Label;
}

namespace vgui2
{
	class Panel;
	class Label;
}

class BuyMouseOverPanelButton;
class BuyPresetButton;
class BuyPresetEditPanel;
class CCSBuySubMenu;

enum
{
	NUM_BUY_PRESET_BUTTONS = 4,
};

class CCSBaseBuyMenu : public CBuyMenu
{
	typedef CBuyMenu BaseClass;

public:
	CCSBaseBuyMenu(IViewport *pViewPort, int team);

protected:
	void SetupControlSettings();
public:
	void SetTeam(int iTeam);
	int GetTeam() const { return m_iTeam; }
	void UpdateGameMode();
	// void LoadTeamResource(int team);
	void UpdateBuyPresets(bool showDefaultPanel = false);

public:
	virtual void Init(void);
	// virtual void VidInit(void);
	virtual void ShowPanel(bool bShow);
	virtual void Update(void);
	virtual void Paint(void);
	virtual void SetVisible(bool state);
	virtual void GotoMenu(int iMenu);
	virtual void ActivateMenu(int iMenu);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

private:
	void SetupBuyPresetControls();
	void ConfigureMainBuyMenuCommands();

	bool m_backgroundLayoutFinished;
	BuyPresetButton *m_pBuyPresetButtons[NUM_BUY_PRESET_BUTTONS];
	BuyPresetEditPanel *m_pLoadout;
	vgui2::Label *m_pMoney;
	vgui2::Panel *m_pMainBackground;
	int m_lastMoney;
};

class CCSBuyMenu_CT : public CCSBaseBuyMenu
{
	typedef CCSBaseBuyMenu BaseClass;

public:
	CCSBuyMenu_CT(IViewport *pViewPort);

	virtual const char *GetName(void) { return PANEL_BUY_CT; }
};

class CCSBuyMenu_TER : public CCSBaseBuyMenu
{
	typedef CCSBaseBuyMenu BaseClass;

public:
	CCSBuyMenu_TER(IViewport *pViewPort);

	virtual const char *GetName(void) { return PANEL_BUY_TER; }
};

#endif
