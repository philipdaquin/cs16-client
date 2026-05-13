
#include "hud.h"
#include "vgui_int.h"
#include <vgui/ISurface.h>
#include "../../util_vector.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "r_studioint.h"
#include "CSBackGroundPanel.h"
#include "../vgui_resource_paths.h"

using namespace vgui2;

#define DEBUG_WINDOW_RESIZING 0
#define DEBUG_WINDOW_REPOSITIONING 0

extern engine_studio_api_t IEngineStudio;

// CS viewport background panel.
// Draw the translucent chrome in PaintBackground() and the logo in
// PostChildPaint() so the logo stays above the chrome.
CCSBackGroundPanel::CCSBackGroundPanel(Panel *parent) : BaseClass(parent)
{
	// Corner, gap, and title controls still participate in the normal layout pass.
	m_pTopLeftPanel = new CBitmapImagePanel(this, "TopLeftPanel", "gfx/vgui/round_corner_nw");
	m_pTopRightPanel = new CBitmapImagePanel(this, "TopRightPanel", "gfx/vgui/round_corner_ne");
	m_pBottomLeftPanel = new CBitmapImagePanel(this, "BottomLeftPanel", "gfx/vgui/round_corner_sw");
	m_pBottomRightPanel = new CBitmapImagePanel(this, "BottomRightPanel", "gfx/vgui/round_corner_se");

	m_pGapPanel = new Panel(this, "GapPanel");
	m_pTitleLabel = new Label(this, "CaptionLabel", "");

	// Keep the logo as a child for layout, but draw it manually in PostChildPaint().
	m_pExclamationPanel = new CBitmapImagePanel(this, "ExclamationPanel", "gfx/vgui/CS_logo");
	// Previous stacking attempt kept here for reference:
	// m_pExclamationPanel->SetVisible(m_enabled);
	// m_pExclamationPanel->SetZPos(1);

	m_offsetX = 0;
	m_offsetY = 0;

	LoadControlSettings(vgui2::resource_paths::kMenuBackgroundPanel, "GAME");
	m_enabled = true;

	// Enable the late paint hook so the logo can be drawn after the chrome.
	SetPostChildPaintEnabled(true);


	m_pTopLeftPanel->SetVisible(m_enabled);
	m_pTopRightPanel->SetVisible(m_enabled);
	m_pBottomLeftPanel->SetVisible(m_enabled);
	m_pBottomRightPanel->SetVisible(m_enabled);
	m_pGapPanel->SetVisible(m_enabled);
	m_pTitleLabel->SetVisible(m_enabled);
}

void CCSBackGroundPanel::SetTitleText(const wchar_t *text)
{
	m_pTitleLabel->SetText(text);
}

void CCSBackGroundPanel::SetTitleText(const char *text)
{
	m_pTitleLabel->SetText(text);
}

void CCSBackGroundPanel::PaintBackground(void)
{
	if (!m_enabled)
		return;

	// Paint the translucent menu chrome first.
	surface()->DrawSetColor(m_bgColor);

	if (IEngineStudio.IsHardware())
	{
		int x1, y1, x2, y2, x3, y3;

		if (m_pGapPanel->GetTall() > m_pGapPanel->GetWide())
		{
			m_pTopLeftPanel->GetPos(x1, y1);
			m_pBottomLeftPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1, y1 + m_pTopLeftPanel->GetTall(), x2 + m_pBottomLeftPanel->GetWide(), y2);

			m_pTopLeftPanel->GetPos(x1, y1);
			m_pGapPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pTopLeftPanel->GetWide(), y1, x2, y2 + m_pGapPanel->GetTall());

			m_pGapPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pGapPanel->GetWide(), y1, x2, y2 + m_pBottomRightPanel->GetTall());

			m_pTopRightPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1, y1 + m_pTopRightPanel->GetTall(), x2 + m_pBottomRightPanel->GetWide(), y2);
		}
		else
		{
			m_pTopLeftPanel->GetPos(x1, y1);
			m_pTopRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pTopLeftPanel->GetWide(), y1, x2, y2 + m_pTopRightPanel->GetTall());

			m_pTopLeftPanel->GetPos(x1, y1);
			m_pGapPanel->GetPos(x2, y2);
			m_pTopRightPanel->GetPos(x3, y3);

			surface()->DrawFilledRect(x1, y1 + m_pTopLeftPanel->GetTall(), x3 + m_pTopRightPanel->GetWide(), y2);

			m_pGapPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1, y1 + m_pGapPanel->GetTall(), x2 + m_pBottomRightPanel->GetWide(), y2);

			m_pBottomLeftPanel->GetPos(x1, y1);
			m_pBottomRightPanel->GetPos(x2, y2);

			surface()->DrawFilledRect(x1 + m_pBottomLeftPanel->GetWide(), y1, x2, y2 + m_pBottomRightPanel->GetTall());
		}
	}
}

void CCSBackGroundPanel::PostChildPaint(void)
{
	if (!m_enabled || !m_pExclamationPanel)
		return;

	// Draw the logo after the chrome so it stays on top.
	surface()->PushMakeCurrent(m_pExclamationPanel->GetVPanel(), false);
	m_pExclamationPanel->PaintBackground();
	surface()->PopMakeCurrent(m_pExclamationPanel->GetVPanel());
}

static int GetAlternateProportionalValueFromNormal(int normalizedValue)
{
	int wide, tall;
	GetHudSize(wide, tall);

	int proH, proW;
	surface()->GetProportionalBase(proW, proH);

	double scaleH = (double)tall / (double)proH;
	double scaleW = (double)wide / (double)proW;
	double scale = (scaleW < scaleH) ? scaleW : scaleH;

	return (int)(normalizedValue * scale);
}

static int GetCSLegacyAlternateProportionalValueFromScaled(HScheme hScheme, int scaledValue)
{
	return GetAlternateProportionalValueFromNormal(scheme()->GetProportionalNormalizedValueEx(hScheme, scaledValue));
}

static void RepositionControl(Panel *pPanel)
{
	int x, y, w, h;
	pPanel->GetBounds(x, y, w, h);

#if DEBUG_WINDOW_RESIZING
	int x1, y1, w1, h1;
	pPanel->GetBounds(x1, y1, w1, h1);

	int x2, y2, w2, h2;
	x2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), x1);
	y2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), y1);
	w2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), w1);
	h2 = scheme()->GetProportionalNormalizedValueEx(pPanel->GetScheme(), h1);
#endif

	x = GetCSLegacyAlternateProportionalValueFromScaled(pPanel->GetScheme(), x);
	y = GetCSLegacyAlternateProportionalValueFromScaled(pPanel->GetScheme(), y);
	w = GetCSLegacyAlternateProportionalValueFromScaled(pPanel->GetScheme(), w);
	h = GetCSLegacyAlternateProportionalValueFromScaled(pPanel->GetScheme(), h);

	pPanel->SetBounds(x, y, w, h);

#if DEBUG_WINDOW_RESIZING
	DevMsg("Resizing '%s' from (%d,%d) %dx%d to (%d,%d) %dx%d -- initially was (%d,%d) %dx%d\n", pPanel->GetName(), x1, y1, w1, h1, x, y, w, h, x2, y2, w2, h2);
#endif
}

static void ResizeCSLegacyWindowControls(EditablePanel *pWindow, int tall, int wide, int offsetX, int offsetY)
{
	if (!pWindow || !pWindow->GetBuildGroup() || !pWindow->GetBuildGroup()->GetPanelList())
		return;

	CUtlVector<PHandle> *panelList = pWindow->GetBuildGroup()->GetPanelList();
	CUtlVector<Panel *> resizedPanels;
	CUtlVector<Panel *> movedPanels;

	for (int i = 0; i < panelList->Size(); ++i)
	{
		PHandle handle = (*panelList)[i];
		Panel *panel = handle.Get();
		bool found = false;

		for (int j = 0; j < resizedPanels.Size(); ++j)
		{
			if (panel == resizedPanels[j])
				found = true;
		}

		if (!panel || found)
		{
			continue;
		}

		resizedPanels.AddToTail(panel);

		if (panel != pWindow)
		{
			RepositionControl(panel);
		}
	}

	for (int i = 0; i < panelList->Size(); ++i)
	{
		PHandle handle = (*panelList)[i];
		Panel *panel = handle.Get();
		bool found = false;

		for (int j = 0; j < movedPanels.Size(); ++j)
		{
			if (panel == movedPanels[j])
				found = true;
		}

		if (!panel || found)
		{
			continue;
		}

		movedPanels.AddToTail(panel);

		if (panel != pWindow)
		{
			int x, y;

			panel->GetPos(x, y);
			panel->SetPos(x + offsetX, y + offsetY);

#if DEBUG_WINDOW_REPOSITIONING
			DevMsg("Repositioning '%s' from (%d,%d) to (%d,%d) -- a distance of (%d,%d)\n", panel->GetName(), x, y, x + offsetX, y + offsetY, offsetX, offsetY);
#endif
		}
	}
}

void CCSBackGroundPanel::Activate(void)
{
	if (!m_enabled)
		return;


	    BaseClass::Activate();

    int screenW, screenH;
    GetHudSize(screenW, screenH);

    if (IsProportional())
    {
        int wide = GetCSLegacyAlternateProportionalValueFromScaled(
            GetScheme(), scheme()->GetProportionalNormalizedValue(640));
        int tall = GetCSLegacyAlternateProportionalValueFromScaled(
            GetScheme(), scheme()->GetProportionalNormalizedValue(480));

        // True center — don't subtract axes from each other
        int offsetX = (screenW - wide) / 2;
        int offsetY = (screenH - tall) / 2;

        m_offsetX = offsetX;
        m_offsetY = offsetY;

        ResizeCSLegacyWindowControls(this, tall, wide, offsetX, offsetY);
    }

	// int posX = 0, posY = 0, posW = 0, posH = 0;
	// GetBounds(posX, posY, posW, posH);

	// gEngfuncs.Con_Printf(
	// 	"[VGUI2-CLIENT] CCSBackGroundPanel::Activate entry this=%p parent=%p parentName='%s' pos=%d,%d size=%dx%d visible=%d\n",
	// 	this,
	// 	(void *)GetVParent(),
	// 	GetParent() ? GetParent()->GetName() : "<null>",
	// 	posX, posY, posW, posH,
	// 	IsVisible() ? 1 : 0);

	// BaseClass::Activate();
	// MoveToCenterOfScreen();

	// if (IsProportional())
	// {
	// 	int screenW, screenH;
	// 	GetHudSize(screenW, screenH);

	// 	int wide, tall;
	// 	GetSize(wide, tall);

	// 	gEngfuncs.Con_Printf(
	// 		"[VGUI2-CLIENT] CCSBackGroundPanel::Activate state this=%p size=%dx%d hud=%dx%d parent=%p parentName='%s'\n",
	// 		this, wide, tall, screenW, screenH, (void *)GetVParent(), GetParent() ? GetParent()->GetName() : "<null>");

	// 	if (wide != screenW || tall != screenH)
	// 	{
	// 		wide = GetCSLegacyAlternateProportionalValueFromScaled(GetScheme(), scheme()->GetProportionalNormalizedValue(640));
	// 		tall = GetCSLegacyAlternateProportionalValueFromScaled(GetScheme(), scheme()->GetProportionalNormalizedValue(480));

	// 		int offsetX = (screenW - wide) / 2;
	// 		int offsetY = (screenH - tall) / 2;

	// 		if (offsetX != 0 && offsetY != 0)
	// 		{
	// 			if (offsetX > offsetY)
	// 			{
	// 				offsetX = offsetX - offsetY;
	// 				offsetY = 0;
	// 			}
	// 			else
	// 			{
	// 				offsetY = offsetY - offsetX;
	// 				offsetX = 0;
	// 			}
	// 		}

	// 		gEngfuncs.Con_Printf(
	// 			"[VGUI2-CLIENT] CCSBackGroundPanel::Activate centered layout this=%p content=%dx%d offset=%d,%d adjusted=%d,%d\n",
	// 			this, wide, tall, offsetX, offsetY, screenW, screenH);
	// 		ResizeCSLegacyWindowControls(this, tall, wide, offsetX, offsetY);
	// 	}
	// }
}

void CCSBackGroundPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	// m_bgColor = pScheme->GetColor("BgColor", Color(0, 0, 0, 200));
	// m_titleColor = pScheme->GetColor("TitleIcon", Color(0, 0, 0, 200));
	// m_borderColor = Color(0, 0, 0, 200);
	m_titleColor = pScheme->GetColor("TitleIcon", Color(0, 0, 0, 200));
	m_bgColor = Color(0, 0, 0, 255);
	m_borderColor = Color(0, 0, 0, 255);

	if (!IEngineStudio.IsHardware())
	{


		
		// m_pTopLeftPanel->setImageColor(pScheme->GetColor("BorderBright", Color(0, 0, 0, 0)));
		// m_pTopRightPanel->setImageColor(Color(0, 0, 0, 0));
		// m_pBottomLeftPanel->setImageColor(Color(0, 0, 0, 0));
		// m_pBottomRightPanel->setImageColor(Color(0, 0, 0, 0));
		m_pTopLeftPanel->setImageColor(Color(255, 255, 255, 255));
		m_pTopRightPanel->setImageColor(Color(255, 255, 255, 255));
		m_pBottomLeftPanel->setImageColor(Color(255, 255, 255, 255));
		m_pBottomRightPanel->setImageColor(Color(255, 255, 255, 255));
	}
	else
	{
		// Color bgColor = Color(255, 255, 255, pScheme->GetColor("BgColor", Color(0, 0, 0, 0))[3]);
		Color bgColor = Color(255, 255, 255, 255);

		m_pTopLeftPanel->setImageColor(bgColor);
		m_pTopRightPanel->setImageColor(bgColor);
		m_pBottomLeftPanel->setImageColor(bgColor);
		m_pBottomRightPanel->setImageColor(bgColor);
	}
}

void CCSBackGroundPanel::PerformLayout(void)
{
	BaseClass::PerformLayout();
}
