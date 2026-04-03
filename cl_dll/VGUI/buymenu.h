#pragma once

#include "VGUI/counterstrikeviewport.h"
#include <vgui_controls/EditablePanel.h>

class CBuySubMenu;

class CBuyMenu : public vgui2::EditablePanel, public IViewportPanel
{
	DECLARE_CLASS_SIMPLE(CBuyMenu, vgui2::EditablePanel);

public:
	CBuyMenu(vgui2::Panel *parent, const char *panelName);
	virtual ~CBuyMenu();

	virtual const char *GetName();
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate();
	virtual bool HasInputElements();
	virtual void ShowPanel(bool state);
	virtual vgui2::VPANEL GetVPanel();
	virtual bool IsVisible();
	virtual void SetParent(vgui2::VPANEL parent);

	virtual void OnCommand(const char *command);
	virtual void OnKeyCodePressed(vgui2::KeyCode code);
	virtual void PerformLayout();

	void SetActiveSubMenu(int menuType);
	void ShowRequestedSubMenu();
	void CloseSubMenu();
	int GetCurrentMoney() const;

private:
	const char *GetTeamSuffix() const;
	const char *GetMenuTypeResource(int menuType) const;
	const char *ResolveCommandResource(const char *command) const;
	CBuySubMenu *GetOrCreatePage(const char *resourceFile);
	void ShowPage(CBuySubMenu *page);
	void UpdateMoneyLabel();
	void UpdateAffordability();
	void CenterInViewport();

	CUtlVector<CBuySubMenu *> m_Pages;
	CBuySubMenu *m_pMainPage;
	CBuySubMenu *m_pCurrentPage;
	vgui2::Panel *m_pPageHost;
	vgui2::Label *m_pMoneyLabel;
	int m_iRequestedMenuType;
	int m_iLastKnownMoney;
};

