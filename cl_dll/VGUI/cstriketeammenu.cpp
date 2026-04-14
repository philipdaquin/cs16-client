#include "cstriketeammenu.h"

#if !defined(VGUI2_STUB_MODE)

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include <FileSystem.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Button.h>
#include <tier1/KeyValues.h>
#include <tier2/tier2.h>

#include "VGUI/counterstrikeviewport.h"
#include "VGUI/counterstrikeviewport_interface.h"
#include "vdf_parser.hpp"

namespace
{

bool ReadFileToString(const char *path, std::string &out)
{
	if (!g_pFullFileSystem || !path || !path[0])
		return false;

	FileHandle_t file = g_pFullFileSystem->Open(path, "rb");
	if (file == FILESYSTEM_INVALID_HANDLE)
		return false;

	unsigned int size = g_pFullFileSystem->Size(file);
	out.assign(size, '\0');
	if (size > 0)
	{
		int read = g_pFullFileSystem->Read(&out[0], static_cast<int>(size), file);
		if (read < 0)
		{
			g_pFullFileSystem->Close(file);
			return false;
		}
		out.resize(static_cast<size_t>(read));
	}

	g_pFullFileSystem->Close(file);
	return true;
}

KeyValues *ConvertVdfNodeToKeyValues(const tyti::vdf::multikey_object &node)
{
	KeyValues *kv = new KeyValues(node.name.c_str());

	for (const auto &attrib : node.attribs)
	{
		kv->SetString(attrib.first.c_str(), attrib.second.c_str());
	}

	for (const auto &childEntry : node.childs)
	{
		if (!childEntry.second)
			continue;

		kv->AddSubKey(ConvertVdfNodeToKeyValues(*childEntry.second));
	}

	return kv;
}

KeyValues *LoadKeyValuesWithVdfParser(const char *resourcePath)
{
	std::string fileData;
	if (!ReadFileToString(resourcePath, fileData))
	{
		printf("[TEAMTRACE] VDF parser failed to read '%s'\n", resourcePath ? resourcePath : "<null>");
		return NULL;
	}

	bool ok = false;
	tyti::vdf::Options options;
	options.ignore_includes = false;
	options.ignore_all_platform_conditionals = false;
	options.strip_escape_symbols = true;

	tyti::vdf::multikey_object root =
		tyti::vdf::read<tyti::vdf::multikey_object>(fileData.begin(), fileData.end(), &ok, options);
	if (!ok)
	{
		printf("[TEAMTRACE] VDF parser failed to parse '%s'\n", resourcePath ? resourcePath : "<null>");
		return NULL;
	}

	printf("[TEAMTRACE] VDF parser parsed root name='%s' attribs=%zu childs=%zu\n",
		root.name.c_str(), root.attribs.size(), root.childs.size());

	return ConvertVdfNodeToKeyValues(root);
}

void DumpPanelTree(vgui2::Panel *panel, int depth)
{
	if (!panel || depth > 3)
		return;

	int x = 0, y = 0, w = 0, h = 0;
	panel->GetBounds(x, y, w, h);
	printf("[VGUI2-CLIENT] TeamMenuTree depth=%d panel=%p name='%s' visible=%d enabled=%d bounds=(%d,%d %dx%d) children=%d\n",
		depth,
		panel,
		panel->GetName() ? panel->GetName() : "<null>",
		panel->IsVisible() ? 1 : 0,
		panel->IsEnabled() ? 1 : 0,
		x, y, w, h,
		panel->GetChildCount());

	for (int i = 0; i < panel->GetChildCount(); ++i)
	{
		DumpPanelTree(panel->GetChild(i), depth + 1);
	}
}

}

CTeamMenu::CTeamMenu(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_pViewport(NULL)
	, m_bControlSettingsLoaded(false)
{
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu ENTRY this=%p parent=%p name='%s'\n",
		this, parent, panelName ? panelName : "<null>");

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-proportional begin this=%p\n", this);
	SetProportional(true);
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-proportional end this=%p\n", this);

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-visible begin this=%p\n", this);
	SetVisible(false);
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-visible end this=%p visible=%d\n",
		this, IsVisible() ? 1 : 0);

	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-paint-background begin this=%p\n", this);
	SetPaintBackgroundEnabled(true);
	SetMoveable(false);
	SetSizeable(false);
	SetTitleBarVisible(false);   // .res has no titlebar, Frame would paint one by default
	SetClipToParent(false);      // don't let viewport clip the menu
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP set-paint-background end this=%p\n", this);
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu STEP constructor-final-state this=%p vpanel=%u parent=%p\n",
		this, (unsigned int)GetVPanel(), GetParent());
	printf("[VGUI2-CLIENT] CTeamMenu::CTeamMenu EXIT this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);
}

void CTeamMenu::EnsureControlSettingsLoaded()
{
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded ENTRY this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);

	if (m_bControlSettingsLoaded)
	{
		printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded EARLY-RETURN already-loaded this=%p\n",
			this);
		return;
	}

	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded STEP before-LoadControlSettings this=%p\n",
		this);

	vgui2::HScheme clientScheme = 0;
	if (vgui2::scheme())
	{
		clientScheme = vgui2::scheme()->LoadSchemeFromFilePath("resource/ClientScheme.res", NULL, "clientscheme");
		if (clientScheme)
		{
			SetScheme(clientScheme);
			printf("[VGUI_RES] Bound scheme: resource/ClientScheme.res tag='clientscheme' panel=%p scheme=%lu\n",
				this, (unsigned long)clientScheme);
		}
		else
		{
			printf("[VGUI_RES] Scheme load failed: resource/ClientScheme.res tag='clientscheme' panel=%p\n",
				this);
		}
	}
	else
	{
		printf("[VGUI_RES] Scheme manager unavailable before TeamMenu load panel=%p\n", this);
	}

	BaseClass::LoadControlSettings("Resource/UI/Teammenu.res");
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded STEP after-LoadControlSettings this=%p\n",
		this);
	vgui2::Panel *pRootFrame = FindChildByName("TeamMenu");
	if (pRootFrame)
	{
		printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded keeping root frame paint enabled root=%p\n",
			pRootFrame);
	}
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded dumping panel tree this=%p\n", this);
	DumpPanelTree(this, 0);
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded STEP child lookup rootframe=%p terbutton=%p ctbutton=%p specbottom=%p mapinfo=%p sysmenu=%p\n",
		pRootFrame,
		FindChildByName("terbutton"),
		FindChildByName("ctbutton"),
		FindChildByName("spec-bottom"),
		FindChildByName("MapInfo"),
		FindChildByName("SysMenu"));
	m_bControlSettingsLoaded = true;
	printf("[VGUI2-CLIENT] CTeamMenu::EnsureControlSettingsLoaded EXIT this=%p loaded=%d\n",
		this, m_bControlSettingsLoaded ? 1 : 0);
}

void CTeamMenu::ApplySchemeSettings(vgui2::IScheme *scheme)
{
	printf("[VGUI2-CLIENT] CTeamMenu::ApplySchemeSettings ENTRY this=%p scheme=%p\n", this, scheme);
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(true);
	printf("[VGUI2-CLIENT] CTeamMenu::ApplySchemeSettings EXIT this=%p\n", this);
}

void CTeamMenu::Paint()
{
	BaseClass::Paint();
}

void CTeamMenu::SetSpectateVisible(bool bVisible)
{
	printf("[VGUI2-CLIENT] CTeamMenu::SetSpectateVisible ENTRY this=%p visible=%d\n",
		this, bVisible ? 1 : 0);
	vgui2::Panel *pSpectateButton = FindChildByName("spec-bottom");
	if (!pSpectateButton)
	{
		printf("[VGUI2-CLIENT] CTeamMenu::SetSpectateVisible missing spec-bottom this=%p\n", this);
		return;
	}

	pSpectateButton->SetVisible(bVisible);
	pSpectateButton->SetEnabled(bVisible);
	printf("[VGUI2-CLIENT] CTeamMenu::SetSpectateVisible EXIT this=%p button=%p\n",
		this, pSpectateButton);
}

void CTeamMenu::OnCommand(const char *command)
{
	printf("[VGUI2-CLIENT] CTeamMenu::OnCommand ENTRY this=%p command='%s' viewport=%p\n",
		this, command ? command : "<null>", m_pViewport);
	if (!command || !command[0])
	{
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand empty-command forwarding to base this=%p\n", this);
		BaseClass::OnCommand(command);
		return;
	}

	if (!strnicmp(command, "jointeam", 8))
	{
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand handling jointeam this=%p command='%s'\n",
			this, command);
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		char szCommand[64];
		snprintf(szCommand, sizeof(szCommand), "%s\n", command);
		VGUI2_RunClientCommand(szCommand);
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand issued jointeam command='%s'\n", szCommand);
		return;
	}

	if (!stricmp(command, "vguicancel"))
	{
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand handling vguicancel this=%p\n", this);
		if (m_pViewport)
			m_pViewport->HideAllGameMenus();

		VGUI2_RunClientCommand("cancelselect\n");
		printf("[VGUI2-CLIENT] CTeamMenu::OnCommand issued cancelselect\n");
		return;
	}

	printf("[VGUI2-CLIENT] CTeamMenu::OnCommand forwarding to base command='%s'\n", command);
	BaseClass::OnCommand(command);
}

#endif
