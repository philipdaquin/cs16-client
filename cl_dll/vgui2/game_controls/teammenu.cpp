#include <cstdio>

#include "hud.h"
#include "teammenu.h"
#include "../vgui_resource_paths.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#define VECTOR_H

#include <tier1/byteswap.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/RichText.h>

#include "mouseoverpanelbutton.h"

using namespace vgui2;


// helper function
const char *GetStringTeamColor( int i )
{
	switch( i )
	{
	case 0:
		return "team0";

	case 1:
		return "team1";

	case 2:
		return "team2";

	case 3:
		return "team3";

	case 4:
	default:
		return "team4";
	}
}


CTeamMenu::CTeamMenu(IViewport* pViewPort) : Frame(NULL, PANEL_TEAM), m_pViewPort(pViewPort)
{

	// m_pViewPort = pViewPort;
	// m_iJumpKey = BUTTON_CODE_INVALID; // this is looked up in Activate()
	// m_iScoreBoardKey = BUTTON_CODE_INVALID; // this is looked up in Activate()


	// initialize dialog
	SetTitle("#Cstrike_Select_Team", true);
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	SetTitleBarVisible(false);
	// SetPaintTitleWhenTitleBarHidden(true);
	SetProportional(true);

	m_pPanel = new EditablePanel(this, "ClassInfo");
	// Displays the team menu map description text from the map info file.
	// Commented out because this panel is being disabled for now, while the rest of the menu still loads.
	// m_pMapInfo = new RichText(this, "MapInfo");
#if defined (ENABLE_HTML_WINDOW)
	m_pMapInfoHTML = new HTML(this, "MapInfoHTML");
#endif

	LoadControlSettings(vgui2::resource_paths::kMenuTeam, "GAME");

	InvalidateLayout();

	m_szMapName[0] = 0;
}

CTeamMenu::~CTeamMenu(void)
{
}

MouseOverPanelButton *CTeamMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

Panel *CTeamMenu::CreateControlByName(const char *controlName)
{
	if (!Q_stricmp("MouseOverPanelButton", controlName))
	{
		MouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pPanel);
		m_mouseoverButtons.AddToTail(newButton);
		return newButton;
	}
	else
	{
		return BaseClass::CreateControlByName(controlName);
	}
}

void CTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CTeamMenu::ApplySchemeSettings this=%p scheme=%p mapName='%s'\n",
		this, (void *)pScheme, m_szMapName);
	BaseClass::ApplySchemeSettings(pScheme);
	Color mapInfoColor = pScheme->GetColor("MapDescriptionText", Color(255, 255, 255, 255));
	gEngfuncs.Con_Printf(
		"[VGUI2-CLIENT] CTeamMenu::ApplySchemeSettings mapInfoColor this=%p rgba=%d,%d,%d,%d\n",
		this,
		mapInfoColor[0], mapInfoColor[1], mapInfoColor[2], mapInfoColor[3]);
	if (m_pMapInfo)
	{
		m_pMapInfo->SetFgColor(mapInfoColor);
	}

	if (*m_szMapName) {
		LoadMapPage(m_szMapName);
	}

}

void CTeamMenu::AutoAssign(void)
{
	gEngfuncs.pfnClientCmd("jointeam 5");
	OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: shows the team menu
//-----------------------------------------------------------------------------
void CTeamMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
	{
		return;
	}

	int x = 0, y = 0, wide = 0, tall = 0;
	GetBounds(x, y, wide, tall);
	gEngfuncs.Con_Printf(
		"[VGUI2-CLIENT] CTeamMenu::ShowPanel this=%p show=%d visible(before)=%d bounds=%d,%d %dx%d parent=%p parentName='%s'\n",
		this, bShow ? 1 : 0, IsVisible() ? 1 : 0, x, y, wide, tall, (void *)GetVParent(), GetParent() ? GetParent()->GetName() : "<null>");

	if (bShow)
	{
		Activate();

		SetMouseInputEnabled(true);

		for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		{
			if (i == 0)
				m_mouseoverButtons[i]->ShowPage();
			else
				m_mouseoverButtons[i]->HidePage();
		}
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}

	m_pViewPort->ShowBackGround(bShow);
}

void CTeamMenu::PerformLayout()
{
	int screenW, screenH;
	surface()->GetScreenSize(screenW, screenH);

	const int offsetX = 640; //(screenW - 840) / 2;
	const int offsetY =  0; //(screenH - 448) / 2;

	SetPos(offsetX, offsetY);

	BaseClass::PerformLayout();
}

void CTeamMenu::Update(void)
{
	// gEngfuncs.Con_Printf("[VGUI2-CLIENT] CTeamMenu::Update this=%p mapName='%s'\n", this, m_szMapName);
	char mapname[MAX_MAP_NAME];
	Q_FileBase(gEngfuncs.pfnGetLevelName(), mapname, sizeof(mapname));

	SetLabelText("mapname", mapname);
	LoadMapPage(mapname);
}

void CTeamMenu::LoadMapPage(const char *mapName)
{

	Q_strncpy(m_szMapName, mapName, strlen(mapName) + 1);

	char mapRES[MAX_PATH];
	
	if (m_pMapInfo)
	{
		m_pMapInfo->SetVisible(true);
	}
#if defined (ENABLE_HTML_WINDOW)
	m_pMapInfoHTML->SetVisible(false);
#endif
	
	Q_snprintf(mapRES, sizeof(mapRES), "maps/%s.txt", mapName);

	if (!filesystem()->FileExists(mapRES))
	{
		if (filesystem()->FileExists("maps/default.txt"))
		{
			Q_snprintf(mapRES, sizeof(mapRES), "maps/default.txt");
		}
		else
		{
			if (m_pMapInfo)
			{
				m_pMapInfo->SetText("");
			}
			return;
		}
	}

	FileHandle_t f = filesystem()->Open(mapRES, "r");
	// gEngfuncs.Con_Printf("[VGUI2-CLIENT] CTeamMenu::LoadMapPage opened map res this=%p mapRES='%s' handle=%p\n",
	// 	this, mapRES, (void *)f);

	int fileSize = filesystem()->Size(f);
	int dataSize = fileSize + sizeof(wchar_t);

	if (dataSize % 2)
		++dataSize;

	wchar_t *memBlock = (wchar_t *)malloc(dataSize);
	int bytesRead = filesystem()->Read(memBlock, fileSize, f);

	if (bytesRead < fileSize)
	{
		char *data = reinterpret_cast<char *>(memBlock);
		data[bytesRead] = 0;
		data[bytesRead + 1] = 0;
	}

	memBlock[dataSize / sizeof(wchar_t) - 1] = 0x0000;

	CByteswap byteSwap;
	byteSwap.SetTargetBigEndian(false);
	byteSwap.SwapBufferToTargetEndian(memBlock, memBlock, dataSize / sizeof(wchar_t));

	if (m_pMapInfo)
	{
		if (memBlock[0] != 0xFEFF)
			m_pMapInfo->SetText(reinterpret_cast<char *>(memBlock));
		else
			m_pMapInfo->SetText(memBlock + 1);

		m_pMapInfo->GotoTextStart();
	}

	filesystem()->Close(f);
	free(memBlock);

	InvalidateLayout();
	Repaint();

}

void CTeamMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry) 
	{
		entry->SetText(text);
	}
}

void CTeamMenu::Reset(void)
{
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CTeamMenu::Reset this=%p childCount=%d\n", this, GetChildCount());
	for (int i = 0; i < GetChildCount(); ++i)
	{
		MouseOverPanelButton *pPanel = dynamic_cast<MouseOverPanelButton *>(GetChild(i));

		if (pPanel) { 
			pPanel->HidePage();
		}
	}

	for (int i = 0; i < m_mouseoverButtons.Count(); ++i)
		m_mouseoverButtons[i]->HidePage();
	gEngfuncs.Con_Printf("[VGUI2-CLIENT] CTeamMenu::Reset complete this=%p\n", this);
}
