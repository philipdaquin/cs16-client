#include "cs_basepanel.h"

#if !defined(VGUI2_STUB_MODE)

#include <string.h>

#include <KeyValues.h>
#include <tier2/tier2.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>

CCSBasePanel::CCSBasePanel(vgui2::Panel *parent, const char *panelName)
	: BaseClass(parent, panelName)
	, m_bTexturedBackground(false)
	, m_nBackgroundMaterial(-1)
	, m_bTiled(false)
	, m_bReflectMouse(false)
{
	m_szBgTexture[0] = '\0';
	m_nTextureSize[0] = 0;
	m_nTextureSize[1] = 0;
}

CCSBasePanel::CCSBasePanel(vgui2::Panel *parent, const char *panelName, int x, int y, int w, int h)
	: BaseClass(parent, panelName)
	, m_bTexturedBackground(false)
	, m_nBackgroundMaterial(-1)
	, m_bTiled(false)
	, m_bReflectMouse(false)
{
	SetBounds(x, y, w, h);
	m_szBgTexture[0] = '\0';
	m_nTextureSize[0] = 0;
	m_nTextureSize[1] = 0;
}

CCSBasePanel::~CCSBasePanel()
{
	if (vgui2::surface() && m_nBackgroundMaterial != -1)
	{
		vgui2::surface()->DestroyTextureID(m_nBackgroundMaterial);
		m_nBackgroundMaterial = -1;
	}
}

void CCSBasePanel::PaintBackground()
{
	if (!m_bTexturedBackground)
	{
		BaseClass::PaintBackground();
		return;
	}

	if (m_nBackgroundMaterial == -1)
	{
		m_nBackgroundMaterial = vgui2::surface()->CreateNewTextureID();
		vgui2::surface()->DrawSetTextureFile(m_nBackgroundMaterial, m_szBgTexture, true, true);
	}

	vgui2::surface()->DrawSetColor(GetFgColor());
	vgui2::surface()->DrawSetTexture(m_nBackgroundMaterial);

	if (m_bTiled && m_nTextureSize[0] == 0)
	{
		vgui2::surface()->DrawGetTextureSize(m_nBackgroundMaterial, m_nTextureSize[0], m_nTextureSize[1]);
	}

	int wide = 0;
	int tall = 0;
	GetSize(wide, tall);

	if (m_bTiled && m_nTextureSize[0] > 0 && m_nTextureSize[1] > 0)
	{
		for (int y = 0; y < tall; y += m_nTextureSize[1])
		{
			for (int x = 0; x < wide; x += m_nTextureSize[0])
			{
				vgui2::surface()->DrawTexturedRect(x, y, x + m_nTextureSize[0], y + m_nTextureSize[1]);
			}
		}
	}
	else
	{
		vgui2::surface()->DrawTexturedRect(0, 0, wide, tall);
	}
}

void CCSBasePanel::SetTexture(const char *textureName, bool tiled)
{
	if (!textureName || !textureName[0])
		return;

	m_bTexturedBackground = true;
	m_bTiled = tiled;
	Q_strncpy(m_szBgTexture, textureName, sizeof(m_szBgTexture));
	m_nTextureSize[0] = 0;
	m_nTextureSize[1] = 0;

	if (m_nBackgroundMaterial == -1)
		m_nBackgroundMaterial = vgui2::surface()->CreateNewTextureID();

	vgui2::surface()->DrawSetTextureFile(m_nBackgroundMaterial, m_szBgTexture, true, true);
}

void CCSBasePanel::SetReflectMouse(bool reflect)
{
	m_bReflectMouse = reflect;
}

void CCSBasePanel::OnCursorMoved(int x, int y)
{
	if (!m_bReflectMouse || !GetParent())
		return;

	LocalToScreen(x, y);
	vgui2::ivgui()->PostMessage(GetParent()->GetVPanel(), new KeyValues("CursorMoved", "xpos", x, "ypos", y), GetVPanel());
}

void CCSBasePanel::OnMousePressed(vgui2::MouseCode code)
{
	if (!m_bReflectMouse || !GetParent())
		return;

	vgui2::ivgui()->PostMessage(GetParent()->GetVPanel(), new KeyValues("MousePressed", "code", code), GetVPanel());
}

void CCSBasePanel::OnMouseDoublePressed(vgui2::MouseCode code)
{
	if (!m_bReflectMouse || !GetParent())
		return;

	vgui2::ivgui()->PostMessage(GetParent()->GetVPanel(), new KeyValues("MouseDoublePressed", "code", code), GetVPanel());
}

void CCSBasePanel::OnMouseReleased(vgui2::MouseCode code)
{
	if (!m_bReflectMouse || !GetParent())
		return;

	vgui2::ivgui()->PostMessage(GetParent()->GetVPanel(), new KeyValues("MouseReleased", "code", code), GetVPanel());
}

void CCSBasePanel::OnMouseWheeled(int delta)
{
	if (!m_bReflectMouse || !GetParent())
		return;

	vgui2::ivgui()->PostMessage(GetParent()->GetVPanel(), new KeyValues("MouseWheeled", "delta", delta), GetVPanel());
}

#endif
