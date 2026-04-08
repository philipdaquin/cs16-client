#pragma once

#include "vgui2_stub_types.h"

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/Panel.h>

class CVGui2TestPanel : public vgui2::Panel
{
public:
	CVGui2TestPanel(vgui2::Panel *parent, const char *panelName);
	virtual void Paint() override;
};

#endif