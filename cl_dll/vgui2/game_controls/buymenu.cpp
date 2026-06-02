#include "hud.h"
#include "../CBaseViewport.h"
#include "buymenu.h"
#include "buysubmenu.h"
#include "../csmoe/vgui_int.h"
#include "mouseoverpanelbutton.h"
#include "../vgui_resource_paths.h"
#include "../../../vgui2_support/vgui_controls/controls.h"
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>

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

void CBuyMenu::ClearDefaultButtonState(void)
{
	GetFocusNavGroup().SetCurrentDefaultButton(NULL, false);
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
		// LoadControlSettings(vgui2::resource_paths::kMenuBuy, "GAME");

		// if (!m_pMainMenu)
		// 	m_pMainMenu = new CBuySubMenu(this, "BuySubMenu");

		// if (m_pMainMenu)
		// {
		// 	m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");
		// }

		// Update();

		Run(m_pMainMenu);
		m_pMainMenu->InvalidateLayout();

		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{

		ResetMenuState();
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	m_pViewPort->ShowBackGround( bShow );
}

void CBuyMenu::Update(void)
{
	NULL;
}

void CBuyMenu::PerformLayout(void)
{


	int wide = 0;
	int tall = 0;
	GetHudSize(wide, tall);
	SetPos(0, 0);
	SetSize(wide, tall);

	BaseClass::PerformLayout();

	if (m_pViewPort && IsVisible())
		m_pViewPort->ShowBackGround(true);
}

void CBuyMenu::OnClose(void)
{
	ClearDefaultButtonState();
	ResetHistory();
	BaseClass::OnClose();
	m_pViewPort->ShowBackGround(false);
}

void CBuyMenu::OnCurrentDefaultButtonSet(vgui2::Panel *defaultButton)
{
	if (!defaultButton)
	{
		ClearDefaultButtonState();
		return;
	}

	GetFocusNavGroup().SetCurrentDefaultButton(defaultButton->GetVPanel(), false);
}

void CBuyMenu::ResetMenuState(void)
{
	ResetHistory();
	ResetCurrentSubPanel();

	if (m_pMainMenu)
	{
		m_pMainMenu->DeleteSubPanels();
		m_pMainMenu->SetVisible(false);
	}
}

vgui2::Panel *CBuyMenu::GetInputFocusPanel(void)
{
	vgui2::WizardSubPanel *currentSubPanel = GetCurrentSubPanel();

	if (currentSubPanel && currentSubPanel->IsVisible())
		return currentSubPanel;

	return this;
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
