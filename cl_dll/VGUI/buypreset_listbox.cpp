#include "buypreset_listbox.h"

#if !defined(VGUI2_STUB_MODE)

CBuyPresetListBox::CBuyPresetListBox(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
}

void CBuyPresetListBox::ApplySchemeSettings(vgui2::HScheme scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
}

void CBuyPresetListBox::Paint()
{
	BaseClass::Paint();
}

#endif