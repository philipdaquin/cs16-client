#pragma once

#include <KeyValues.h>
#include <tier1/utlvector.h>
#include <vgui_controls/EditablePanel.h>

class MouseOverPanelButton;
class CBuyMenu;

class CBuySubMenu : public vgui2::EditablePanel
{
	DECLARE_CLASS_SIMPLE(CBuySubMenu, vgui2::EditablePanel);

public:
	CBuySubMenu(vgui2::Panel *parent, const char *panelName, CBuyMenu *pOwner);
	virtual ~CBuySubMenu();

	bool LoadFromResource(const char *resourceFile);
	void UpdateAffordability(int money);
	const char *GetResourceFile() const;
	bool HasCostedButtons() const;

	virtual void OnCommand(const char *command);

private:
	void CollectButtons(vgui2::Panel *panel);

	CBuyMenu *m_pOwner;
	CUtlVector<MouseOverPanelButton *> m_Buttons;
	char m_szResourceFile[260];
};

