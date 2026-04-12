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
	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu ENTRY this=%p parent=%p name='%s'\n",
		this, parent, panelName ? panelName : "<null>");

	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu STEP set-proportional begin this=%p\n", this);
	SetProportional(false);
	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu STEP set-proportional end this=%p\n", this);

	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu STEP set-visible begin this=%p\n", this);
	SetVisible(false);
	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu STEP set-visible end this=%p visible=%d\n",
		this, IsVisible() ? 1 : 0);

	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu STEP set-paint-background begin this=%p\n", this);
	SetPaintBackgroundEnabled(true);
	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu STEP set-paint-background end this=%p\n", this);
	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu STEP constructor-final-state this=%p vpanel=%u parent=%p menuType=%d\n",
		this, (unsigned int)GetVPanel(), GetParent(), m_iMenuType);
	printf("[VGUI2-CLIENT] CClassMenu::CClassMenu EXIT this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);
}

void CClassMenu::SetMenuType(int menuType)
{
	printf("[VGUI2-CLIENT] CClassMenu::SetMenuType ENTRY this=%p old=%d new=%d loaded=%d\n",
		this, m_iMenuType, menuType, m_bControlSettingsLoaded ? 1 : 0);
	if (m_iMenuType == menuType)
	{
		printf("[VGUI2-CLIENT] CClassMenu::SetMenuType EARLY-RETURN same-type this=%p\n", this);
		return;
	}

	m_iMenuType = menuType;
	if (m_bControlSettingsLoaded)
	{
		printf("[VGUI2-CLIENT] CClassMenu::SetMenuType reloading controls this=%p menuType=%d\n",
			this, m_iMenuType);
		ReloadControlSettings();
	}
	printf("[VGUI2-CLIENT] CClassMenu::SetMenuType EXIT this=%p menuType=%d\n", this, m_iMenuType);
}

void CClassMenu::EnsureControlSettingsLoaded()
{
	printf("[VGUI2-CLIENT] CClassMenu::EnsureControlSettingsLoaded ENTRY this=%p loaded=%d menuType=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0, m_iMenuType);
	if (m_bControlSettingsLoaded)
	{
		printf("[VGUI2-CLIENT] CClassMenu::EnsureControlSettingsLoaded EARLY-RETURN already-loaded this=%p\n",
			this);
		return;
	}

	printf("[VGUI2-CLIENT] CClassMenu::EnsureControlSettingsLoaded STEP before-ReloadControlSettings this=%p menuType=%d\n",
		this, m_iMenuType);
	ReloadControlSettings();
	m_bControlSettingsLoaded = true;
	printf("[VGUI2-CLIENT] CClassMenu::EnsureControlSettingsLoaded EXIT this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);
}

void CClassMenu::ReloadControlSettings()
{
	const char *pszResource = m_iMenuType == MENU_CLASS_CT ? "Resource/UI/Classmenu_CT.res" : "Resource/UI/Classmenu_TER.res";
	printf("[VGUI2-CLIENT] CClassMenu::ReloadControlSettings ENTRY this=%p resource='%s'\n",
		this, pszResource);
	LoadControlSettings(pszResource);
	printf("[VGUI2-CLIENT] CClassMenu::ReloadControlSettings EXIT this=%p terbutton=%p ctbutton=%p cancel=%p\n",
		this,
		FindChildByName("terrorist"),
		FindChildByName("counterterrorist"),
		FindChildByName("CancelButton"));
}

void CClassMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	printf("[VGUI2-CLIENT] CClassMenu::ApplySchemeSettings ENTRY this=%p scheme=%p\n", this, scheme);
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
	printf("[VGUI2-CLIENT] CClassMenu::ApplySchemeSettings EXIT this=%p\n", this);
}

void CClassMenu::Paint()
{
	printf("[VGUI2-CLIENT] CClassMenu::Paint ENTRY this=%p visible=%d menuType=%d\n",
		this, IsVisible() ? 1 : 0, m_iMenuType);
	BaseClass::Paint();
	printf("[VGUI2-CLIENT] CClassMenu::Paint EXIT this=%p\n", this);
}

void CClassMenu::OnCommand(const char *command)
{
	printf("[VGUI2-CLIENT] CClassMenu::OnCommand ENTRY this=%p command='%s' viewport=%p menuType=%d\n",
		this, command ? command : "<null>", m_pViewport, m_iMenuType);
	if (!command || !command[0])
	{
		printf("[VGUI2-CLIENT] CClassMenu::OnCommand empty-command forwarding to base this=%p\n", this);
		BaseClass::OnCommand(command);
		return;
	}

	if (!strnicmp(command, "joinclass", 9))
	{
		printf("[VGUI2-CLIENT] CClassMenu::OnCommand handling joinclass this=%p command='%s'\n",
			this, command);
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		char szCommand[64];
		snprintf(szCommand, sizeof(szCommand), "%s\n", command);
		VGUI2_RunClientCommand(szCommand);
		printf("[VGUI2-CLIENT] CClassMenu::OnCommand issued joinclass command='%s'\n", szCommand);
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		printf("[VGUI2-CLIENT] CClassMenu::OnCommand handling vguicancel this=%p\n", this);
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		VGUI2_RunClientCommand("cancelselect\n");
		printf("[VGUI2-CLIENT] CClassMenu::OnCommand issued cancelselect\n");
		return;
	}

	printf("[VGUI2-CLIENT] CClassMenu::OnCommand forwarding to base command='%s'\n", command);
	BaseClass::OnCommand(command);
}

#endif
