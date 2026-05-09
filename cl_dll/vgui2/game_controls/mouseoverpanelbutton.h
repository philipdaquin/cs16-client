#ifndef MOUSEOVERPANELBUTTON_H
#define MOUSEOVERPANELBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui/IScheme.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/PHandle.h"
#include "vgui/KeyCode.h"
#include "FileSystem.h"
#include <ctype.h>

extern vgui2::Panel *g_lastPanel;
extern vgui2::Button *g_lastButton;

class MouseOverPanelButton : public vgui2::Button
{
	DECLARE_CLASS_SIMPLE(MouseOverPanelButton, vgui2::Button);

public:
    MouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *templatePanel) : Button(parent, panelName, "MouseOverButton")
	{
			m_pPanel = new vgui2::EditablePanel(parent, NULL);
			m_pPanel ->SetVisible(false);
			m_pPanel->SetMouseInputEnabled(false);
			m_pPanel->SetKeyBoardInputEnabled(false);

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
			m_bDisablePageInput = true;
		}

	virtual void SetPreserveArmedButtons(bool bPreserve) { m_bPreserveArmedButtons = bPreserve; }
	virtual void SetUpdateDefaultButtons(bool bUpdate) { m_bUpdateDefaultButtons = bUpdate; }
	virtual void SetDisablePageInput(bool bDisable) { m_bDisablePageInput = bDisable; }

	virtual void ShowPage(void)
	{
		if (m_pPanel)
		{
			if (m_bDisablePageInput)
				DisableInputRecursive(m_pPanel);
			m_pPanel->SetVisible(true);
			m_pPanel->MoveToFront();
			g_lastPanel = m_pPanel;
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
		char normalizedName[_MAX_PATH];
		char lowerName[_MAX_PATH];

		auto tryResource = [&](const char *name) -> const char * {
			if (!name || !name[0])
				return NULL;

			Q_snprintf(classPanel, sizeof(classPanel), "classes/%s.res", name);
			if (g_pFullFileSystem && g_pFullFileSystem->FileExists(classPanel))
				return classPanel;
			return NULL;
		};

		Q_strncpy(normalizedName, className ? className : "", sizeof(normalizedName));

		const char *suffixes[] = { "-bottom", "-text", "_cmd", "_bottom", "_text" };
		for (const char *suffix : suffixes)
		{
			const size_t suffixLen = Q_strlen(suffix);
			const size_t nameLen = Q_strlen(normalizedName);
			if (nameLen > suffixLen && !Q_stricmp(normalizedName + nameLen - suffixLen, suffix))
			{
				normalizedName[nameLen - suffixLen] = '\0';
				break;
			}
		}

		const char *candidates[] = { className, normalizedName };
		for (const char *candidate : candidates)
		{
			const char *found = tryResource(candidate);
			if (found)
				return found;

			if (candidate && candidate[0])
			{
				Q_strncpy(lowerName, candidate, sizeof(lowerName));
				for (char *p = lowerName; *p; ++p)
					*p = (char)tolower((unsigned char)*p);

				if (Q_stricmp(lowerName, candidate))
				{
					found = tryResource(lowerName);
					if (found)
						return found;
				}
			}
		}

		if (g_pFullFileSystem && g_pFullFileSystem->FileExists("classes/default.res"))
		{
			Q_snprintf(classPanel, sizeof(classPanel), "classes/default.res");
			return classPanel;
		}

		return NULL;
	}

	virtual bool LoadClassPage(void)
	{
		const char *classPage = GetClassPage(GetName());

		if (classPage && classPage[0])
		{
			m_pPanel->LoadControlSettings(classPage, "GAME");
			if (m_bDisablePageInput)
				DisableInputRecursive(m_pPanel);
		}

		return true;
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		BaseClass::ApplySettings(resourceData);

		LoadClassPage();
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
			if (g_lastButton && g_lastButton != this)
                g_lastButton->SetArmed(false);

            g_lastButton = this;
		}

		if (m_pPanel)
		{
            if(!m_pPanel->IsVisible())
            {
                if (g_lastPanel && g_lastPanel->IsVisible())
                    g_lastPanel->SetVisible(false);

                ShowPage();
            }
		}
	}

	virtual void OnKeyCodeReleased(vgui2::KeyCode code)
	{
		BaseClass::OnKeyCodeReleased(code);

		if (m_bPreserveArmedButtons)
		{
			if (g_lastButton)
                g_lastButton->SetArmed(true);
		}
	}

protected:
	static void DisableInputRecursive(vgui2::Panel *panel)
	{
		if (!panel)
			return;

		panel->SetMouseInputEnabled(false);
		panel->SetKeyBoardInputEnabled(false);

		for (int i = 0; i < panel->GetChildCount(); ++i)
			DisableInputRecursive(panel->GetChild(i));
	}

	vgui2::EditablePanel *m_pPanel;
	bool m_bPreserveArmedButtons;
	bool m_bUpdateDefaultButtons;
	bool m_bDisablePageInput;

};

#endif
