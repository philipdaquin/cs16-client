

#include <vgui/VGUI.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>

#include <vgui_controls/controls.h>

#include "../CClientVGUI.h"
#include <IEngineVGui.h>
#include <vgui/IPanel.h>

void GetHudSize(int &w, int &h)
{
	vgui2::VPANEL hudParent = engineVgui()->GetPanel(PANEL_CLIENTDLL);

	if (hudParent)
		vgui2::ipanel()->GetSize(hudParent, w, h);
	else
		vgui2::surface()->GetScreenSize(w, h);
}
