#include "buypresetpanel.h"

#if !defined(VGUI2_STUB_MODE)

CBuyPresetPanel::CBuyPresetPanel(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
}

void CBuyPresetPanel::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
}

void CBuyPresetPanel::Paint()
{
	BaseClass::Paint();
}

#endif
