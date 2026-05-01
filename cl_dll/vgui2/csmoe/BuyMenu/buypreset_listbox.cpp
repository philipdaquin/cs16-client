#include "buypreset_listbox.h"
#include <vgui/IScheme.h>
#include <vgui_controls/EditablePanel.h>

using namespace vgui2;

static int MaxInt(int a, int b)
{
	return (a > b) ? a : b;
}

BuyPresetListBox::BuyPresetListBox(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	m_visibleIndex = -1;
	m_lastSize = 0;
	m_iScrollbarSize = SCROLLBAR_SIZE;
	m_iDefaultHeight = DEFAULT_HEIGHT;
	m_iPanelBuffer = PANELBUFFER;
	m_vbar = new ScrollBar(this, "PanelListPanelVScroll", true);
	m_vbar->AddActionSignalTarget(this);
	m_pPanelEmbedded = new EditablePanel(this, "PanelListEmbedded");
	m_pPanelEmbedded->SetPaintBackgroundEnabled(false);
	m_pPanelEmbedded->SetPaintBorderEnabled(false);
}

BuyPresetListBox::~BuyPresetListBox()
{
	DeleteAllItems();
}

void BuyPresetListBox::OnCommand(const char *command)
{
	if (GetParent())
		GetParent()->OnCommand(command);
}

void BuyPresetListBox::OnMouseWheeled(int delta)
{
	m_vbar->SetValue(m_vbar->GetValue() - delta * m_iDefaultHeight);
}

int BuyPresetListBox::computeVPixelsNeeded(void)
{
	int pixels = m_iPanelBuffer;
	for (int i = 0; i < m_items.Count(); ++i)
	{
		if (m_items[i].panel)
			pixels += m_items[i].panel->GetTall() + m_iPanelBuffer;
	}
	return pixels;
}

int BuyPresetListBox::AddItem(Panel *panel, void *userData)
{
	if (!panel)
		return -1;

	DataItem item = { panel, userData };
	panel->SetParent(m_pPanelEmbedded);
	m_items.AddToTail(item);
	InvalidateLayout();
	return m_items.Count() - 1;
}

int BuyPresetListBox::GetItemCount(void) const
{
	return m_items.Count();
}

void BuyPresetListBox::SwapItems(int index1, int index2)
{
	if (index1 < 0 || index2 < 0 || index1 >= m_items.Count() || index2 >= m_items.Count())
		return;

	DataItem temp = m_items[index1];
	m_items[index1] = m_items[index2];
	m_items[index2] = temp;
	InvalidateLayout();
}

Panel *BuyPresetListBox::GetItemPanel(int index) const
{
	return (index >= 0 && index < m_items.Count()) ? m_items[index].panel : NULL;
}

void *BuyPresetListBox::GetItemUserData(int index)
{
	return (index >= 0 && index < m_items.Count()) ? m_items[index].userData : NULL;
}

void BuyPresetListBox::SetItemUserData(int index, void *userData)
{
	if (index >= 0 && index < m_items.Count())
		m_items[index].userData = userData;
}

void BuyPresetListBox::RemoveItem(int index)
{
	if (index < 0 || index >= m_items.Count())
		return;

	if (m_items[index].panel)
		m_items[index].panel->MarkForDeletion();
	m_items.Remove(index);
	InvalidateLayout();
}

void BuyPresetListBox::DeleteAllItems()
{
	while (m_items.Count())
		RemoveItem(0);
	m_vbar->SetValue(0);
}

void BuyPresetListBox::OnSizeChanged(int wide, int tall)
{
	BaseClass::OnSizeChanged(wide, tall);
	InvalidateLayout();
}

void BuyPresetListBox::PerformLayout()
{
	BaseClass::PerformLayout();
	int wide, tall;
	GetSize(wide, tall);
	const int vpixels = computeVPixelsNeeded();
	m_vbar->SetBounds(wide - m_iScrollbarSize, 1, m_iScrollbarSize, tall - 2);
	m_vbar->SetRange(0, MaxInt(0, vpixels - tall + m_iDefaultHeight));
	m_vbar->SetRangeWindow(m_iDefaultHeight);
	m_pPanelEmbedded->SetBounds(1, -m_vbar->GetValue(), wide - m_iScrollbarSize - 2, vpixels);

	int y = m_iPanelBuffer;
	for (int i = 0; i < m_items.Count(); ++i)
	{
		Panel *panel = m_items[i].panel;
		if (!panel)
			continue;
		panel->SetBounds(8, y, wide - m_iScrollbarSize - 16, panel->GetTall());
		y += panel->GetTall() + m_iPanelBuffer;
	}
	m_lastSize = vpixels;
}

void BuyPresetListBox::MakeItemVisible(int index)
{
	m_visibleIndex = index;
	m_lastSize = 0;
	InvalidateLayout();
}

void BuyPresetListBox::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetBgColor(pScheme->GetColor("BuyPresetListBox.BgColor", GetBgColor()));
	SetBorder(pScheme->GetBorder("BrowserBorder"));
	m_vbar->SetBorder(pScheme->GetBorder("BrowserBorder"));
}

void BuyPresetListBox::OnSliderMoved(int)
{
	InvalidateLayout();
	Repaint();
}

void BuyPresetListBox::MoveScrollBarToTop()
{
	m_vbar->SetValue(0);
	OnSliderMoved(0);
}
