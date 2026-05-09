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
#include <vgui/MouseCode.h>
#include <vgui_controls/ImagePanel.h>

class BuyMouseOverPanelButton : public NewMouseOverPanelButton
{
	typedef NewMouseOverPanelButton BaseClass;

public:
	BuyMouseOverPanelButton(vgui2::Panel *parent, const char *panelName, vgui2::EditablePanel *panel) : BaseClass(parent, panelName, panel)
	{
		SetButtonActivationType(ACTIVATE_ONPRESSED);
		m_iPrice = 0;
		m_iASRestrict = 0;
		m_iDEUseOnly = 0;
	}

	~BuyMouseOverPanelButton(void)
	{
	}

	virtual void ApplySettings(KeyValues *resourceData)
	{
		KeyValues *kv = resourceData->FindKey("cost", false);

		if (kv)
			m_iPrice = kv->GetInt();

		kv = resourceData->FindKey("as_restrict", false);

		if (kv)
			m_iASRestrict = kv->GetInt();

		kv = resourceData->FindKey("de_useonly", false);

		if (kv)
			m_iDEUseOnly = kv->GetInt();

		BaseClass::ApplySettings(resourceData);
	}

	int GetASRestrict(void) { return m_iASRestrict; }
	int GetDEUseOnly(void) { return m_iDEUseOnly; }

	virtual void PerformLayout(void)
	{
		BaseClass::PerformLayout();

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

		if (!m_pKeyboard)
			m_pKeyboard = vgui2::scheme()->GetImage("resource/Control/button_ingame/keyboard", true);

		if (!m_pBlankSlot)
			m_pBlankSlot = vgui2::scheme()->GetImage(pScheme->GetResourceString("BuyButton.Up"), true);

		if (!m_pSelect)
			m_pSelect = vgui2::scheme()->GetImage(pScheme->GetResourceString("BuyButton.On"), true);

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
	}

	virtual void PaintBackground(void)
	{
		BaseClass::PaintBackground();
	}

	virtual void OnMousePressed(vgui2::MouseCode code)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] BuyMouseOverPanelButton::OnMousePressed this=%p name='%s' code=%d visible=%d enabled=%d armed=%d price=%d panel=%p parent=%p\n",
			this,
			GetName() ? GetName() : "<null>",
			(int)code,
			IsVisible() ? 1 : 0,
			IsEnabled() ? 1 : 0,
			IsArmed() ? 1 : 0,
			m_iPrice,
			(void *)GetClassPanel(),
			(void *)GetParent());
		BaseClass::OnMousePressed(code);
	}

	virtual void OnMouseReleased(vgui2::MouseCode code)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] BuyMouseOverPanelButton::OnMouseReleased this=%p name='%s' code=%d visible=%d enabled=%d armed=%d price=%d panel=%p parent=%p\n",
			this,
			GetName() ? GetName() : "<null>",
			(int)code,
			IsVisible() ? 1 : 0,
			IsEnabled() ? 1 : 0,
			IsArmed() ? 1 : 0,
			m_iPrice,
			(void *)GetClassPanel(),
			(void *)GetParent());
		BaseClass::OnMouseReleased(code);
	}

	virtual void FireActionSignal(void)
	{
		gEngfuncs.Con_Printf("[VGUI2-CLIENT] BuyMouseOverPanelButton::FireActionSignal this=%p name='%s' price=%d armed=%d visible=%d enabled=%d panel=%p parent=%p\n",
			this,
			GetName() ? GetName() : "<null>",
			m_iPrice,
			IsArmed() ? 1 : 0,
			IsVisible() ? 1 : 0,
			IsEnabled() ? 1 : 0,
			(void *)GetClassPanel(),
			(void *)GetParent());
		BaseClass::FireActionSignal();
	}

	void SetCurrentPrice(int iPrice)
	{
		m_iPrice = iPrice;
	}

	int GetCurrentPrice(void)
	{
		return m_iPrice;
	}

protected:
	int m_iPrice;
	int m_iKeyboard;
	int m_iASRestrict;
	int m_iDEUseOnly;
};

#endif
