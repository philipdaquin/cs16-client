#include "hud.h"
#include "../CBaseViewport.h"
#include "buymenu.h"
#include "buysubmenu.h"
#include "mouseoverpanelbutton.h"
#include "../vgui_resource_paths.h"
#include "../csmoe/vgui_int.h"
#include "../../../vgui2_support/vgui_controls/controls.h"
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include <vgui/IInputInternal.h>

using namespace vgui2;

CBuyMenu::CBuyMenu(IViewport *pViewPort) : WizardPanel(NULL, PANEL_BUY), m_pViewPort(pViewPort)
{
	SetScheme("ClientScheme");
	SetTitle("#Cstrike_Buy_Menu", true);



gEngfuncs.Con_Printf("[CBuyMenu::CBuyMenu]");

	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	SetTitleBarVisible(false);
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
	
		// m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");

		// LoadControlSettings(vgui2::resource_paths::kMenuBuy, "GAME");

		// if (!m_pMainMenu)
		// 	m_pMainMenu = new CBuySubMenu(this, "BuySubMenu");

		// m_pMainMenu->LoadControlSettings(vgui2::resource_paths::kMenuBuyMain, "GAME");


		Update();

		Run(m_pMainMenu);

		// Activate();
		SetMouseInputEnabled(true);
	}
	else
	{

		// ResetMenuState();
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
	int screenW, screenH;
	surface()->GetScreenSize(screenW, screenH);

	const int menuW = scheme()->GetProportionalScaledValue(640);
	const int menuH = scheme()->GetProportionalScaledValue(480);

	const int offsetX = ((screenW - menuW) / 2) + 5;
	const int offsetY = (screenH - menuH) / 2;

	SetPos(offsetX, offsetY);

	BaseClass::PerformLayout();

	if (m_pViewPort)
		m_pViewPort->ShowBackGround(true);
}

void CBuyMenu::OnClose(void)
{
	ResetHistory();
	BaseClass::OnClose();
	m_pViewPort->ShowBackGround(false);
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
