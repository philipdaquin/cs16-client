#ifndef CSBUYSUBMENU_H
#define CSBUYSUBMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls/buysubmenu.h"
#include "buymouseoverpanelbutton.h"

class CCSBuySubMenu : public CBuySubMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSBuySubMenu, CBuySubMenu);

public:
	CCSBuySubMenu(vgui2::Panel *parent, const char *name = "BuySubMenu");

protected:
	virtual void OnCommand(const char *command) override;
	virtual void PerformLayout() override;
	virtual void OnSizeChanged(int newWide, int newTall) override;
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui2::EditablePanel *panel) override;
	virtual CBuySubMenu *CreateNewSubMenu(const char *name = "BuySubMenu") override;

	bool m_backgroundLayoutFinished;
};

#endif
