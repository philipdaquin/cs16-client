#include "cs_baseviewport.h"

#if !defined(VGUI2_STUB_MODE)

#include <tier2/tier2.h>
#include <vgui/IPanel.h>

CCSBaseViewport::CCSBaseViewport(vgui2::VPANEL parent, const char *panelName)
	: BaseClass(NULL, panelName)
{
	SetParent(parent);
	SetScheme("ClientScheme");
	SetProportional(true);
	SetVisible(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
}

void CCSBaseViewport::ShowPanel(bool state)
{
	SetVisible(state);
	SetMouseInputEnabled(state);
	SetKeyBoardInputEnabled(state);
}

void CCSBaseViewport::ReloadScheme(const char *fromFile)
{
	if (!fromFile || !vgui2::scheme())
		return;

	vgui2::HScheme scheme = vgui2::scheme()->LoadSchemeFromFileEx(GetVParent(), fromFile, "ClientScheme");
	if (scheme)
	{
		SetScheme(scheme);
		SetProportional(true);
	}

	InvalidateLayout(true, true);
}

void CCSBaseViewport::PerformLayout()
{
	int wide = 640;
	int tall = 480;

	if (GetVParent())
		vgui2::ipanel()->GetSize(GetVParent(), wide, tall);

	SetBounds(0, 0, wide, tall);
	BaseClass::PerformLayout();
}

void CCSBaseViewport::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
}

#endif
