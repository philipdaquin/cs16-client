#include "cstriketeammenu.h"

#if !defined(VGUI2_STUB_MODE)

CTeamMenu::CTeamMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
}

void CTeamMenu::ApplySchemeSettings(vgui2::HScheme scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
}

void CTeamMenu::Paint()
{
	BaseClass::Paint();
}

#endif