#include <vgui_controls/RichText.h>
#include "CGameUITestPanel.h"
#include <IEngineVGui.h>
#include "CBaseViewport.h"
#include "cl_dll.h"
#include "cl_util.h"

static void __CmdFunc_OpenGameUITestPanel()
{
	CGameUITestPanel *panel = dynamic_cast<CGameUITestPanel *>(g_pViewport->FindGameUIPanelByName("GameUITestPanel"));
	if (!panel)
	{
		gEngfuncs.Con_Printf("__CmdFunc_OpenGameUITestPanel: panel is NULL\n");
		return;
	}

	panel->Activate();
}

CGameUITestPanel::CGameUITestPanel(vgui2::VPANEL parent) : BaseClass(nullptr, "GameUITestPanel")
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor enter this=%p parent=%p\n", this, (void *)parent);
	BaseClass::SetParent(parent);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after BaseClass::SetParent this=%p parent=%p\n", this, (void *)parent);

	SetKeyBoardInputEnabled(true);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetKeyBoardInputEnabled this=%p\n", this);
	SetMouseInputEnabled(true);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetMouseInputEnabled this=%p\n", this);

	SetProportional(false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetProportional this=%p\n", this);
	SetTitleBarVisible(true);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetTitleBarVisible this=%p\n", this);
	SetMinimizeButtonVisible(false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetMinimizeButtonVisible this=%p\n", this);
	SetMaximizeButtonVisible(false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetMaximizeButtonVisible this=%p\n", this);
	SetCloseButtonVisible(true);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetCloseButtonVisible this=%p\n", this);
	SetSizeable(false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetSizeable this=%p\n", this);
	SetMoveable(true);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetMoveable this=%p\n", this);
	SetVisible(false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetVisible this=%p\n", this);
	SetTitle(L"Quote of the day", false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetTitle this=%p\n", this);

	m_pRichText = new vgui2::RichText(this, "QuoteBox");
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after RichText this=%p richText=%p\n", this, (void *)m_pRichText);
	m_pRichText->SetVerticalScrollbar(false);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetVerticalScrollbar this=%p richText=%p\n", this, (void *)m_pRichText);

	SetScheme("ClientScheme");
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after SetScheme this=%p\n", this);
	Reset();
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after Reset this=%p\n", this);
    gEngfuncs.pfnAddCommand("gameui_open_test_panel", __CmdFunc_OpenGameUITestPanel);
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CGameUITestPanel ctor after pfnAddCommand this=%p\n", this);
}

CGameUITestPanel::~CGameUITestPanel() {}

void CGameUITestPanel::Reset()
{
	constexpr int WIDE = 400, TALL = 200, GAP = 6;
	SetSize(WIDE, TALL);
	MoveToCenterOfScreen();
	m_pRichText->SetSize(WIDE - GAP - GAP, TALL - GAP - GAP - GetCaptionHeight());
	m_pRichText->SetPos(GAP, GetCaptionHeight() + GAP);

	const wchar_t *quotes[] = {
		L"\"DO YOU **** *****?\" - Gunnery Sergeant Hartman, your senior drill instructor",
		L"\"Freeman, you fool!\" - that scientist from the teleport chamber",
		L"\"Did you submit your status report to the administrator, today?\" - Did you submit your status report to the administrator, today?"
	};

	int idx = rand() % ARRAYSIZE(quotes);
	m_pRichText->SetText(quotes[idx]);
}

void CGameUITestPanel::OnCommand(const char* command)
{
	if (!strcmp(command, "Close"))
	{
		m_bIsOpen = false;
		BaseClass::OnCommand(command);
	}
	else BaseClass::OnCommand(command);
}

void CGameUITestPanel::Activate()
{
	if (!m_bIsOpen)
	{
		m_bIsOpen = true;
		Reset();
	}
	BaseClass::Activate();
}

const char *CGameUITestPanel::GetName()
{
	return "GameUITestPanel";
}

void CGameUITestPanel::ShowPanel(bool state)
{
	if (BaseClass::IsVisible() == state)
		return;

	if (state)
	{
		BaseClass::Activate();
	}
	else
	{
		BaseClass::SetVisible(false);
	}
}

void CGameUITestPanel::OnGameUIActivated()
{
	if (m_bIsOpen)
		ShowPanel(true);
}

void CGameUITestPanel::OnGameUIDeactivated()
{
	if (m_bIsOpen)
		ShowPanel(false);
}

vgui2::VPANEL CGameUITestPanel::GetVPanel()
{
	return BaseClass::GetVPanel();
}

bool CGameUITestPanel::IsVisible()
{
	return BaseClass::IsVisible();
}
