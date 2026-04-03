#pragma once

#include "VGUI/counterstrikeviewport.h"
#include <vgui_controls/EditablePanel.h>

class CSpectatorGUI : public vgui2::EditablePanel, public IViewportPanel
{
	DECLARE_CLASS_SIMPLE(CSpectatorGUI, vgui2::EditablePanel);

public:
	CSpectatorGUI(vgui2::Panel *parent, const char *panelName);

	virtual const char *GetName();
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate();
	virtual bool HasInputElements();
	virtual void ShowPanel(bool state);
	virtual vgui2::VPANEL GetVPanel();
	virtual bool IsVisible();
	virtual void SetParent(vgui2::VPANEL parent);

private:
	void UpdateLabelText(const char *controlName, const char *text);
	void UpdateScoreLabels();
	void UpdateTimerLabel();
};
