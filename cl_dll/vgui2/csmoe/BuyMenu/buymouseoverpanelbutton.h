#ifndef BUYMOUSEOVERPANELBUTTON_H
#define BUYMOUSEOVERPANELBUTTON_H

#ifdef _WIN32
#pragma once
#endif

#include <KeyValues.h>
#include <FileSystem.h>
#include "../newmouseoverpanelbutton.h"
#include "hud.h"
#include "shared_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui_controls/ImagePanel.h>

class BuyMouseOverPanelButton : public NewMouseOverPanelButton
{
	typedef NewMouseOverPanelButton BaseClass;

public:
	BuyMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *panel) : BaseClass(parent, panelName, panel)
	{
		SetDisablePageInput(false);
		SetButtonActivationType(ACTIVATE_ONPRESSED);
		m_iPrice = 0;
		m_iPreviousPrice = 0;
		m_iASRestrict = 0;
		m_iDEUseOnly = 0;
		m_bIsBargain = false;
		m_command = NULL;
		m_avaliableColor = Color(0, 0, 0, 0);
		m_unavailableColor = Color(0, 0, 0, 0);
		m_bargainColor = Color(0, 255, 0, 192);
		m_pBlackMarketPrice = NULL;
	}

	~BuyMouseOverPanelButton(void)
	{
		delete [] m_command;
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		BaseClass::ApplySettings(resourceData);

		KeyValues *kv = resourceData->FindKey("cost", false);
		if (kv)
			m_iPrice = kv->GetInt();

		kv = resourceData->FindKey("as_restrict", false);
		if (kv)
			m_iASRestrict = kv->GetInt();

		kv = resourceData->FindKey("de_useonly", false);
		if (kv)
			m_iDEUseOnly = kv->GetInt();

		delete [] m_command;
		m_command = NULL;

		kv = resourceData->FindKey("command", false);
		if (kv)
			m_command = CloneCommand(kv->GetString());

		SetPriceState();
		SetMapTypeState();
	}

	int GetASRestrict(void) { return m_iASRestrict; }
	int GetDEUseOnly(void) { return m_iDEUseOnly; }

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();
		SetPriceState();
		SetMapTypeState();

#ifndef CS_SHIELD_ENABLED
		if (!Q_stricmp(GetName(), "shield"))
		{
			SetVisible(false);
			SetEnabled(false);
		}
#endif
	}

	virtual void ApplySchemeSettings(vgui2::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		m_avaliableColor = pScheme->GetColor("Label.TextColor", Color(0, 0, 0, 0));
		m_unavailableColor = pScheme->GetColor("Label.DisabledFgColor2", Color(0, 0, 0, 0));
		m_bargainColor = Color(0, 255, 0, 192);

		if (m_iKeyOffset == -1)
		{
			m_iKeyOffset = 3;

			if (IsProportional())
				m_iKeyOffset = vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), m_iKeyOffset);
		}

		if (m_iKeySize == -1)
		{
			m_iKeySize = 18;

			if (IsProportional())
				m_iKeySize = vgui2::scheme()->GetProportionalScaledValueEx(GetScheme(), m_iKeySize);
		}

		SetPriceState();
		SetMapTypeState();
	}

	virtual void OnCursorEntered(void)
	{
		::MouseOverPanelButton::OnCursorEntered();
	}

	virtual void PaintBackground(void)
	{
		BaseClass::PaintBackground();
	}

	void SetCurrentPrice(int iPrice)
	{
		m_iPrice = iPrice;
	}

	void SetPreviousPrice(int iPrice)
	{
		m_iPreviousPrice = iPrice;
	}

	int GetCurrentPrice(void)
	{
		return m_iPrice;
	}

	const char *GetBuyCommand(void)
	{
		return m_command;
	}

	void SetBargainButton(bool state)
	{
		m_bIsBargain = state;
	}

	void SetPriceState(void)
	{
		bool available = true;
		const int account = cl::gHUD.m_Money.m_iMoneyCount;

		if (m_iPrice > 0 && m_iPrice > account)
			available = false;

		if (available)
		{
			SetFgColor(m_bIsBargain ? m_bargainColor : m_avaliableColor);
			if (m_command)
				SetCommand(m_command);
		}
		else
		{
			SetFgColor(m_unavailableColor);
			SetCommand("buy_unavailable");
		}
	}

	void SetMapTypeState(void)
	{
		// Source also gates by as_/de_ map rules. This client does not expose
		// a CSGameRules equivalent here, so keep parsed flags but avoid guessing.
	}

	void RefreshState(void)
	{
		SetPriceState();
		SetMapTypeState();
	}

	virtual void ShowPage(void)
	{
		if (g_lastPanel && g_lastPanel->GetParent())
		{
			for (int i = 0; i < g_lastPanel->GetParent()->GetChildCount(); ++i)
			{
				MouseOverPanelButton *buyButton = dynamic_cast<MouseOverPanelButton *>(g_lastPanel->GetParent()->GetChild(i));
				if (buyButton)
					buyButton->HidePage();
			}
		}

		BaseClass::ShowPage();
	}

	virtual void HidePage(void)
	{
		BaseClass::HidePage();
	}

protected:
	static char *CloneCommand(const char *command)
	{
		if (!command || !command[0])
			return NULL;

		const size_t len = Q_strlen(command) + 1;
		char *out = new char[len];
		Q_strncpy(out, command, len);
		return out;
	}

	int m_iPrice;
	int m_iPreviousPrice;
	int m_iKeyboard;
	int m_iASRestrict;
	int m_iDEUseOnly;
	bool m_bIsBargain;
	char *m_command;
	Color m_avaliableColor;
	Color m_unavailableColor;
	Color m_bargainColor;

public:
	vgui2::EditablePanel *m_pBlackMarketPrice;
};

#endif
