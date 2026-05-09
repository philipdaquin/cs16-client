#include "cstrikeclassmenu.h"

#include "hud.h"
#include "CBackGroundPanel.h"
#include "vgui_resource_paths.h"
#include "vgui_int.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
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
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_TER ctor this=%p viewport=%p loading='%s'\n",
		this, (void *)pViewPort, vgui2::resource_paths::kMenuClassTER);
	LoadControlSettings(vgui2::resource_paths::kMenuClassTER, "GAME");
	// CreateBackground(this);
}

void CClassMenu_TER::SetupControlSettings()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_TER::SetupControlSettings this=%p loading='%s'\n",
		this, vgui2::resource_paths::kMenuClassTER);
	MouseOverPanelButton::s_lastPanel = nullptr;
	MouseOverPanelButton::s_lastButton = nullptr;
	m_mouseoverButtons.Purge();
	if (EditablePanel *classInfo = dynamic_cast<EditablePanel *>(FindChildByName("ClassInfo")))
		m_pPanel = classInfo;
	LoadControlSettings(vgui2::resource_paths::kMenuClassTER, "GAME");
	if (EditablePanel *classInfo = dynamic_cast<EditablePanel *>(FindChildByName("ClassInfo")))
		m_pPanel = classInfo;
	m_backgroundLayoutFinished = false;
	InvalidateLayout();
}

void CClassMenu_TER::UpdateGameMode()
{
	SetupControlSettings();
}

const char *CClassMenu_TER::GetName()
{
	return PANEL_CLASS_TER;
}

void CClassMenu_TER::ShowPanel(bool bShow)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_TER::ShowPanel this=%p show=%d visible(before)=%d\n",
		this, bShow ? 1 : 0, IsVisible() ? 1 : 0);
	if (bShow)
	{
		int wide = 0;
		int tall = 0;
		GetHudSize(wide, tall);
		SetPos(0, 0);
		SetSize(wide, tall);
		SetupControlSettings();
	}
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
	// Temporary visual test:
	// if (!IsVisible())
	// 	return;
	//
	// surface()->DrawSetColor(Color(160, 40, 40, 220));
	// surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
}

void CClassMenu_TER::PerformLayout()
{
	BaseClass::PerformLayout();

	if (!m_backgroundLayoutFinished)
	{
		LayoutBackgroundPanel(this);
		m_backgroundLayoutFinished = true;
	}
}

void CClassMenu_TER::ApplySchemeSettings(IScheme *pScheme)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_TER::ApplySchemeSettings this=%p scheme=%p resource='%s'\n",
		this, (void *)pScheme, vgui2::resource_paths::kMenuClassTER);
	BaseClass::ApplySchemeSettings(pScheme);
	// ApplyBackgroundSchemeSettings(this, pScheme);
}

CClassMenu_CT::CClassMenu_CT(IViewport *pViewPort)
	: CClassMenu(pViewPort, PANEL_CLASS_CT),
	  m_backgroundLayoutFinished(false)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_CT ctor this=%p viewport=%p loading='%s'\n",
		this, (void *)pViewPort, vgui2::resource_paths::kMenuClassCT);
	LoadControlSettings(vgui2::resource_paths::kMenuClassCT, "GAME");
	// CreateBackground(this);
}

void CClassMenu_CT::SetupControlSettings()
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_CT::SetupControlSettings this=%p loading='%s'\n",
		this, vgui2::resource_paths::kMenuClassCT);
	MouseOverPanelButton::s_lastPanel = nullptr;
	MouseOverPanelButton::s_lastButton = nullptr;
	m_mouseoverButtons.Purge();
	if (EditablePanel *classInfo = dynamic_cast<EditablePanel *>(FindChildByName("ClassInfo")))
		m_pPanel = classInfo;
	LoadControlSettings(vgui2::resource_paths::kMenuClassCT, "GAME");
	if (EditablePanel *classInfo = dynamic_cast<EditablePanel *>(FindChildByName("ClassInfo")))
		m_pPanel = classInfo;
	m_backgroundLayoutFinished = false;
	InvalidateLayout();
}

void CClassMenu_CT::UpdateGameMode()
{
	SetupControlSettings();
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
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_CT::ShowPanel this=%p show=%d visible(before)=%d\n",
		this, bShow ? 1 : 0, IsVisible() ? 1 : 0);
	if (bShow)
	{
		int wide = 0;
		int tall = 0;
		GetHudSize(wide, tall);
		SetPos(0, 0);
		SetSize(wide, tall);
		SetupControlSettings();
	}
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
	// Temporary visual test:
	// if (!IsVisible())
	// 	return;
	//
	// surface()->DrawSetColor(Color(40, 40, 160, 220));
	// surface()->DrawFilledRect(0, 0, GetWide(), GetTall());
}

void CClassMenu_CT::PerformLayout()
{
	BaseClass::PerformLayout();

	if (!m_backgroundLayoutFinished)
	{
		LayoutBackgroundPanel(this);
		m_backgroundLayoutFinished = true;
	}
}

void CClassMenu_CT::ApplySchemeSettings(IScheme *pScheme)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CClassMenu_CT::ApplySchemeSettings this=%p scheme=%p resource='%s'\n",
		this, (void *)pScheme, vgui2::resource_paths::kMenuClassCT);
	BaseClass::ApplySchemeSettings(pScheme);
	// ApplyBackgroundSchemeSettings(this, pScheme);
}
