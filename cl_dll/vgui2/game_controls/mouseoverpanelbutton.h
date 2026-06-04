#ifndef MOUSEOVERPANELBUTTON_H
#define MOUSEOVERPANELBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui/IScheme.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/PHandle.h"
#include "vgui/KeyCode.h"
#include "vgui/MouseCode.h"
#include "FileSystem.h"
#include "hud.h"

class MouseOverPanelButton : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(MouseOverPanelButton, vgui2::Button);

public:
    MouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *templatePanel) : Button(parent, panelName, "MouseOverButton")
	{
		m_pPanel = new vgui2::EditablePanel(parent, NULL);
		m_pPanel ->SetVisible(false);

		int x, y, wide, tall;
		templatePanel->GetBounds(x, y, wide, tall);

		int px, py;
		templatePanel->GetPinOffset(px, py);

		int rx, ry;
		templatePanel->GetResizeOffset(rx, ry);

		m_pPanel->SetBounds(x, y, wide, tall);
		m_pPanel->SetAutoResize(templatePanel->GetPinCorner(), templatePanel->GetAutoResize(), px, py, rx, ry);

		m_bPreserveArmedButtons = false;
		m_bUpdateDefaultButtons = false;
	}

	virtual void SetPreserveArmedButtons(bool bPreserve) { m_bPreserveArmedButtons = bPreserve; }
	virtual void SetUpdateDefaultButtons(bool bUpdate) { m_bUpdateDefaultButtons = bUpdate; }

	virtual void ShowPage(void)
	{
		if (s_lastPanel)
		{
			for (int i = 0; i < s_lastPanel->GetParent()->GetChildCount(); i++)
			{
                MouseOverPanelButton *pButton = dynamic_cast<MouseOverPanelButton *>(s_lastPanel->GetParent()->GetChild(i));

				if (pButton)
					pButton->HidePage();
			}
		}

		if (m_pPanel)
		{
			m_pPanel->SetVisible(true);
			m_pPanel->MoveToFront();
			s_lastPanel = m_pPanel;
		}
	}

	virtual void HidePage(void)
	{
		if (m_pPanel)
			m_pPanel->SetVisible(false);
	}

	virtual const char *GetClassPage(const char *className)
	{
		static char classPanel[_MAX_PATH];
		Q_snprintf(classPanel, sizeof(classPanel), "classes/%s.res", className);
		return classPanel;
	}

	virtual bool LoadClassPage(void)
	{
		const char *classPage = GetClassPage(GetName());
		char lowerClassPage[_MAX_PATH];
		Q_strncpy(lowerClassPage, classPage, sizeof(lowerClassPage));
		for (char *p = lowerClassPage; *p; ++p)
		{
			if (*p >= 'A' && *p <= 'Z')
				*p = static_cast<char>(*p - 'A' + 'a');
		}

		if (vgui2::filesystem() && vgui2::filesystem()->FileExists(classPage))
		{
			m_pPanel->LoadControlSettings(classPage, "GAME");
		}
		else if (vgui2::filesystem() && vgui2::filesystem()->FileExists(lowerClassPage))
		{
			m_pPanel->LoadControlSettings(lowerClassPage, "GAME");
		}
		else
		{
			m_pPanel->LoadControlSettings("classes/default.res", "GAME");
		}

		return true;
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		BaseClass::ApplySettings(resourceData);

		// Main buy menu buttons should keep the authored resource width/height instead of stretching with the layout.
		const char *name = GetName();
		if (name && !strncmp(name, "wpn-button-", 11))
		{
			int pinX = 0;
			int pinY = 0;
			GetPinOffset(pinX, pinY);
			SetAutoResize(GetPinCorner(), AUTORESIZE_NO, pinX, pinY, 0, 0);
		}

		LoadClassPage();
	}

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();

		const char *name = GetName();
		const int textSuffixLen = Q_strlen("-text");
		if (!name || strncmp(name, "wpn-button-", 11) || Q_strlen(name) <= textSuffixLen)
			return;

		const int nameLen = Q_strlen(name);
		if (strcmp(name + nameLen - textSuffixLen, "-text"))
			return;

		char backgroundName[64];
		Q_strncpy(backgroundName, name, sizeof(backgroundName));
		backgroundName[nameLen - textSuffixLen] = 0;
		Q_strncat(backgroundName, "-bg", sizeof(backgroundName), COPY_ALL_CHARACTERS);

		vgui2::Panel *parent = GetParent();
		vgui2::Panel *background = parent ? parent->FindChildByName(backgroundName) : NULL;
		if (!background)
			return;

		int x, y, wide, tall;
		background->GetBounds(x, y, wide, tall);
		SetBounds(x, y, wide, tall);

		int pinX = 0;
		int pinY = 0;
		background->GetPinOffset(pinX, pinY);
		SetAutoResize(background->GetPinCorner(), AUTORESIZE_NO, pinX, pinY, 0, 0);
	}

	auto *GetClassPanel(void) { return m_pPanel; }

	virtual void OnCursorExited(void)
	{
		if (!m_bPreserveArmedButtons)
			BaseClass::OnCursorExited();
	}

	virtual void OnCursorEntered(void)
	{
		BaseClass::OnCursorEntered();

		if (!IsEnabled())
			return;

		if (m_bUpdateDefaultButtons)
			SetAsDefaultButton(1);

		if (m_bPreserveArmedButtons)
		{
			if (s_lastButton && s_lastButton != this)
                s_lastButton->SetArmed(false);

            s_lastButton = this;
		}

		if (m_pPanel)
		{
            if(!m_pPanel->IsVisible())
            {
                if (s_lastPanel && s_lastPanel->IsVisible())
                    s_lastPanel->SetVisible(false);

                ShowPage();
            }
		}
	}

	virtual void OnMousePressed(vgui2::MouseCode code)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] MouseOverPanelButton::OnMousePressed this=%p name='%s' code=%d visible=%d enabled=%d armed=%d panel=%p parent=%p\n",
			this,
			GetName() ? GetName() : "<null>",
			(int)code,
			IsVisible() ? 1 : 0,
			IsEnabled() ? 1 : 0,
			IsArmed() ? 1 : 0,
			(void *)m_pPanel,
			(void *)GetParent());
		BaseClass::OnMousePressed(code);
	}

	virtual void OnMouseReleased(vgui2::MouseCode code)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] MouseOverPanelButton::OnMouseReleased this=%p name='%s' code=%d visible=%d enabled=%d armed=%d panel=%p parent=%p\n",
			this,
			GetName() ? GetName() : "<null>",
			(int)code,
			IsVisible() ? 1 : 0,
			IsEnabled() ? 1 : 0,
			IsArmed() ? 1 : 0,
			(void *)m_pPanel,
			(void *)GetParent());
		BaseClass::OnMouseReleased(code);
	}

	virtual void OnKeyCodeReleased(vgui2::KeyCode code)
	{
		BaseClass::OnKeyCodeReleased(code);

		if (m_bPreserveArmedButtons)
		{
			if (s_lastButton)
                s_lastButton->SetArmed(true);
		}
	}

protected:
	vgui2::EditablePanel *m_pPanel;
	bool m_bPreserveArmedButtons;
	bool m_bUpdateDefaultButtons;

public:
    static inline vgui2::DHANDLE<vgui2::Panel> s_lastPanel;
    static inline vgui2::DHANDLE<vgui2::Button> s_lastButton;
};

#endif
