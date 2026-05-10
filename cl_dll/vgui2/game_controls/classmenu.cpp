#include "hud.h"
#include "classmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

#include "mouseoverpanelbutton.h"
#include "../vgui_resource_paths.h"

using namespace vgui2;

static const char *GetClassMenuResourceForTeam(int team)
{
	if (team == TEAM_TERRORIST)
		return vgui2::resource_paths::kMenuClassTER;

	return vgui2::resource_paths::kMenuClassCT;
}

CClassMenu::CClassMenu(IViewport* pViewPort) : Frame(NULL, PANEL_CLASS), m_pViewPort(pViewPort)
{
	m_iTeam = 0;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	// SetPaintTitleWhenTitleBarHidden(true);
	SetProportional(true);

	// info window about this class
	m_pPanel = new EditablePanel( this, "ClassInfo" );

	// Split CS class menus load Classmenu_CT.res/Classmenu_TER.res in their derived constructors.
}

CClassMenu::CClassMenu(IViewport* pViewPort, const char *panelName) : Frame(NULL, panelName ? panelName : PANEL_CLASS), m_pViewPort(pViewPort)
{
	m_iTeam = 0;

	// initialize dialog
	SetTitle("", true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );
	// SetPaintTitleWhenTitleBarHidden(true);
	SetProportional(true);

	// info window about this class
	m_pPanel = new EditablePanel( this, "ClassInfo" );

	// Inheriting classes are responsible for calling LoadControlSettings()!
}

CClassMenu::~CClassMenu(void)
{
}

void CClassMenu::PerformLayout(void)
{
	int screenW, screenH;
	surface()->GetScreenSize(screenW, screenH);


	std::fprintf(stderr, "CLASS MENU screenW: %d\n", screenW);
	std::fprintf(stderr, "CLASS MENU screenH: %d\n", screenH);


	/// *** HARCODED OFFSETS*** MAKE THESE DYNAMIC BASED ON TEH VIDEO RESOLUTION.
	const int offsetX = 454; //(screenW - 640) / 2;
	const int offsetY = 0; // (screenH - 448) / 2;

	SetPos(offsetX, offsetY);

	BaseClass::PerformLayout();
}

MouseOverPanelButton *CClassMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

Panel *CClassMenu::CreateControlByName(const char *controlName)
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

void CClassMenu::Reset(void)
{
	for (int i = 0; i < GetChildCount(); ++i)
	{
		MouseOverPanelButton *pPanel = dynamic_cast<MouseOverPanelButton *>(GetChild(i));

		if (pPanel)
			pPanel->HidePage();
	}

	// Turn the first button back on again (so we have a default description shown)
	Assert( m_mouseoverButtons.Count() );
	for ( int i=0; i<m_mouseoverButtons.Count(); ++i )
	{
		if ( i == 0 )
		{
			m_mouseoverButtons[i]->ShowPage();	// Show the first page
		}
		else
		{
			m_mouseoverButtons[i]->HidePage();	// Hide the rest
		}
	}
}

void CClassMenu::OnCommand(const char *command)
{
	const bool isCancel = (Q_stricmp(command, "vguicancel") == 0);

	if (!isCancel)
	{
		gEngfuncs.pfnClientCmd(command);
		m_pViewPort->HideAllVGUIMenu();
	}
	else
	{
		Close();
	}

	m_pViewPort->ShowBackGround(false);

	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: shows the class menu
//-----------------------------------------------------------------------------
void CClassMenu::ShowPanel(bool bShow)
{

	if (BaseClass::IsVisible() == bShow) return;


	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );

		// load a default class page
		for ( int i=0; i<m_mouseoverButtons.Count(); ++i )
		{
			if ( i == 0 )
			{
				m_mouseoverButtons[i]->ShowPage();	// Show the first page
			}
			else
			{
				m_mouseoverButtons[i]->HidePage();	// Hide the rest
			}
		}

		//Temporary disbaled
		// if ( m_iScoreBoardKey == BUTTON_CODE_INVALID )
		// {
		// 	m_iScoreBoardKey = gameuifuncs->GetButtonCodeForBind( "showscores" );
		// }
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
	}

	m_pViewPort->ShowBackGround( bShow );
}

void CClassMenu::SetData(KeyValues *data)
{
	m_iTeam = data->GetInt("team");
}

void CClassMenu::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetText(text);
}

void CClassMenu::SetEnableButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetEnabled(state);
}

void CClassMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetVisible(state);
}

void CClassMenu::OnKeyCodePressed(KeyCode code)
{
	BaseClass::OnKeyCodePressed(code);
}
