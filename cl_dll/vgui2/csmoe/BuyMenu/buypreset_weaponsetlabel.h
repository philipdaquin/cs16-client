#ifndef BUYPRESET_WEAPONSETLABEL_H
#define BUYPRESET_WEAPONSETLABEL_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IImage.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Panel.h>
#include "buy_presets.h"

void DrawDashedLine(int x0, int y0, int x1, int y1, int dashLen, int gapLen);

class BuyPresetImage : public vgui2::IImage
{
public:
	BuyPresetImage(vgui2::IImage *realImage);

	void Paint() override;
	void SetPos(int x, int y) override;
	void GetContentSize(int &wide, int &tall) override;
	void GetSize(int &wide, int &tall) override;
	void SetSize(int wide, int tall) override;
	void SetColor(Color col) override;

private:
	vgui2::IImage *m_image;
	int m_wide;
	int m_tall;
};

struct ImageInfo
{
	vgui2::IImage *image;
	int w;
	int h;
	int x;
	int y;
	int fullW;
	int fullH;

	void FitInBounds(int baseX, int baseY, int width, int height, bool center, int scaleAt1024, bool halfHeight = false);
	void Paint();
};

class WeaponImageInfo
{
public:
	WeaponImageInfo();
	~WeaponImageInfo();

	void SetBounds(int left, int top, int wide, int tall);
	void SetCentered(bool isCentered);
	void SetScaleAt1024(int weaponScale, int ammoScale);
	void SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary, bool useCurrentAmmoType);
	void ApplyTextSettings(vgui2::IScheme *pScheme, bool isProportional);
	void Paint();
	void PaintText();

private:
	void PerformLayout();

	int m_left;
	int m_top;
	int m_wide;
	int m_tall;
	bool m_isPrimary;
	int m_weaponScale;
	int m_ammoScale;
	bool m_needLayout;
	bool m_isCentered;
	ImageInfo m_weapon;
	ImageInfo m_ammo;
	vgui2::TextImage *m_pAmmoText;
};

class ItemImageInfo
{
public:
	ItemImageInfo();
	~ItemImageInfo();

	void SetBounds(int left, int top, int wide, int tall);
	void SetItem(const char *imageFname, int count);
	void ApplyTextSettings(vgui2::IScheme *pScheme, bool isProportional);
	void Paint();
	void PaintText();

private:
	void PerformLayout();

	int m_left;
	int m_top;
	int m_wide;
	int m_tall;
	int m_count;
	bool m_needLayout;
	ImageInfo m_image;
	vgui2::TextImage *m_pText;
};

class WeaponLabel : public vgui2::Panel
{
	typedef vgui2::Panel BaseClass;

public:
	WeaponLabel(vgui2::Panel *parent, const char *panelName);
	~WeaponLabel();

	void SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary, bool showAmmo = false);
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	void PerformLayout() override;
	void Paint() override;

private:
	WeaponImageInfo m_weapon;
};

class EquipmentLabel : public vgui2::Panel
{
	typedef vgui2::Panel BaseClass;

public:
	EquipmentLabel(vgui2::Panel *parent, const char *panelName, const char *imageFname = NULL);
	~EquipmentLabel();

	void SetItem(const char *imageFname, int count);
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	void PerformLayout() override;
	void Paint() override;

private:
	ItemImageInfo m_item;
};

class BuyPresetEditPanel : public vgui2::EditablePanel
{
	typedef vgui2::EditablePanel BaseClass;

public:
	BuyPresetEditPanel(vgui2::Panel *parent, const char *panelName, const char *resourceFilename, int fallbackIndex, bool editableName);
	~BuyPresetEditPanel();

	void SetWeaponSet(const WeaponSet *pWeaponSet, bool current);
	void SetText(const wchar_t *text);
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	void OnCommand(const char *command) override;
	void OnSizeChanged(int wide, int tall) override;
	void SetPanelBgColor(Color color);

private:
	void Reset();

	vgui2::Panel *m_pBgPanel;
	vgui2::Label *m_pTitleLabel;
	vgui2::Label *m_pCostLabel;
	WeaponLabel *m_pPrimaryWeapon;
	WeaponLabel *m_pSecondaryWeapon;
	EquipmentLabel *m_pHEGrenade;
	EquipmentLabel *m_pSmokeGrenade;
	EquipmentLabel *m_pFlashbangs;
	EquipmentLabel *m_pDefuser;
	EquipmentLabel *m_pNightvision;
	EquipmentLabel *m_pArmor;
	int m_baseWide;
	int m_baseTall;
	int m_fallbackIndex;
};

#endif
