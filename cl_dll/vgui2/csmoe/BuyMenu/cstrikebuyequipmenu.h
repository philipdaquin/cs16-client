#ifndef CSTRIKEBUYEQUIPMENU_H
#define CSTRIKEBUYEQUIPMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls/buymenu.h"
#include "IViewport.h"

#define PANEL_BUY_EQUIP_CT "BuyEquipment_CT"
#define PANEL_BUY_EQUIP_TER "BuyEquipment_TER"

class CCSBuyEquipMenu_CT : public CBuyMenu
{
	typedef CBuyMenu BaseClass;

public:
	CCSBuyEquipMenu_CT(IViewport *pViewPort);

	virtual const char *GetName(void) { return PANEL_BUY_EQUIP_CT; }
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

private:
	bool m_backgroundLayoutFinished;
};

class CCSBuyEquipMenu_TER : public CBuyMenu
{
	typedef CBuyMenu BaseClass;

public:
	CCSBuyEquipMenu_TER(IViewport *pViewPort);

	virtual const char *GetName(void) { return PANEL_BUY_EQUIP_TER; }
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

private:
	bool m_backgroundLayoutFinished;
};

#endif
