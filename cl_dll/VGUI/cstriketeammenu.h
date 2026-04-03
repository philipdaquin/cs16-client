#pragma once

#include "VGUI/counterstrikeviewport.h"
#include <vgui_controls/Frame.h>

class CTeamMenuPanel : public vgui2::Frame, public IViewportPanel
{
	DECLARE_CLASS_SIMPLE(CTeamMenuPanel, vgui2::Frame);

public:
	CTeamMenuPanel(vgui2::Panel *parent, const char *panelName);

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

	virtual void OnCommand(const char *command);
	virtual void OnKeyCodePressed(vgui2::KeyCode code);

private:
	void UpdateMapName();
	void UpdateButtonStates();
	void CenterInViewport();

	int m_iCurrentTeam;
};

