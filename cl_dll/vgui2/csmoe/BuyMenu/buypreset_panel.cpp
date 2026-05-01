#include "buypreset_weaponsetlabel.h"
#include "shared_util.h"
#include <vgui/IBorder.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/controls.h>

using namespace vgui2;

BuyPresetEditPanel::BuyPresetEditPanel(Panel *parent, const char *panelName, const char *resourceFilename, int fallbackIndex, bool)
	: BaseClass(parent, panelName)
{
	SetProportional(parent ? parent->IsProportional() : true);
	m_fallbackIndex = fallbackIndex;
	m_baseWide = scheme()->GetProportionalScaledValueEx(GetScheme(), 220);
	m_baseTall = scheme()->GetProportionalScaledValueEx(GetScheme(), 160);
	SetSize(m_baseWide, m_baseTall);

	m_pBgPanel = new Panel(this, "mainBackground");
	m_pTitleLabel = new Label(this, "title", "");
	m_pCostLabel = new Label(this, "cost", "");
	m_pPrimaryWeapon = new WeaponLabel(this, "primary");
	m_pSecondaryWeapon = new WeaponLabel(this, "secondary");
	m_pHEGrenade = new EquipmentLabel(this, "hegrenade");
	m_pSmokeGrenade = new EquipmentLabel(this, "smokegrenade");
	m_pFlashbangs = new EquipmentLabel(this, "flashbang");
	m_pDefuser = new EquipmentLabel(this, "defuser");
	m_pNightvision = new EquipmentLabel(this, "nightvision");
	m_pArmor = new EquipmentLabel(this, "armor");

	if (resourceFilename && resourceFilename[0])
		LoadControlSettings(resourceFilename, "GAME");

	Reset();
}

BuyPresetEditPanel::~BuyPresetEditPanel()
{
}

void BuyPresetEditPanel::SetWeaponSet(const WeaponSet *pWeaponSet, bool)
{
	Reset();
	if (!pWeaponSet)
		return;

	char costText[64];
	Q_snprintf(costText, sizeof(costText), "$%d", pWeaponSet->FullCost());
	m_pCostLabel->SetText(costText);
	m_pPrimaryWeapon->SetWeapon(&pWeaponSet->m_primaryWeapon, true, true);
	m_pSecondaryWeapon->SetWeapon(&pWeaponSet->m_secondaryWeapon, false, true);

	if (pWeaponSet->m_HEGrenade)
		m_pHEGrenade->SetItem("gfx/vgui/hegrenade_square", 1);
	if (pWeaponSet->m_smokeGrenade)
		m_pSmokeGrenade->SetItem("gfx/vgui/smokegrenade_square", 1);
	if (pWeaponSet->m_flashbangs)
		m_pFlashbangs->SetItem("gfx/vgui/flashbang_square", pWeaponSet->m_flashbangs);
	if (pWeaponSet->m_defuser)
		m_pDefuser->SetItem("gfx/vgui/defuser", 1);
	if (pWeaponSet->m_nightvision)
		m_pNightvision->SetItem("gfx/vgui/nightvision", 1);
	if (pWeaponSet->m_armor)
		m_pArmor->SetItem(pWeaponSet->m_helmet ? "gfx/vgui/kevlar_helmet" : "gfx/vgui/kevlar", 1);
}

void BuyPresetEditPanel::SetText(const wchar_t *text)
{
	m_pTitleLabel->SetText(text ? text : L"");
}

void BuyPresetEditPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(Color(0, 0, 0, 0));
	m_pBgPanel->SetBgColor(pScheme->GetColor("Button.BgColor", Color(0, 0, 0, 128)));
	m_pBgPanel->SetBorder(pScheme->GetBorder("BuyPresetButtonBorder"));
}

void BuyPresetEditPanel::OnCommand(const char *command)
{
	if (command && Q_stricmp(command, "close"))
		PostActionSignal(new KeyValues("Command", "command", SharedVarArgs((char *)"%s %d", (char *)command, m_fallbackIndex)));

	BaseClass::OnCommand(command);
}

void BuyPresetEditPanel::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	InvalidateLayout();
}

void BuyPresetEditPanel::SetPanelBgColor(Color color)
{
	if (m_pBgPanel)
		m_pBgPanel->SetBgColor(color);
}

void BuyPresetEditPanel::Reset()
{
	m_pTitleLabel->SetText("#Cstrike_BuyPresetNewChoice");
	m_pCostLabel->SetText("");
	BuyPresetWeapon weapon;
	m_pPrimaryWeapon->SetWeapon(&weapon, true, false);
	m_pSecondaryWeapon->SetWeapon(&weapon, false, false);
	m_pHEGrenade->SetItem(NULL, 0);
	m_pSmokeGrenade->SetItem(NULL, 0);
	m_pFlashbangs->SetItem(NULL, 0);
	m_pDefuser->SetItem(NULL, 0);
	m_pNightvision->SetItem(NULL, 0);
	m_pArmor->SetItem(NULL, 0);
}
