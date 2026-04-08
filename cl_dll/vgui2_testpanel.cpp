#include "vgui2_testpanel.h"

#if !defined(VGUI2_STUB_MODE)

#include <vgui2/ISurface.h>

CVGui2TestPanel::CVGui2TestPanel(vgui2::Panel *parent, const char *panelName)
	: vgui2::Panel(parent, panelName)
{
}

void CVGui2TestPanel::Paint()
{
	vgui2::ISurface *surface = g_pVGuiSurface;
	if (!surface)
		return;

	int w, h;
	GetSize(w, h);

	surface->DrawSetColor(0, 255, 0, 180);
	surface->DrawFilledRect(0, 0, w, h);
}

#endif