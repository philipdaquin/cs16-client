#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/Frame.h>
#include "VGUI/cs_baseviewport.h"

class CTeamMenu;
class CClassMenu;
class CBuyMenu;
class CBuySubMenu;
class CBuyPresetPanel;
class CBuyPresetListBox;

class CCounterStrikeViewport : public CCSBaseViewport
{
	DECLARE_CLASS_SIMPLE(CCounterStrikeViewport, CCSBaseViewport);

public:
	CCounterStrikeViewport(vgui2::VPANEL parent);
	~CCounterStrikeViewport();

	enum BuyMenuCategory_t
	{
		PISTOL = 0,
		SHOTGUN,
		SUBMACHINEGUN,
		RIFLE,
		MACHINEGUN,
		ITEM,
		CATEGORY_COUNT = 6
	};

	enum
	{
		SUBMENU_TER = 0,
		SUBMENU_CT = 1,
		SUBMENU_COUNT = 2
	};

	void CreatePanels();
	void DestroyPanels();

	void ShowTeamMenu();
	void ShowClassMenu(int menuType);
	void ShowBuyMenu();
	void ShowBuySubMenu(BuyMenuCategory_t category);
	void HideAllGameMenus();
	void ShowBackGround(bool bShow);
	void ReloadScheme(const char *fromFile = "resource/ClientScheme.res");

	virtual void ApplySchemeSettings(vgui2::IScheme *scheme) override;
	virtual void PerformLayout() override;
	virtual void Paint() override;

	int GetSubMenuIndex(BuyMenuCategory_t category, bool isCT) const
	{
		return category * SUBMENU_COUNT + (isCT ? SUBMENU_CT : SUBMENU_TER);
	}

private:
	CTeamMenu *m_pTeamMenu;
	CClassMenu *m_pClassMenu;
	CBuyMenu *m_pBuyMenu;
	CBuySubMenu *m_apBuySubMenus[CATEGORY_COUNT * SUBMENU_COUNT];
	CBuyPresetPanel *m_pBuyPresetPanel;
	CBuyPresetListBox *m_pBuyPresetListBox;
	vgui2::Frame *m_pBackGround;

	bool m_bPanelsCreated;
};

#endif
