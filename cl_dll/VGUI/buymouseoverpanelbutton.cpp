#include "buymouseoverpanelbutton.h"

#if !defined(VGUI2_STUB_MODE)

#include <vgui2/ISurface.h>

vgui2::MouseOverPanelButton::MouseOverPanelButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd)
	: BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
{
}

vgui2::MouseOverPanelButton::MouseOverPanelButton(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget, const char *pCmd)
	: BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
{
}

void vgui2::MouseOverPanelButton::OnMousePressed(MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		FireActionSignal();
	}
}

void vgui2::MouseOverPanelButton::OnMouseReleased(MouseCode code)
{
}

void vgui2::MouseOverPanelButton::OnMouseDoublePressed(MouseCode code)
{
	OnMousePressed(code);
}

#endif