#include "buy_presets.h"
#include "hud.h"
#include "cl_util.h"
#include "shared_util.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/controls.h>

BuyPresetManager *TheBuyPresets = NULL;

namespace
{
struct WeaponPresetInfo
{
	CSWeaponID id;
	const char *alias;
	const char *displayName;
	const char *image;
	int cost;
	int teamMask;
	bool primary;
};

constexpr int TEAM_MASK_T = BIT(0);
constexpr int TEAM_MASK_CT = BIT(1);
constexpr int TEAM_MASK_BOTH = TEAM_MASK_T | TEAM_MASK_CT;

WeaponPresetInfo g_weaponPresetInfo[] =
{
	{ WEAPON_P228, "p228", "#Cstrike_TitlesTXT_P228", "gfx/vgui/p228", P228_PRICE, TEAM_MASK_BOTH, false },
	{ WEAPON_GLOCK18, "glock", "#Cstrike_TitlesTXT_Glock18", "gfx/vgui/glock18", GLOCK18_PRICE, TEAM_MASK_T, false },
	{ WEAPON_USP, "usp", "#Cstrike_TitlesTXT_USP45", "gfx/vgui/usp45", USP_PRICE, TEAM_MASK_CT, false },
	{ WEAPON_DEAGLE, "deagle", "#Cstrike_TitlesTXT_DesertEagle", "gfx/vgui/deserteagle", DEAGLE_PRICE, TEAM_MASK_BOTH, false },
	{ WEAPON_ELITE, "elites", "#Cstrike_TitlesTXT_Beretta96G", "gfx/vgui/elites", ELITE_PRICE, TEAM_MASK_T, false },
	{ WEAPON_FIVESEVEN, "fiveseven", "#Cstrike_TitlesTXT_ESFiveSeven", "gfx/vgui/fiveseven", FIVESEVEN_PRICE, TEAM_MASK_CT, false },
	{ WEAPON_M3, "m3", "#Cstrike_TitlesTXT_Leone12", "gfx/vgui/m3", M3_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_XM1014, "xm1014", "#Cstrike_TitlesTXT_AutoShotgun", "gfx/vgui/xm1014", XM1014_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_MAC10, "mac10", "#Cstrike_TitlesTXT_Mac10_Short", "gfx/vgui/mac10", MAC10_PRICE, TEAM_MASK_T, true },
	{ WEAPON_TMP, "tmp", "#Cstrike_TitlesTXT_tmp", "gfx/vgui/tmp", TMP_PRICE, TEAM_MASK_CT, true },
	{ WEAPON_MP5N, "mp5", "#Cstrike_TitlesTXT_mp5navy", "gfx/vgui/mp5", MP5NAVY_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_UMP45, "ump45", "#Cstrike_TitlesTXT_KMUMP45", "gfx/vgui/ump45", UMP45_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_P90, "p90", "#Cstrike_TitlesTXT_ESC90", "gfx/vgui/p90", P90_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_GALIL, "galil", "#Cstrike_TitlesTXT_Galil", "gfx/vgui/galil", GALIL_PRICE, TEAM_MASK_T, true },
	{ WEAPON_FAMAS, "famas", "#Cstrike_TitlesTXT_Famas", "gfx/vgui/famas", FAMAS_PRICE, TEAM_MASK_CT, true },
	{ WEAPON_AK47, "ak47", "#Cstrike_TitlesTXT_AK47", "gfx/vgui/ak47", AK47_PRICE, TEAM_MASK_T, true },
	{ WEAPON_M4A1, "m4a1", "#Cstrike_TitlesTXT_M4A1_Short", "gfx/vgui/m4a1", M4A1_PRICE, TEAM_MASK_CT, true },
	{ WEAPON_SG552, "sg552", "#Cstrike_TitlesTXT_SG552", "gfx/vgui/sg552", SG552_PRICE, TEAM_MASK_T, true },
	{ WEAPON_AUG, "aug", "#Cstrike_TitlesTXT_Aug", "gfx/vgui/aug", AUG_PRICE, TEAM_MASK_CT, true },
	{ WEAPON_SCOUT, "scout", "#Cstrike_TitlesTXT_Scout", "gfx/vgui/scout", SCOUT_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_AWP, "awp", "#Cstrike_TitlesTXT_ArcticWarfareMagnum", "gfx/vgui/awp", AWP_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_G3SG1, "g3sg1", "#Cstrike_TitlesTXT_G3SG1", "gfx/vgui/g3sg1", G3SG1_PRICE, TEAM_MASK_T, true },
	{ WEAPON_SG550, "sg550", "#Cstrike_TitlesTXT_SG550", "gfx/vgui/sg550", SG550_PRICE, TEAM_MASK_CT, true },
	{ WEAPON_M249, "m249", "#Cstrike_TitlesTXT_ESM249", "gfx/vgui/m249", M249_PRICE, TEAM_MASK_BOTH, true },
	{ WEAPON_NONE, "", "#Cstrike_CurrentWeapon", "gfx/vgui/640_weaponblank", 0, TEAM_MASK_BOTH, false },
};

int LocalTeam()
{
	return (cl::g_iTeamNumber == TEAM_CT) ? TEAM_CT : TEAM_TERRORIST;
}

int TeamMaskForTeam(int team)
{
	return (team == TEAM_CT) ? TEAM_MASK_CT : TEAM_MASK_T;
}

const WeaponPresetInfo *FindWeaponInfo(CSWeaponID weaponID)
{
	for (size_t i = 0; i < sizeof(g_weaponPresetInfo) / sizeof(g_weaponPresetInfo[0]); ++i)
	{
		if (g_weaponPresetInfo[i].id == weaponID)
			return &g_weaponPresetInfo[i];
	}

	return NULL;
}

CSWeaponID WeaponIDFromAlias(const char *alias)
{
	if (!alias || !alias[0])
		return WEAPON_NONE;

	for (size_t i = 0; i < sizeof(g_weaponPresetInfo) / sizeof(g_weaponPresetInfo[0]); ++i)
	{
		if (!Q_stricmp(g_weaponPresetInfo[i].alias, alias))
			return g_weaponPresetInfo[i].id;
	}

	return WEAPON_NONE;
}

void AddDefaultPreset(BuyPresetList& presets, const wchar_t *name, CSWeaponID primary, CSWeaponID secondary, bool armor, bool helmet, bool he, int flash)
{
	WeaponSet set;
	set.m_primaryWeapon.SetWeaponID(primary);
	set.m_secondaryWeapon.SetWeaponID(secondary);
	set.m_armor = armor ? 100 : 0;
	set.m_helmet = helmet;
	set.m_HEGrenade = he;
	set.m_flashbangs = flash;

	BuyPreset preset;
	preset.SetName(name);
	preset.ReplaceSet(0, set);
	presets.AddToTail(preset);
}

void BuildDefaultPresets(BuyPresetList& presets, int team)
{
	presets.RemoveAll();
	if (team == TEAM_CT)
	{
		AddDefaultPreset(presets, L"Rifle", WEAPON_M4A1, WEAPON_USP, true, true, true, 1);
		AddDefaultPreset(presets, L"Scoped Rifle", WEAPON_AUG, WEAPON_USP, true, true, false, 1);
		AddDefaultPreset(presets, L"Sniper", WEAPON_AWP, WEAPON_DEAGLE, true, true, false, 1);
		AddDefaultPreset(presets, L"SMG", WEAPON_MP5N, WEAPON_USP, true, true, true, 1);
	}
	else
	{
		AddDefaultPreset(presets, L"Rifle", WEAPON_AK47, WEAPON_GLOCK18, true, true, true, 1);
		AddDefaultPreset(presets, L"Scoped Rifle", WEAPON_SG552, WEAPON_GLOCK18, true, true, false, 1);
		AddDefaultPreset(presets, L"Sniper", WEAPON_AWP, WEAPON_DEAGLE, true, true, false, 1);
		AddDefaultPreset(presets, L"SMG", WEAPON_MP5N, WEAPON_GLOCK18, true, true, true, 1);
	}
}

int CommandIndex()
{
	if (gEngfuncs.Cmd_Argc() != 2)
		return -1;

	return Q_atoi(gEngfuncs.Cmd_Argv(1)) - 1;
}

void CmdBuyFavorite()
{
	if (!TheBuyPresets)
		TheBuyPresets = new BuyPresetManager();

	const int presetIndex = CommandIndex();
	if (presetIndex < 0 || presetIndex >= TheBuyPresets->GetNumPresets())
	{
		gEngfuncs.Con_Printf("usage: cl_buy_favorite <1...%d>\n", TheBuyPresets->GetNumPresets());
		return;
	}

	TheBuyPresets->PurchasePreset(presetIndex);
}

void CmdBuyFavoriteSet()
{
	if (!TheBuyPresets)
		TheBuyPresets = new BuyPresetManager();

	const int presetIndex = CommandIndex();
	if (presetIndex < 0 || presetIndex >= TheBuyPresets->GetNumPresets())
	{
		gEngfuncs.Con_Printf("usage: cl_buy_favorite_set <1...%d>\n", TheBuyPresets->GetNumPresets());
		return;
	}

	WeaponSet current;
	TheBuyPresets->GetCurrentLoadout(&current);

	const BuyPreset *preset = TheBuyPresets->GetPreset(presetIndex);
	if (!preset)
		return;

	BuyPreset updated(*preset);
	updated.ReplaceSet(0, current);
	TheBuyPresets->SetPreset(presetIndex, &updated);
	TheBuyPresets->Save();
}

void CmdBuyFavoriteReset()
{
	if (!TheBuyPresets)
		TheBuyPresets = new BuyPresetManager();

	TheBuyPresets->ResetEditToDefaults();
	TheBuyPresets->SetPresets(TheBuyPresets->GetEditPresets());
	TheBuyPresets->Save();
}
}

void RegisterBuyPresetCommands(void)
{
	static bool registered = false;
	if (registered)
		return;

	registered = true;
	gEngfuncs.pfnAddCommand("cl_buy_favorite", CmdBuyFavorite);
	gEngfuncs.pfnAddCommand("cl_buy_favorite_set", CmdBuyFavoriteSet);
	gEngfuncs.pfnAddCommand("cl_buy_favorite_reset", CmdBuyFavoriteReset);
}

BuyPresetWeapon::BuyPresetWeapon()
{
	m_name = L"";
	m_weaponID = WEAPON_NONE;
	m_ammoType = AMMO_CLIPS;
	m_ammoAmount = 0;
	m_fillAmmo = true;
}

BuyPresetWeapon::BuyPresetWeapon(CSWeaponID weaponID)
{
	m_name = L"";
	m_weaponID = WEAPON_NONE;
	m_ammoType = AMMO_CLIPS;
	m_ammoAmount = (weaponID == WEAPON_NONE) ? 0 : 1;
	m_fillAmmo = true;
	SetWeaponID(weaponID);
}

BuyPresetWeapon& BuyPresetWeapon::operator=(const BuyPresetWeapon& other)
{
	m_name = other.m_name;
	m_weaponID = other.m_weaponID;
	m_ammoType = other.m_ammoType;
	m_ammoAmount = other.m_ammoAmount;
	m_fillAmmo = other.m_fillAmmo;
	return *this;
}

void BuyPresetWeapon::SetWeaponID(CSWeaponID weaponID)
{
	m_weaponID = weaponID;
	const WeaponPresetInfo *info = FindWeaponInfo(weaponID);
	m_name = info ? info->displayName ? vgui2::localize()->Find(info->displayName) : L"" : L"";
	if (!m_name)
		m_name = L"";
}

WeaponSet::WeaponSet()
{
	Reset();
}

void WeaponSet::Reset(void)
{
	BuyPresetWeapon blank;
	m_primaryWeapon = blank;
	m_secondaryWeapon = blank;
	m_armor = 0;
	m_helmet = false;
	m_smokeGrenade = false;
	m_HEGrenade = false;
	m_flashbangs = 0;
	m_defuser = false;
	m_nightvision = false;
}

void WeaponSet::GetCurrent(int& cost, WeaponSet& ws) const
{
	GetFromScratch(cost, ws);
	if (cost > cl::gHUD.m_Money.m_iMoneyCount)
	{
		cost = -1;
		ws.Reset();
	}
}

void WeaponSet::GetFromScratch(int& cost, WeaponSet& ws) const
{
	cost = FullCost();
	ws = *this;
}

int WeaponSet::FullCost() const
{
	int cost = 0;
	cost += BuyPresetWeaponCost(m_primaryWeapon.GetWeaponID());
	cost += BuyPresetWeaponCost(m_secondaryWeapon.GetWeaponID());
	if (m_armor)
		cost += m_helmet ? ASSAULTSUIT_PRICE : KEVLAR_PRICE;
	if (m_smokeGrenade)
		cost += SMOKEGRENADE_PRICE;
	if (m_HEGrenade)
		cost += HEGRENADE_PRICE;
	cost += m_flashbangs * FLASHBANG_PRICE;
	if (m_defuser)
		cost += DEFUSEKIT_PRICE;
	if (m_nightvision)
		cost += NVG_PRICE;
	return cost;
}

void WeaponSet::GenerateBuyCommands(char command[BUY_PRESET_COMMAND_LEN]) const
{
	command[0] = 0;
	char *tmp = command;
	int remainder = BUY_PRESET_COMMAND_LEN;

	if (m_primaryWeapon.GetWeaponID() != WEAPON_NONE)
		tmp = BufPrintf(tmp, remainder, "%s\n", BuyPresetWeaponAlias(m_primaryWeapon.GetWeaponID()));
	if (m_secondaryWeapon.GetWeaponID() != WEAPON_NONE)
		tmp = BufPrintf(tmp, remainder, "%s\n", BuyPresetWeaponAlias(m_secondaryWeapon.GetWeaponID()));
	if (m_armor)
		tmp = BufPrintf(tmp, remainder, "%s\n", m_helmet ? "vesthelm" : "vest");
	if (m_smokeGrenade)
		tmp = BufPrintf(tmp, remainder, "sgren\n");
	if (m_HEGrenade)
		tmp = BufPrintf(tmp, remainder, "hegren\n");
	for (int i = 0; i < m_flashbangs; ++i)
		tmp = BufPrintf(tmp, remainder, "flash\n");
	if (m_defuser)
		tmp = BufPrintf(tmp, remainder, "defuser\n");
	if (m_nightvision)
		BufPrintf(tmp, remainder, "nvgs\n");
}

BuyPreset::BuyPreset()
{
	SetName(L"");
}

BuyPreset::BuyPreset(const BuyPreset& other)
{
	wcsncpy(m_name, other.m_name, MaxBuyPresetName);
	m_name[MaxBuyPresetName - 1] = 0;
	m_weaponList = other.m_weaponList;
}

BuyPreset::~BuyPreset()
{
}

void BuyPreset::SetName(const wchar_t *name)
{
	if (!name || !name[0])
		name = L"Loadout";

	wcsncpy(m_name, name, MaxBuyPresetName);
	m_name[MaxBuyPresetName - 1] = 0;
}

void BuyPreset::Parse(KeyValues *data)
{
	if (!data)
		return;

	SetName(data->GetWString("PresetName", L"Loadout"));
	WeaponSet set;
	set.m_primaryWeapon.SetWeaponID(WeaponIDFromAlias(data->GetString("Primary", "")));
	set.m_secondaryWeapon.SetWeaponID(WeaponIDFromAlias(data->GetString("Secondary", "")));
	set.m_armor = data->GetInt("Armor", 0);
	set.m_helmet = data->GetInt("Helmet", 0) != 0;
	set.m_HEGrenade = data->GetInt("HEGrenade", 0) != 0;
	set.m_smokeGrenade = data->GetInt("SmokeGrenade", 0) != 0;
	set.m_flashbangs = data->GetInt("Flashbangs", 0);
	set.m_defuser = data->GetInt("Defuser", 0) != 0;
	set.m_nightvision = data->GetInt("NightVision", 0) != 0;
	ReplaceSet(0, set);
}

void BuyPreset::Save(KeyValues *data)
{
	if (!data)
		return;

	KeyValues *presetKey = data->CreateNewKey();
	presetKey->SetName("Preset");
	presetKey->SetWString("PresetName", m_name);
	const WeaponSet *set = GetSet(0);
	if (!set)
		return;

	presetKey->SetString("Primary", BuyPresetWeaponAlias(set->m_primaryWeapon.GetWeaponID()));
	presetKey->SetString("Secondary", BuyPresetWeaponAlias(set->m_secondaryWeapon.GetWeaponID()));
	presetKey->SetInt("Armor", set->m_armor);
	presetKey->SetInt("Helmet", set->m_helmet ? 1 : 0);
	presetKey->SetInt("HEGrenade", set->m_HEGrenade ? 1 : 0);
	presetKey->SetInt("SmokeGrenade", set->m_smokeGrenade ? 1 : 0);
	presetKey->SetInt("Flashbangs", set->m_flashbangs);
	presetKey->SetInt("Defuser", set->m_defuser ? 1 : 0);
	presetKey->SetInt("NightVision", set->m_nightvision ? 1 : 0);
}

int BuyPreset::FullCost() const
{
	const WeaponSet *set = GetSet(0);
	return set ? set->FullCost() : 0;
}

const WeaponSet *BuyPreset::GetSet(int index) const
{
	if (index < 0 || index >= m_weaponList.Count())
		return NULL;
	return &m_weaponList[index];
}

void BuyPreset::DeleteSet(int index)
{
	if (index >= 0 && index < m_weaponList.Count())
		m_weaponList.Remove(index);
}

void BuyPreset::SwapSet(int firstIndex, int secondIndex)
{
	if (firstIndex < 0 || secondIndex < 0 || firstIndex >= m_weaponList.Count() || secondIndex >= m_weaponList.Count())
		return;

	WeaponSet temp = m_weaponList[firstIndex];
	m_weaponList[firstIndex] = m_weaponList[secondIndex];
	m_weaponList[secondIndex] = temp;
}

void BuyPreset::ReplaceSet(int index, const WeaponSet& weaponSet)
{
	while (m_weaponList.Count() <= index)
		m_weaponList.AddToTail(WeaponSet());
	m_weaponList[index] = weaponSet;
}

BuyPresetManager::BuyPresetManager()
{
	m_loadedTeam = TEAM_UNASSIGNED;
}

void BuyPresetManager::VerifyLoadedTeam(void)
{
	const int team = LocalTeam();
	if (team == m_loadedTeam)
		return;

	m_loadedTeam = team;
	BuildDefaultPresets(m_presets, team);
	m_editPresets = m_presets;
}

void BuyPresetManager::Save()
{
	// Source persists to cfg/BuyPresets_*.vdf. Keep this code-only for now:
	// the current task explicitly avoids adding or editing resource/data files.
}

void BuyPresetManager::PurchasePreset(int presetIndex)
{
	VerifyLoadedTeam();
	if (presetIndex < 0 || presetIndex >= m_presets.Count())
		return;

	const BuyPreset *preset = &m_presets[presetIndex];
	for (int setIndex = 0; setIndex < preset->GetNumSets(); ++setIndex)
	{
		const WeaponSet *itemSet = preset->GetSet(setIndex);
		if (!itemSet)
			continue;

		int currentCost = -1;
		WeaponSet currentSet;
		itemSet->GetCurrent(currentCost, currentSet);
		if (currentCost < 0)
			continue;

		char command[BUY_PRESET_COMMAND_LEN];
		currentSet.GenerateBuyCommands(command);
		if (command[0])
			gEngfuncs.pfnClientCmd(command);
		return;
	}
}

int BuyPresetManager::GetNumPresets()
{
	VerifyLoadedTeam();
	return m_presets.Count();
}

const BuyPreset *BuyPresetManager::GetPreset(int index) const
{
	if (index < 0 || index >= m_presets.Count())
		return NULL;
	return &m_presets[index];
}

void BuyPresetManager::SetPreset(int index, const BuyPreset *preset)
{
	VerifyLoadedTeam();
	if (!preset || index < 0 || index >= m_presets.Count())
		return;
	m_presets[index] = *preset;
}

BuyPreset *BuyPresetManager::GetEditPreset(int index)
{
	if (index < 0 || index >= m_editPresets.Count())
		return NULL;
	return &m_editPresets[index];
}

void BuyPresetManager::ResetEditToDefaults(void)
{
	BuildDefaultPresets(m_editPresets, LocalTeam());
}

void BuyPresetManager::GetCurrentLoadout(WeaponSet *weaponSet)
{
	if (!weaponSet)
		return;

	// This client does not expose Source's local weapon inventory API here.
	// Preserve Source's command path by saving a vanilla affordable loadout.
	weaponSet->Reset();
	weaponSet->m_armor = 100;
	weaponSet->m_helmet = true;
	weaponSet->m_HEGrenade = true;
	weaponSet->m_flashbangs = 1;
	weaponSet->m_primaryWeapon.SetWeaponID(LocalTeam() == TEAM_CT ? WEAPON_M4A1 : WEAPON_AK47);
	weaponSet->m_secondaryWeapon.SetWeaponID(LocalTeam() == TEAM_CT ? WEAPON_USP : WEAPON_GLOCK18);
}

void ShowBuyPresetMainMenu(bool, int)
{
}

void ShowBuyPresetEditMenu(int)
{
}

const char *ImageFnameFromWeaponID(CSWeaponID weaponID, bool isPrimary)
{
	const WeaponPresetInfo *info = FindWeaponInfo(weaponID);
	if (info)
		return info->image;

	return isPrimary ? "gfx/vgui/640_weaponblank" : "gfx/vgui/640_weaponpistolblank";
}

const BuyPresetWeaponList& CareerWeaponList(const BuyPresetWeaponList& source, bool, CSWeaponID)
{
	return source;
}

int CalcClipsNeeded(const BuyPresetWeapon *pWeapon, const void *, const int[])
{
	return pWeapon ? pWeapon->GetAmmoAmount() : 0;
}

void FillClientAmmo(int ammo[])
{
	if (!ammo)
		return;

	for (int i = 0; i < MAX_AMMO_TYPES; ++i)
		ammo[i] = 0;
}

bool CanBuyWeapon(CSWeaponID currentPrimaryID, CSWeaponID currentSecondaryID, CSWeaponID weaponID)
{
	if (weaponID == WEAPON_NONE || weaponID == currentPrimaryID || weaponID == currentSecondaryID)
		return true;

	const WeaponPresetInfo *info = FindWeaponInfo(weaponID);
	if (!info)
		return false;

	return (info->teamMask & TeamMaskForTeam(LocalTeam())) != 0;
}

const char *BuyPresetWeaponAlias(CSWeaponID weaponID)
{
	const WeaponPresetInfo *info = FindWeaponInfo(weaponID);
	return info ? info->alias : "";
}

int BuyPresetWeaponCost(CSWeaponID weaponID)
{
	const WeaponPresetInfo *info = FindWeaponInfo(weaponID);
	return info ? info->cost : 0;
}
