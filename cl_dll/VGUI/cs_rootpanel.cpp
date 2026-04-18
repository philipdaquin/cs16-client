#include "cs_rootpanel.h"
#include "cs_basepanel.h"

#if !defined(VGUI2_STUB_MODE)

#include <tier2/tier2.h>
#include <IEngineVGui.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>

typedef float vec_t;
typedef vec_t vec3_t[3];

#include "hud.h"

namespace
{

class CCSRootPanel : public CCSBasePanel
{
	DECLARE_CLASS_SIMPLE(CCSRootPanel, CCSBasePanel);

public:
	explicit CCSRootPanel(vgui2::VPANEL parent)
		: BaseClass(NULL, "CSRootPanel")
	{
		SetParent(parent);
		SetPaintEnabled(false);
		SetPaintBorderEnabled(false);
		SetPaintBackgroundEnabled(false);
		SetPostChildPaintEnabled(true);
		UpdateBounds();
	}

	void UpdateBounds()
	{
		int wide = gHUD.m_scrinfo.iWidth;
		int tall = gHUD.m_scrinfo.iHeight;

		if (wide <= 0 || tall <= 0)
		{
			if (vgui2::surface())
				vgui2::surface()->GetScreenSize(wide, tall);
		}

		if (wide <= 0)
			wide = 640;
		if (tall <= 0)
			tall = 480;

		SetBounds(0, 0, wide, tall);
	}
};

static CCSRootPanel *g_pCSRootPanel = NULL;
static vgui2::VPANEL g_OriginalEmbeddedPanel = 0;

static vgui2::VPANEL ResolveRootParent()
{
	if (g_pEngineVGui)
	{
		vgui2::VPANEL enginePanel = g_pEngineVGui->GetPanel(PANEL_CLIENTDLL);
		if (enginePanel)
			return enginePanel;
	}

	return vgui2::surface() ? vgui2::surface()->GetEmbeddedPanel() : 0;
}

}

void VGUI2_CreateClientRootPanel()
{
	if (g_pCSRootPanel || !vgui2::surface())
		return;

	g_OriginalEmbeddedPanel = vgui2::surface()->GetEmbeddedPanel();
	vgui2::VPANEL parent = ResolveRootParent();
	if (!parent)
		return;

	g_pCSRootPanel = new CCSRootPanel(parent);
	vgui2::surface()->SetEmbeddedPanel(g_pCSRootPanel->GetVPanel());
}

void VGUI2_DestroyClientRootPanel()
{
	if (vgui2::surface() && g_OriginalEmbeddedPanel)
		vgui2::surface()->SetEmbeddedPanel(g_OriginalEmbeddedPanel);

	delete g_pCSRootPanel;
	g_pCSRootPanel = NULL;
	g_OriginalEmbeddedPanel = 0;
}

void VGUI2_UpdateClientRootPanelBounds()
{
	if (g_pCSRootPanel)
		g_pCSRootPanel->UpdateBounds();
}

vgui2::VPANEL VGUI2_GetClientRootPanel()
{
	return g_pCSRootPanel ? g_pCSRootPanel->GetVPanel() : 0;
}

#endif
