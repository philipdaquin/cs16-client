#ifndef CSMOE_BUY_PRESETS_H
#define CSMOE_BUY_PRESETS_H

#ifdef _WIN32
#pragma once
#endif

#define USE_BUY_PRESETS 1

#include <KeyValues.h>
#include <tier1/utlvector.h>
#include "cdll_dll.h"
#include "weapontype.h"

typedef WeaponIdType CSWeaponID;

enum BuyPresetStringSizes
{
	BUY_PRESET_COMMAND_LEN = 256,
	MaxBuyPresetName = 64,
	MaxBuyPresetImageFname = 64,
};

enum AmmoSizeType
{
	AMMO_PERCENT,
	AMMO_CLIPS,
	AMMO_ROUNDS
};

enum { NUM_PRESETS = 4 };

class BuyPresetWeapon
{
public:
	BuyPresetWeapon();
	BuyPresetWeapon(CSWeaponID weaponID);
	BuyPresetWeapon& operator=(const BuyPresetWeapon& other);

	const wchar_t *GetName() const { return m_name; }
	CSWeaponID GetWeaponID() const { return m_weaponID; }
	void SetWeaponID(CSWeaponID weaponID);

	void SetAmmoType(AmmoSizeType ammoType) { m_ammoType = ammoType; }
	void SetAmmoAmount(int ammoAmount) { m_ammoAmount = ammoAmount; }
	void SetFillAmmo(bool fill) { m_fillAmmo = fill; }

	AmmoSizeType GetAmmoType() const { return m_ammoType; }
	int GetAmmoAmount() const { return m_ammoAmount; }
	bool GetFillAmmo() const { return m_fillAmmo; }

private:
	const wchar_t *m_name;
	CSWeaponID m_weaponID;
	AmmoSizeType m_ammoType;
	int m_ammoAmount;
	bool m_fillAmmo;
};

typedef CUtlVector<BuyPresetWeapon> BuyPresetWeaponList;

class WeaponSet
{
public:
	WeaponSet();

	void GetCurrent(int& cost, WeaponSet& ws) const;
	void GetFromScratch(int& cost, WeaponSet& ws) const;
	void GenerateBuyCommands(char command[BUY_PRESET_COMMAND_LEN]) const;
	int FullCost() const;
	void Reset(void);

	const BuyPresetWeapon& GetPrimaryWeapon() const { return m_primaryWeapon; }
	const BuyPresetWeapon& GetSecondaryWeapon() const { return m_secondaryWeapon; }

	BuyPresetWeapon m_primaryWeapon;
	BuyPresetWeapon m_secondaryWeapon;
	int m_armor;
	bool m_helmet;
	bool m_smokeGrenade;
	bool m_HEGrenade;
	int m_flashbangs;
	bool m_defuser;
	bool m_nightvision;
};

typedef CUtlVector<WeaponSet> WeaponSetList;

class BuyPreset
{
public:
	BuyPreset();
	~BuyPreset();
	BuyPreset(const BuyPreset& other);

	void SetName(const wchar_t *name);
	const wchar_t *GetName() const { return m_name; }

	void Parse(KeyValues *data);
	void Save(KeyValues *data);

	int GetNumSets() const { return m_weaponList.Count(); }
	const WeaponSet *GetSet(int index) const;
	int FullCost() const;

	void DeleteSet(int index);
	void SwapSet(int firstIndex, int secondIndex);
	void ReplaceSet(int index, const WeaponSet& weaponSet);

private:
	wchar_t m_name[MaxBuyPresetName];
	WeaponSetList m_weaponList;
};

typedef CUtlVector<BuyPreset> BuyPresetList;

class BuyPresetManager
{
public:
	BuyPresetManager();

	void Save();
	void PurchasePreset(int presetIndex);
	int GetNumPresets();
	const BuyPreset *GetPreset(int index) const;
	void SetPreset(int index, const BuyPreset *preset);

	void SetPresets(const BuyPresetList& presets) { m_presets = presets; }
	void SetEditPresets(const BuyPresetList& presets) { m_editPresets = presets; }
	int GetNumEditPresets() const { return m_editPresets.Count(); }
	BuyPreset *GetEditPreset(int index);
	const CUtlVector<BuyPreset>& GetEditPresets() const { return m_editPresets; }
	void ResetEditPresets() { m_editPresets = m_presets; }
	void ResetEditToDefaults(void);

	void GetCurrentLoadout(WeaponSet *weaponSet);

private:
	BuyPresetList m_presets;
	BuyPresetList m_editPresets;
	int m_loadedTeam;
	void VerifyLoadedTeam(void);
};

extern BuyPresetManager *TheBuyPresets;

void RegisterBuyPresetCommands(void);
void ShowBuyPresetMainMenu(bool runUpdate, int reopenBuyMenu);
void ShowBuyPresetEditMenu(int presetIndex);

const char *ImageFnameFromWeaponID(CSWeaponID weaponID, bool isPrimary);
const BuyPresetWeaponList& CareerWeaponList(const BuyPresetWeaponList& source, bool isPrimary, CSWeaponID currentClientID);
int CalcClipsNeeded(const BuyPresetWeapon *pWeapon, const void *pInfo, const int ammo[]);
void FillClientAmmo(int ammo[]);
bool CanBuyWeapon(CSWeaponID currentPrimaryID, CSWeaponID currentSecondaryID, CSWeaponID weaponID);
const char *BuyPresetWeaponAlias(CSWeaponID weaponID);
int BuyPresetWeaponCost(CSWeaponID weaponID);

#endif
