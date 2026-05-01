#include "cstrikeclassmenu.h"

#include "hud.h"
#include "CBackGroundPanel.h"
#include "vgui_resource_paths.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Button.h>

using namespace vgui2;

CUtlVector<CCSClassImagePanel *> g_ClassImagePanels;

CCSClassImagePanel::CCSClassImagePanel(Panel *pParent, const char *pName)
	: ImagePanel(pParent, pName)
{
	g_ClassImagePanels.AddToTail(this);
	m_ModelName[0] = 0;
}

CCSClassImagePanel::~CCSClassImagePanel()
{
	g_ClassImagePanels.FindAndRemove(this);
}

void CCSClassImagePanel::ApplySettings(KeyValues *inResourceData)
{
	const char *pName = inResourceData->GetString("3DModel");
	if (pName)
		Q_strncpy(m_ModelName, pName, sizeof(m_ModelName));

	BaseClass::ApplySettings(inResourceData);
}

void CCSClassImagePanel::Paint()
{
	BaseClass::Paint();
}

CClassMenu_TER::CClassMenu_TER(IViewport *pViewPort)
	: CClassMenu(pViewPort, PANEL_CLASS_TER),
	  m_backgroundLayoutFinished(false)
{
	LoadControlSettings(vgui2::resource_paths::kMenuClassTER);
	CreateBackground(this);
}

const char *CClassMenu_TER::GetName()
{
	return PANEL_CLASS_TER;
}

void CClassMenu_TER::ShowPanel(bool bShow)
{
	BaseClass::ShowPanel(bShow);
}

void CClassMenu_TER::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *pAutoButton = FindChildByName("autoselect_t");
		if (pAutoButton)
			pAutoButton->RequestFocus();
	}
}

void CClassMenu_TER::Update()
{
	SetVisibleButton("CancelButton", cl::g_iTeamNumber == TEAM_TERRORIST);

	// CS Source hides this legacy extra model button unless the matching models
	// are available. Keep the vanilla resource behavior here.
	SetVisibleButton("militia", false);
}

Panel *CClassMenu_TER::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp(controlName, "CSClassImagePanel"))
		return new CCSClassImagePanel(nullptr, controlName);

	return BaseClass::CreateControlByName(controlName);
}

void CClassMenu_TER::PaintBackground()
{
}

void CClassMenu_TER::PerformLayout()
{
	BaseClass::PerformLayout();

	if (!m_backgroundLayoutFinished)
		LayoutBackgroundPanel(this);
	m_backgroundLayoutFinished = true;
}

void CClassMenu_TER::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	ApplyBackgroundSchemeSettings(this, pScheme);
}

CClassMenu_CT::CClassMenu_CT(IViewport *pViewPort)
	: CClassMenu(pViewPort, PANEL_CLASS_CT),
	  m_backgroundLayoutFinished(false)
{
	LoadControlSettings(vgui2::resource_paths::kMenuClassCT);
	CreateBackground(this);
}

Panel *CClassMenu_CT::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp(controlName, "CSClassImagePanel"))
		return new CCSClassImagePanel(nullptr, controlName);

	return BaseClass::CreateControlByName(controlName);
}

const char *CClassMenu_CT::GetName()
{
	return PANEL_CLASS_CT;
}

void CClassMenu_CT::ShowPanel(bool bShow)
{
	BaseClass::ShowPanel(bShow);
}

void CClassMenu_CT::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *pAutoButton = FindChildByName("autoselect_ct");
		if (pAutoButton)
			pAutoButton->RequestFocus();
	}
}

void CClassMenu_CT::Update()
{
	SetVisibleButton("CancelButton", cl::g_iTeamNumber == TEAM_CT);

	// CS Source hides this legacy extra model button unless the matching models
	// are available. Keep the vanilla resource behavior here.
	SetVisibleButton("spetsnaz", false);
}

void CClassMenu_CT::PaintBackground()
{
}

void CClassMenu_CT::PerformLayout()
{
	BaseClass::PerformLayout();

	if (!m_backgroundLayoutFinished)
		LayoutBackgroundPanel(this);
	m_backgroundLayoutFinished = true;
}

void CClassMenu_CT::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	ApplyBackgroundSchemeSettings(this, pScheme);
}
