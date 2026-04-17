#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/EditablePanel.h>

class CCSBaseViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CCSBaseViewport, vgui2::EditablePanel);

public:
	CCSBaseViewport(vgui2::VPANEL parent, const char *panelName);

	void ShowPanel(bool state);
	void ReloadScheme(const char *fromFile = "resource/ClientScheme.res");

	void PerformLayout() override;
	void ApplySchemeSettings(vgui2::IScheme *scheme) override;
};

#endif
