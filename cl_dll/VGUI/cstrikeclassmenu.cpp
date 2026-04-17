#include "cstrikeclassmenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"
#include "../dlls/cdll_dll.h"

CClassMenu::CClassMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
	, m_iMenuType(MENU_CLASS_T)
	, m_bControlSettingsLoaded(false)
{
	SetScheme("ClientScheme");
	SetProportional(true);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	SetPaintBorderEnabled(true);
	SetMoveable(false);
	SetSizeable(false);
	SetTitleBarVisible(false);

	LoadControlSettings(m_iMenuType == MENU_CLASS_CT ? "Resource/UI/Classmenu_CT.res" : "Resource/UI/Classmenu_TER.res");
	InvalidateLayout(true, true);
	m_bControlSettingsLoaded = true;
}

void CClassMenu::SetMenuType(int menuType)
{
	if (m_iMenuType == menuType)
		return;

	m_iMenuType = menuType;
	if (m_bControlSettingsLoaded)
		ReloadControlSettings();
}

void CClassMenu::EnsureControlSettingsLoaded()
{
	if (m_bControlSettingsLoaded)
		return;

	ReloadControlSettings();
	m_bControlSettingsLoaded = true;
}

void CClassMenu::ReloadControlSettings()
{
	const char *pszResource = m_iMenuType == MENU_CLASS_CT ? "Resource/UI/Classmenu_CT.res" : "Resource/UI/Classmenu_TER.res";
	LoadControlSettings(pszResource);
	InvalidateLayout(true, true);
}

void CClassMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetBgColor(scheme->GetColor("BgColor", Color(0, 0, 0, 0)));
	SetPaintBackgroundEnabled(true);
}

void CClassMenu::Paint()
{
	BaseClass::Paint();
}

void CClassMenu::OnCommand(const char *command)
{
	if (!command || !command[0])
	{
		BaseClass::OnCommand(command);
		return;
	}

	if (!strnicmp(command, "joinclass", 9))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		char szCommand[64];
		snprintf(szCommand, sizeof(szCommand), "%s\n", command);
		VGUI2_RunClientCommand(szCommand);
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		VGUI2_RunClientCommand("cancelselect\n");
		return;
	}

	BaseClass::OnCommand(command);
}

#endif
