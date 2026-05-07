#ifndef CSCLASSMENU_H
#define CSCLASSMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls/classmenu.h"
#include "teamname.h"
#include <FileSystem.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>
#include "../../../SourceSDK/public/tier1/utlvector.h"

#define PANEL_CLASS_CT "ClassMenu_CT"
#define PANEL_CLASS_TER "ClassMenu_TER"

class CCSClassImagePanel : public vgui2::ImagePanel
{
public:
	DECLARE_CLASS_SIMPLE(CCSClassImagePanel, vgui2::ImagePanel);

	CCSClassImagePanel(vgui2::Panel *pParent, const char *pName);
	~CCSClassImagePanel() override;

	void ApplySettings(KeyValues *inResourceData) override;
	void Paint() override;

	char m_ModelName[128];
};

extern CUtlVector<CCSClassImagePanel *> g_ClassImagePanels;

class CClassMenu_TER : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE(CClassMenu_TER, CClassMenu);

public:
	explicit CClassMenu_TER(IViewport *pViewPort);

	void PaintBackground() override;
	void PerformLayout() override;
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;

	vgui2::Panel *CreateControlByName(const char *controlName) override;
	const char *GetName() override;
	void ShowPanel(bool bShow) override;
	void Update() override;
	void UpdateGameMode();
	void SetupControlSettings();
	void SetVisible(bool state) override;

	bool m_backgroundLayoutFinished;
};

class CClassMenu_CT : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE(CClassMenu_CT, CClassMenu);

public:
	explicit CClassMenu_CT(IViewport *pViewPort);

	void PaintBackground() override;
	void PerformLayout() override;
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;

	vgui2::Panel *CreateControlByName(const char *controlName) override;
	const char *GetName() override;
	void ShowPanel(bool bShow) override;
	void Update() override;
	void UpdateGameMode();
	void SetupControlSettings();
	void SetVisible(bool state) override;

	bool m_backgroundLayoutFinished;
};

// Compatibility shell for older call sites. The viewport now uses the Source
// split CT/T panels directly, but keeping this type avoids churn elsewhere.
class CCSClassMenu : public CClassMenu_CT
{
private:
	DECLARE_CLASS_SIMPLE(CCSClassMenu, CClassMenu_CT);

public:
	explicit CCSClassMenu(IViewport *pViewPort) : CClassMenu_CT(pViewPort) {}

	void UpdateGameMode() {}
	bool CheckShowType() { return false; }
	void SetTeam(TeamName) {}
	void SetupTeamPage(TeamName, size_t) {}
	void SetupPage(size_t) {}
};

#endif
