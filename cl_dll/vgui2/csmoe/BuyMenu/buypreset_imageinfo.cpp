#include "buypreset_weaponsetlabel.h"
#include "shared_util.h"
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/controls.h>

using namespace vgui2;

static int MinInt(int a, int b)
{
	return (a < b) ? a : b;
}

static float MinFloat(float a, float b)
{
	return (a < b) ? a : b;
}

BuyPresetImage::BuyPresetImage(IImage *realImage)
{
	m_image = realImage;
	if (m_image)
		m_image->GetSize(m_wide, m_tall);
	else
		m_wide = m_tall = 0;
}

void BuyPresetImage::Paint()
{
	if (m_image)
		m_image->Paint();
}

void BuyPresetImage::SetPos(int x, int y)
{
	if (m_image)
		m_image->SetPos(x, y);
}

void BuyPresetImage::GetContentSize(int &wide, int &tall)
{
	GetSize(wide, tall);
}

void BuyPresetImage::GetSize(int &wide, int &tall)
{
	wide = m_wide;
	tall = m_tall;
}

void BuyPresetImage::SetSize(int wide, int tall)
{
	m_wide = wide;
	m_tall = tall;
	if (m_image)
		m_image->SetSize(wide, tall);
}

void BuyPresetImage::SetColor(Color col)
{
	if (m_image)
		m_image->SetColor(col);
}

void ImageInfo::FitInBounds(int baseX, int baseY, int width, int height, bool center, int, bool halfHeight)
{
	x = baseX;
	y = baseY;
	w = width;
	h = halfHeight ? height / 2 : height;
	fullW = width;
	fullH = height;

	if (!image)
		return;

	int imageWide = 0;
	int imageTall = 0;
	image->GetContentSize(imageWide, imageTall);
	if (imageWide > 0 && imageTall > 0)
	{
		const float scale = MinFloat(width / (float)imageWide, h / (float)imageTall);
		w = (int)(imageWide * scale);
		h = (int)(imageTall * scale);
	}

	if (center)
	{
		x = baseX + (width - w) / 2;
		y = baseY + (height - h) / 2;
	}

	image->SetPos(x, y);
	image->SetSize(w, h);
}

void ImageInfo::Paint()
{
	if (image)
		image->Paint();
}

WeaponImageInfo::WeaponImageInfo()
{
	m_left = m_top = m_wide = m_tall = 0;
	m_isPrimary = false;
	m_weaponScale = m_ammoScale = 0;
	m_needLayout = false;
	m_isCentered = true;
	memset(&m_weapon, 0, sizeof(m_weapon));
	memset(&m_ammo, 0, sizeof(m_ammo));
	m_pAmmoText = new TextImage("");
}

WeaponImageInfo::~WeaponImageInfo()
{
	delete m_pAmmoText;
}

void WeaponImageInfo::ApplyTextSettings(IScheme *pScheme, bool isProportional)
{
	m_pAmmoText->SetColor(pScheme->GetColor("Label.TextColor", Color(255, 255, 255, 255)));
	m_pAmmoText->SetFont(pScheme->GetFont("Default", isProportional));
	m_pAmmoText->SetWrap(false);
}

void WeaponImageInfo::SetBounds(int left, int top, int wide, int tall)
{
	m_left = left;
	m_top = top;
	m_wide = wide;
	m_tall = tall;
	m_needLayout = true;
}

void WeaponImageInfo::SetCentered(bool isCentered)
{
	m_isCentered = isCentered;
	m_needLayout = true;
}

void WeaponImageInfo::SetScaleAt1024(int weaponScale, int ammoScale)
{
	m_weaponScale = weaponScale;
	m_ammoScale = ammoScale;
	m_needLayout = true;
}

void WeaponImageInfo::SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary, bool)
{
	m_isPrimary = isPrimary;
	m_weapon.image = NULL;
	m_ammo.image = NULL;
	m_pAmmoText->SetText(L"");

	if (pWeapon && pWeapon->GetWeaponID() != WEAPON_NONE)
	{
		m_weapon.image = scheme()->GetImage(ImageFnameFromWeaponID(pWeapon->GetWeaponID(), isPrimary), true);
		if (pWeapon->GetAmmoAmount() > 1)
		{
			wchar_t text[32];
			swprintf(text, sizeof(text) / sizeof(text[0]), L"x%d", pWeapon->GetAmmoAmount());
			m_pAmmoText->SetText(text);
		}
	}

	m_needLayout = true;
}

void WeaponImageInfo::PerformLayout()
{
	m_needLayout = false;
	m_weapon.FitInBounds(m_left, m_top, m_wide, m_tall, m_isCentered, m_weaponScale);
	m_ammo.FitInBounds(m_left + m_wide * 3 / 4, m_top + m_tall / 2, m_wide / 4, m_tall / 2, true, m_ammoScale);
	int textWide, textTall;
	m_pAmmoText->ResizeImageToContent();
	m_pAmmoText->GetSize(textWide, textTall);
	m_pAmmoText->SetPos(m_left + m_wide - textWide, m_top + m_tall - textTall);
}

void WeaponImageInfo::Paint()
{
	if (m_needLayout)
		PerformLayout();
	m_weapon.Paint();
	m_ammo.Paint();
}

void WeaponImageInfo::PaintText()
{
	if (m_needLayout)
		PerformLayout();
	m_pAmmoText->Paint();
}

ItemImageInfo::ItemImageInfo()
{
	m_left = m_top = m_wide = m_tall = 0;
	m_count = 0;
	m_needLayout = false;
	memset(&m_image, 0, sizeof(m_image));
	m_pText = new TextImage("");
}

ItemImageInfo::~ItemImageInfo()
{
	delete m_pText;
}

void ItemImageInfo::ApplyTextSettings(IScheme *pScheme, bool isProportional)
{
	m_pText->SetColor(pScheme->GetColor("Label.TextColor", Color(255, 255, 255, 255)));
	m_pText->SetFont(pScheme->GetFont("Default", isProportional));
	m_pText->SetWrap(false);
}

void ItemImageInfo::SetBounds(int left, int top, int wide, int tall)
{
	m_left = left;
	m_top = top;
	m_wide = wide;
	m_tall = tall;
	m_needLayout = true;
}

void ItemImageInfo::SetItem(const char *imageFname, int count)
{
	m_count = imageFname ? count : 0;
	m_image.image = imageFname ? scheme()->GetImage(imageFname, true) : NULL;
	m_pText->SetText(L"");
	if (m_count > 1)
	{
		wchar_t text[32];
		swprintf(text, sizeof(text) / sizeof(text[0]), L"x%d", m_count);
		m_pText->SetText(text);
	}
	m_needLayout = true;
}

void ItemImageInfo::PerformLayout()
{
	m_needLayout = false;
	m_image.FitInBounds(m_left, m_top, m_wide, m_tall, true, 0);
	int textWide, textTall;
	m_pText->ResizeImageToContent();
	m_pText->GetSize(textWide, textTall);
	m_pText->SetPos(m_left + m_wide - textWide, m_top + m_tall - textTall);
}

void ItemImageInfo::Paint()
{
	if (m_needLayout)
		PerformLayout();
	if (m_count)
		m_image.Paint();
}

void ItemImageInfo::PaintText()
{
	if (m_needLayout)
		PerformLayout();
	m_pText->Paint();
}

WeaponLabel::WeaponLabel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
	SetMouseInputEnabled(false);
}

WeaponLabel::~WeaponLabel()
{
}

void WeaponLabel::SetWeapon(const BuyPresetWeapon *pWeapon, bool isPrimary, bool showAmmo)
{
	BuyPresetWeapon weapon(WEAPON_NONE);
	if (pWeapon)
		weapon = *pWeapon;
	if (!showAmmo)
		weapon.SetAmmoAmount(0);
	m_weapon.SetWeapon(&weapon, isPrimary, false);
}

void WeaponLabel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_weapon.ApplyTextSettings(pScheme, IsProportional());
}

void WeaponLabel::PerformLayout()
{
	BaseClass::PerformLayout();
	m_weapon.SetBounds(0, 0, GetWide(), GetTall());
}

void WeaponLabel::Paint()
{
	BaseClass::Paint();
	m_weapon.Paint();
	m_weapon.PaintText();
}

EquipmentLabel::EquipmentLabel(Panel *parent, const char *panelName, const char *imageFname) : BaseClass(parent, panelName)
{
	SetMouseInputEnabled(false);
	m_item.SetItem(imageFname, imageFname ? 1 : 0);
}

EquipmentLabel::~EquipmentLabel()
{
}

void EquipmentLabel::SetItem(const char *imageFname, int count)
{
	m_item.SetItem(imageFname, count);
}

void EquipmentLabel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_item.ApplyTextSettings(pScheme, IsProportional());
}

void EquipmentLabel::PerformLayout()
{
	BaseClass::PerformLayout();
	m_item.SetBounds(0, 0, GetWide(), GetTall());
}

void EquipmentLabel::Paint()
{
	BaseClass::Paint();
	m_item.Paint();
	m_item.PaintText();
}

void DrawDashedLine(int x0, int y0, int x1, int y1, int dashLen, int gapLen)
{
	surface()->DrawSetColor(Color(255, 255, 255, 255));
	if (x0 == x1)
	{
		for (int y = y0; y < y1; y += dashLen + gapLen)
			surface()->DrawFilledRect(x0, y, x1 + 1, MinInt(y + dashLen, y1));
	}
	else
	{
		for (int x = x0; x < x1; x += dashLen + gapLen)
			surface()->DrawFilledRect(x, y0, MinInt(x + dashLen, x1), y1 + 1);
	}
}
