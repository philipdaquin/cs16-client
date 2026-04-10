#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/EditablePanel.h>

class CBuyPresetPanel : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CBuyPresetPanel, vgui2::EditablePanel);

public:
	CBuyPresetPanel(vgui2::Panel *parent, const char *panelName);

	virtual void ApplySchemeSettings(vgui2::IScheme *scheme) override;
	virtual void Paint() override;
};

#endif
