#include "buysubmenu.h"

#if !defined(VGUI2_STUB_MODE)

CBuySubMenu::CBuySubMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	m_Category = CCounterStrikeViewport::CATEGORY_COUNT;
	m_bIsCT = false;
}

void CBuySubMenu::SetCategory(CCounterStrikeViewport::BuyMenuCategory_t category, bool isCT)
{
	m_Category = category;
	m_bIsCT = isCT;
}

void CBuySubMenu::ApplySchemeSettings(vgui2::HScheme scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
}

void CBuySubMenu::Paint()
{
	BaseClass::Paint();
}

#endif