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

WeaponImagePanel::WeaponImagePanel(Panel *parent, const char *name) : BaseClass(parent, name)
{
	m_bBanned = false;
	m_pBannedImage = vgui2::scheme()->GetImage("gfx/vgui/not_available", true);
}

static void BuildWeaponImagePath(const char *name, char *path, size_t pathSize)
{
	if (!path || !pathSize)
		return;

	if (!name || !name[0])
	{
		path[0] = '\0';
		return;
	}

	if (!strncmp(name, "gfx/vgui/", 9))
	{
		snprintf(path, pathSize, "%s", name);
		return;
	}

	snprintf(path, pathSize, "gfx/vgui/%s", name);
}

void WeaponImagePanel::SetWeapon(const char *name)
{
	if (!name || !name[0])
	{
		return SetWeapon(nullptr);
	}

	char path[MAX_PATH];
	BuildWeaponImagePath(name, path, sizeof(path));
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
