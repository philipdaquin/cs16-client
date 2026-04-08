#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/Button.h>

namespace vgui2
{

class MouseOverPanelButton : public Button
{
	DECLARE_CLASS_SIMPLE(MouseOverPanelButton, Button);

public:
	MouseOverPanelButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL);
	MouseOverPanelButton(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL);

protected:
	virtual void OnMousePressed(MouseCode code) override;
	virtual void OnMouseReleased(MouseCode code) override;
	virtual void OnMouseDoublePressed(MouseCode code) override;
};

}

#endif