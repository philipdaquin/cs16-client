#include "hud.h"
#include <stdio.h>
#include <wchar.h>
#include <tier1/utlsymbol.h>

#include <vgui/IBorder.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <vgui/MouseCode.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include "WeaponImagePanel.h"

using namespace vgui2;

struct VanillaWeaponImageAlias
{
	const char *name;
	const char *image;
};

static const VanillaWeaponImageAlias kVanillaWeaponImageAliases[] =
{
	{ "weapon_usp", "usp45" },
	{ "usp", "usp45" },
	{ "weapon_glock18", "glock18" },
	{ "weapon_deagle", "deserteagle" },
	{ "deagle", "deserteagle" },
	{ "weapon_p228", "p228" },
	{ "weapon_fiveseven", "fiveseven" },
	{ "weapon_elite", "elites" },
	{ "elite", "elites" },
	{ "weapon_m3", "m3" },
	{ "weapon_xm1014", "xm1014" },
	{ "weapon_tmp", "tmp" },
	{ "weapon_mac10", "mac10" },
	{ "weapon_mp5navy", "mp5" },
	{ "mp5navy", "mp5" },
	{ "weapon_ump45", "ump45" },
	{ "weapon_p90", "p90" },
	{ "weapon_famas", "famas" },
	{ "weapon_galil", "galil" },
	{ "weapon_ak47", "ak47" },
	{ "weapon_m4a1", "m4a1" },
	{ "weapon_aug", "aug" },
	{ "weapon_sg552", "sg552" },
	{ "weapon_scout", "scout" },
	{ "weapon_awp", "awp" },
	{ "weapon_g3sg1", "g3sg1" },
	{ "weapon_sg550", "sg550" },
	{ "weapon_m249", "m249" },
	{ "weapon_hegrenade", "hegrenade" },
	{ "hegrenade", "hegrenade" },
	{ "weapon_flashbang", "flashbang" },
	{ "flash", "flashbang" },
	{ "flashbang", "flashbang" },
	{ "weapon_smokegrenade", "smokegrenade" },
	{ "sgren", "smokegrenade" },
	{ "smokegrenade", "smokegrenade" },
	{ "vest", "kevlar" },
	{ "vesthelm", "kevlar_helmet" },
	{ "defuser", "defuser" },
	{ "nvgs", "nightvision" },
	{ "nightvision", "nightvision" },
	{ "shield", "shield" },
	{ "weapon_shield", "shield" },
	{ "weapon_knife", "" },
	{ "knife", "" },
};

WeaponImagePanel::WeaponImagePanel(Panel *parent, const char *name) : BaseClass(parent, name)
{
	m_bBanned = false;
	m_pBannedImage = vgui2::scheme()->GetImage("gfx/vgui/not_available", true);
}

const char *WeaponImagePanel::ResolveVanillaImageName(const char *name)
{
	if (!name || !name[0])
	{
		return "";
	}

	for (size_t i = 0; i < sizeof(kVanillaWeaponImageAliases) / sizeof(kVanillaWeaponImageAliases[0]); ++i)
	{
		if (!stricmp(name, kVanillaWeaponImageAliases[i].name))
			return kVanillaWeaponImageAliases[i].image;
	}

	if (!strncmp(name, "weapon_", 7))
		name += 7;
	else if (!strncmp(name, "z4b_", 4))
		name += 4;
	else if (!strncmp(name, "knife_", 6))
		name += 6;

	if (!stricmp(name, "mp5navy"))
		return "mp5";
	if (!stricmp(name, "scarl") || !stricmp(name, "scarh"))
		return "scar";
	if (!stricmp(name, "xm8c") || !stricmp(name, "xm8s"))
		return "xm8";

	return name;
}

void WeaponImagePanel::BuildVanillaImagePath(const char *name, char *path, size_t pathSize)
{
	if (!path || !pathSize)
		return;

	const char *imageName = ResolveVanillaImageName(name);
	if (!imageName || !imageName[0])
	{
		path[0] = '\0';
		return;
	}

	if (!strncmp(imageName, "gfx/vgui/", 9))
	{
		snprintf(path, pathSize, "%s", imageName);
		return;
	}

	snprintf(path, pathSize, "gfx/vgui/%s", imageName);
}

void WeaponImagePanel::SetWeapon(const char *name)
{
	if (!name || !name[0])
	{
		return SetWeapon(nullptr);
	}

	char path[MAX_PATH];
	BuildVanillaImagePath(name, path, sizeof(path));
	if (path[0])
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] WeaponImagePanel::SetWeapon name='%s' resolved='%s'\n",
			name, path);
	}
	SetImage(path);

	m_bBanned = false;
}

void WeaponImagePanel::SetWeapon(nullptr_t)
{
	SetImage((vgui2::IImage *)NULL);
	m_bBanned = false;
}

void WeaponImagePanel::PaintBackground()
{
	BaseClass::PaintBackground();
	
	if (m_bBanned)
	{
		vgui2::IImage *backup = GetImage();
		SetImage(m_pBannedImage);
		BaseClass::PaintBackground();
		SetImage(backup);
	}
	
}
