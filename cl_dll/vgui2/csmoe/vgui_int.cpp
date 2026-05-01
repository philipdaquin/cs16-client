

#include <vgui/VGUI.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>
#include <IEngineVGui.h>

#include <vgui_controls/controls.h>

#include "../CClientVGUI.h"
#include <IEngineVGui.h>
#include <vgui/IPanel.h>

void GetHudSize(int &w, int &h)
{
	w = h = 0;

	IEngineVGui *vguiEngine = engineVgui();
	if (!vguiEngine)
	{
		vgui2::surface()->GetScreenSize(w, h);
		return;
	}

	vgui2::VPANEL hudParent = vguiEngine->GetPanel(PANEL_CLIENTDLL);

	if (hudParent)
		vgui2::ipanel()->GetSize(hudParent, w, h);
	else
		vgui2::surface()->GetScreenSize(w, h);
}
