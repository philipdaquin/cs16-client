
#ifndef DARKALPHATEXTENTRY_H
#define DARKALPHATEXTENTRY_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IBorder.h>
#include <vgui/IScheme.h>
#include <KeyValues.h>

#include "../vgui_controls/TextEntry.h"
#include "../vgui_controls/ImagePanel.h"

class DarkAlphaTextEntry : public vgui2::TextEntry
{
	DECLARE_CLASS_SIMPLE(DarkAlphaTextEntry, TextEntry);
	typedef vgui2::TextEntry BaseClass;

public:
	DarkAlphaTextEntry(vgui2::Panel *parent, const char *panelName) :
		TextEntry(parent, panelName)
	{
		m_bImageBackground = false;
		for (int i = 0; i < 3; ++i)
		{
			m_pTopBackground[i] = nullptr;
			m_pCenterBackground[i] = nullptr;
			m_pBottomBackground[i] = nullptr;
		}
	}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme) override
	{
		BaseClass::ApplySchemeSettings(pScheme);
		m_bImageBackground = true;
		m_pTopBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_top_left@n", true);
		m_pTopBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_top_center@n", true);
		m_pTopBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_top_right@n", true);
		m_pCenterBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_center_left@n", true);
		m_pCenterBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_center_center@n", true);
		m_pCenterBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_center_right@n", true);
		m_pBottomBackground[0] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_bottom_left@n", true);
		m_pBottomBackground[1] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_bottom_center@n", true);
		m_pBottomBackground[2] = vgui2::scheme()->GetImage("resource/control/textentry/Tooltip_default_bottom_Right@n", true);

		SetFgColor(GetSchemeColor("LabelDimText", pScheme));
	}
	virtual void PaintBackground() override
	{
		// Keep the dark alpha text entry compatible with the base implementation.
		BaseClass::PaintBackground();
	}

private:
	bool m_bImageBackground;
	vgui2::IImage *m_pTopBackground[3];
	vgui2::IImage *m_pCenterBackground[3];
	vgui2::IImage *m_pBottomBackground[3];
};

#endif
