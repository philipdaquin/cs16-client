#pragma once

#include <vgui/VGUI2.h>
#include <KeyValues.h>
#include <tier1/utlvector.h>
#include <vgui_controls/EditablePanel.h>

namespace vgui2
{
class IScheme;
}

class IViewportPanel
{
public:
	virtual ~IViewportPanel() {}
	virtual const char *GetName() = 0;
	virtual void SetData(KeyValues *data) = 0;
	virtual void Reset() = 0;
	virtual void Update() = 0;
	virtual bool NeedsUpdate() = 0;
	virtual bool HasInputElements() = 0;
	virtual void ShowPanel(bool state) = 0;
	virtual vgui2::VPANEL GetVPanel() = 0;
	virtual bool IsVisible() = 0;
	virtual void SetParent(vgui2::VPANEL parent) = 0;
};

#define PANEL_TEAM "TeamMenu"
#define PANEL_CLASS_T "ClassMenuT"
#define PANEL_CLASS_CT "ClassMenuCT"
#define PANEL_BUY "BuyMenu"
#define PANEL_SPECGUI "SpectatorGUI"

class CCounterStrikeViewport : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CCounterStrikeViewport, vgui2::EditablePanel);

public:
	CCounterStrikeViewport();
	virtual ~CCounterStrikeViewport();

	void Start();
	void Reset();
	void UpdateAllPanels();
	void HideAllPanels();

	IViewportPanel *FindPanelByName(const char *name);
	IViewportPanel *GetActivePanel() const;
	void ShowPanel(const char *name, bool state);
	void ShowPanel(int menuType, bool state);

	vgui2::HScheme GetSchemeHandle() const;

	virtual void Paint();
	virtual void OnThink();
	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme);

private:
	void CreateDefaultPanels();
	void AddNewPanel(IViewportPanel *panel);
	void SetToFullScreen();

	CUtlVector<IViewportPanel *> m_Panels;
	IViewportPanel *m_pActivePanel;
	vgui2::HScheme m_hScheme;
};

extern CCounterStrikeViewport *g_pViewport;

