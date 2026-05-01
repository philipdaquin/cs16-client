#ifndef BUYPRESET_LISTBOX_H
#define BUYPRESET_LISTBOX_H

#ifdef _WIN32
#pragma once
#endif

#include <tier1/utlvector.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ScrollBar.h>

class BuyPresetListBox : public vgui2::Panel
{
	DECLARE_CLASS_SIMPLE(BuyPresetListBox, vgui2::Panel);

public:
	BuyPresetListBox(vgui2::Panel *parent, char const *panelName);
	~BuyPresetListBox();

	int AddItem(vgui2::Panel *panel, void *userData);
	int GetItemCount(void) const;
	void SwapItems(int index1, int index2);
	void MakeItemVisible(int index);
	vgui2::Panel *GetItemPanel(int index) const;
	void *GetItemUserData(int index);
	void SetItemUserData(int index, void *userData);
	void RemoveItem(int index);
	void DeleteAllItems();
	void OnSizeChanged(int wide, int tall) override;
	void OnMouseWheeled(int delta) override;
	void MoveScrollBarToTop();
	MESSAGE_FUNC_INT(OnSliderMoved, "ScrollBarSliderMoved", position);

protected:
	int computeVPixelsNeeded(void);
	void PerformLayout() override;
	void ApplySchemeSettings(vgui2::IScheme *pScheme) override;
	void OnCommand(const char *command) override;

private:
	enum { SCROLLBAR_SIZE = 18, DEFAULT_HEIGHT = 24, PANELBUFFER = 5 };
	struct DataItem
	{
		vgui2::Panel *panel;
		void *userData;
	};

	CUtlVector<DataItem> m_items;
	vgui2::ScrollBar *m_vbar;
	vgui2::Panel *m_pPanelEmbedded;
	int m_iScrollbarSize;
	int m_iDefaultHeight;
	int m_iPanelBuffer;
	int m_visibleIndex;
	int m_lastSize;
};

#endif
