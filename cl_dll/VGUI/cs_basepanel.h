#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

#include <vgui_controls/Panel.h>

class CCSBasePanel : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(CCSBasePanel, vgui2::Panel);

public:
	CCSBasePanel(vgui2::Panel *parent, const char *panelName);
	CCSBasePanel(vgui2::Panel *parent, const char *panelName, int x, int y, int w, int h);
	~CCSBasePanel() override;

	void PaintBackground() override;
	void SetTexture(const char *textureName, bool tiled = false);
	void SetReflectMouse(bool reflect);
	void OnCursorMoved(int x, int y) override;
	void OnMousePressed(vgui2::MouseCode code) override;
	void OnMouseDoublePressed(vgui2::MouseCode code) override;
	void OnMouseReleased(vgui2::MouseCode code) override;
	void OnMouseWheeled(int delta) override;

private:
	bool m_bTexturedBackground;
	int m_nBackgroundMaterial;
	char m_szBgTexture[256];
	bool m_bTiled;
	int m_nTextureSize[2];
	bool m_bReflectMouse;
};

#endif
