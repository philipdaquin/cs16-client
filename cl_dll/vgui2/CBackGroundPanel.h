#ifndef GAME_CLIENT_UI_VGUI2_CBACKGROUNDPANEL_H
#define GAME_CLIENT_UI_VGUI2_CBACKGROUNDPANEL_H

#include <vgui_controls/Frame.h>
#include <vgui_controls/EditablePanel.h>

class CBackGroundPanel : public vgui2::Frame
{
public:
	typedef CBackGroundPanel ThisClass;
	typedef vgui2::Frame BaseClass;

public:
	CBackGroundPanel( vgui2::Panel* pParent );
	
	// don't respond to mouse clicks
	void OnMousePressed( vgui2::MouseCode code ) override {}

	vgui2::VPANEL IsWithinTraverse( int x, int y, bool traversePopups ) override { return NULL_HANDLE; }

	void ApplySchemeSettings( vgui2::IScheme* pScheme ) override;

	void PerformLayout() override;
};

void CreateBackground(vgui2::EditablePanel *pWindow);
void LayoutBackgroundPanel(vgui2::EditablePanel *pWindow);
void ApplyBackgroundSchemeSettings(vgui2::EditablePanel *pWindow, vgui2::IScheme *pScheme);
void ResizeWindowControls(vgui2::EditablePanel *pWindow, int tall, int wide, int offsetX, int offsetY);
int GetAlternateProportionalValueFromScaled(vgui2::HScheme scheme, int scaledValue);
void DrawRoundedBackground(Color bgColor, int x, int y, int wide, int tall);
void DrawRoundedBackground(Color bgColor, int wide, int tall);
void DrawRoundedBorder(Color borderColor, int wide, int tall);

#endif //GAME_CLIENT_UI_VGUI2_CBACKGROUNDPANEL_H
