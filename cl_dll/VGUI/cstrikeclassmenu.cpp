#include "cstrikeclassmenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>

#include "VGUI/counterstrikeviewport.h"
#include "include/tf_defs.h"

CClassMenu::CClassMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
	, m_iMenuType(MENU_CLASS_T)
{
	SetProportional(false);
	SetVisible(false);
	SetPaintBackgroundEnabled(true);
	ReloadControlSettings();
}

void CClassMenu::SetMenuType(int menuType)
{
	if (m_iMenuType == menuType)
		return;

	m_iMenuType = menuType;
	ReloadControlSettings();
}

void CClassMenu::ReloadControlSettings()
{
	LoadControlSettings(m_iMenuType == MENU_CLASS_CT ? "Resource/UI/Classmenu_CT.res" : "Resource/UI/Classmenu_TER.res");
}

void CClassMenu::ApplySchemeSettings(vgui2::HScheme scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
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
		ClientCmd(szCommand);
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		ClientCmd("cancelselect\n");
		return;
	}

	BaseClass::OnCommand(command);
}

#endif
