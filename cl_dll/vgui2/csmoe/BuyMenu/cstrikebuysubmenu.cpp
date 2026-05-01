#include "hud.h"
#include "../../CBaseViewport.h"
#include "../../CBackGroundPanel.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "buy_presets.h"
#include "cstrikebuysubmenu.h"
#include "../vgui_int.h"
#include "shared_util.h"

using namespace vgui2;

CCSBuySubMenu::CCSBuySubMenu(vgui2::Panel *parent, const char *name)
	: CBuySubMenu(parent, name)
{
	m_backgroundLayoutFinished = false;
}

void CCSBuySubMenu::OnCommand(const char *command)
{
	if (!command)
		return;

	const char *buyPresetSetString = "cl_buy_favorite_query_set ";
	if (!Q_strnicmp(command, buyPresetSetString, Q_strlen(buyPresetSetString)))
	{
		char setCommand[64];
		Q_snprintf(setCommand, sizeof(setCommand), "cl_buy_favorite_set %d", Q_atoi(command + Q_strlen(buyPresetSetString)));
		BaseClass::OnCommand(setCommand);
		return;
	}

	if (!Q_stricmp(command, "buy_unavailable"))
	{
		BaseClass::OnCommand("vguicancel");
		return;
	}

	BaseClass::OnCommand(command);
}

void CCSBuySubMenu::OnSizeChanged(int newWide, int newTall)
{
	m_backgroundLayoutFinished = false;
	BaseClass::OnSizeChanged(newWide, newTall);
}

void CCSBuySubMenu::PerformLayout()
{
	BaseClass::PerformLayout();

	int screenW, screenH;
	GetHudSize(screenW, screenH);

	int fullW = scheme()->GetProportionalScaledValueEx(GetScheme(), 640);
	int fullH = scheme()->GetProportionalScaledValueEx(GetScheme(), 480);

	fullW = GetAlternateProportionalValueFromScaled(GetScheme(), fullW);
	fullH = GetAlternateProportionalValueFromScaled(GetScheme(), fullH);

	const int offsetX = (screenW - fullW) / 2;
	const int offsetY = (screenH - fullH) / 2;

	if (!m_backgroundLayoutFinished)
		ResizeWindowControls(this, GetTall(), GetWide(), offsetX, offsetY);
	m_backgroundLayoutFinished = true;

	HandleAvailability();
}

MouseOverPanelButton *CCSBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new BuyMouseOverPanelButton(this, nullptr, panel);
}

CBuySubMenu *CCSBuySubMenu::CreateNewSubMenu(const char *name)
{
	return new CCSBuySubMenu(this, name);
}

void CCSBuySubMenu::HandleAvailability()
{
	for (int i = 0; i < GetChildCount(); ++i)
	{
		BuyMouseOverPanelButton *button = dynamic_cast<BuyMouseOverPanelButton *>(GetChild(i));
		if (button)
			button->RefreshState();
	}
}
