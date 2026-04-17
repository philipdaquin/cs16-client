#include "buymouseoverpanelbutton.h"

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/MessageMap.h>
#include <vgui/ISurface.h>

static vgui2::Panel *CreateMouseOverPanelButton()
{
	return new vgui2::MouseOverPanelButton(NULL, NULL, "");
}

DECLARE_BUILD_FACTORY_CUSTOM_ALIAS(vgui2::MouseOverPanelButton, MouseOverPanelButton, CreateMouseOverPanelButton);

vgui2::MouseOverPanelButton::MouseOverPanelButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget, const char *pCmd)
	: BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	, m_pPage(NULL)
{
}

vgui2::MouseOverPanelButton::MouseOverPanelButton(Panel *parent, const char *panelName, const wchar_t *text, Panel *pActionSignalTarget, const char *pCmd)
	: BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	, m_pPage(NULL)
{
}

vgui2::MouseOverPanelButton::MouseOverPanelButton(Panel *parent, const char *panelName, EditablePanel *panel)
	: BaseClass(parent, panelName, "")
	, m_pPage(panel)
{
}

void vgui2::MouseOverPanelButton::ShowPage()
{
	if (m_pPage)
		m_pPage->SetVisible(true);
}

void vgui2::MouseOverPanelButton::HidePage()
{
	if (m_pPage)
		m_pPage->SetVisible(false);
}

void vgui2::MouseOverPanelButton::OnMousePressed(MouseCode code)
{
	if (code == MOUSE_LEFT)
	{
		FireActionSignal();
	}
}

void vgui2::MouseOverPanelButton::OnMouseReleased(MouseCode code)
{
}

void vgui2::MouseOverPanelButton::OnMouseDoublePressed(MouseCode code)
{
	OnMousePressed(code);
}

#endif
