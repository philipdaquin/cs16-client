#include "hud.h"
#include "../CBaseViewport.h"
#include "buymenu.h"
#include "buysubmenu.h"
#include "mouseoverpanelbutton.h"
#include "../vgui_resource_paths.h"

using namespace vgui2;

CBuyMenu::CBuyMenu(IViewport *pViewPort) : WizardPanel(NULL, PANEL_BUY), m_pViewPort(pViewPort)
{
	SetScheme("ClientScheme");
	SetTitle("#Cstrike_Buy_Menu", true);

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	SetTitleBarVisible(false);
	// SetPaintTitleWhenTitleBarHidden(true);
	SetAutoDelete(false);

	m_pMainMenu = new CBuySubMenu(this, "BuySubMenu");

	LoadControlSettings(vgui2::resource_paths::kMenuBuy, "GAME");

	m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");
	m_pMainMenu->SetVisible(false);

	ShowButtons(false);
}

CBuyMenu::~CBuyMenu(void)
{
	if (m_pMainMenu)
		m_pMainMenu->DeleteSubPanels();
}

void CBuyMenu::Init(void)
{
}

void CBuyMenu::VidInit(void)
{
	SetVisible(false);
}

void CBuyMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;


	if (bShow)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] CBuyMenu::ShowPanel reloading base buy resources this=%p buy='%s' main='%s'\n",
			this, vgui2::resource_paths::kMenuBuy, vgui2::resource_paths::kMenuBuyMain);
		LoadControlSettings(vgui2::resource_paths::kMenuBuy, "GAME");
		if (m_pMainMenu)
			m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");

		Update();

		// Run(m_pMainMenu);

		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	m_pViewPort->ShowBackGround( bShow );
}

void CBuyMenu::Update(void)
{
	NULL;
}

void CBuyMenu::OnClose(void)
{
	BaseClass::OnClose();

	ResetHistory();
}

void CBuyMenu::OnKeyCodeTyped(KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		OnClose();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}
