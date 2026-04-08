#include "cstrikeclassmenu.h"

#if !defined(VGUI2_STUB_MODE)

CClassMenu::CClassMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
}

void CClassMenu::ApplySchemeSettings(vgui2::HScheme scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
}

void CClassMenu::Paint()
{
	BaseClass::Paint();
}

#endif