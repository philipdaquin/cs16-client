#ifndef CSTRIKEBUYMOUSEOVERPANELBUTTON_H
#define CSTRIKEBUYMOUSEOVERPANELBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include "../../../hud.h"
#include "../teamname.h"
#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui/KeyCode.h>
#include <filesystem.h>
#include "shared_util.h"
#include "game_controls/mouseoverpanelbutton.h"
#include "buymouseoverpanelbutton.h"
#include "WeaponImagePanel.h"

class CSBuyMouseOverPanelButton : public BuyMouseOverPanelButton
{
private:
	typedef BuyMouseOverPanelButton BaseClass;
public:
	CSBuyMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *panel);

	virtual void Paint() override;
	virtual void PerformLayout() override;

	void UpdateWeapon(const char *weapon = "");
	void SetBanWeapon(const char* weapon = "", int iLevel = 0);

	WeaponImagePanel *m_pWeaponImage;
	vgui2::ImagePanel* m_pLockedImage;
	vgui2::ImagePanel* m_pLockedImageBg;

	vgui2::ImagePanel* m_pBlankBg;

	vgui2::Label* m_pLevelText;

	TeamName m_iTeam;
};

#endif
