#include "vgui2_client_runtime.h"

#if !defined(VGUI2_STUB_MODE)

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <wchar.h>

#include <Color.h>
#include <KeyValues.h>
#include <FileSystem.h>
#include <vgui/IClientPanel.h>
#include <vgui/IInputInternal.h>
#include <vgui/KeyCode.h>
#include <vgui/MouseCode.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Panel.h>
#include <tier2/tier2.h>

typedef float vec_t;
typedef vec_t vec3_t[3];

#include "include/wrect.h"
#include "cl_dll.h"
#include "cvardef.h"
#include "../engine/keydefs.h"

namespace
{

struct FontData
{
	bool valid;
	int tall;
	int weight;
	int blur;
	int scanlines;
	int flags;
	int lowRange;
	int highRange;
	int charWidth;
	std::string name;

	FontData()
		: valid(false), tall(13), weight(0), blur(0), scanlines(0), flags(0),
		  lowRange(0), highRange(255), charWidth(8)
	{
	}
};

struct PanelData
{
	bool alive;
	int pos[2];
	int size[2];
	int absPos[2];
	int inset[4];
	int zpos;
	bool visible;
	bool enabled;
	bool popup;
	bool keyboardInputEnabled;
	bool mouseInputEnabled;
	bool minimized;
	bool topmost;
	bool needsSolve;
	vgui2::VPANEL parent;
	std::vector<vgui2::VPANEL> children;
	vgui2::IClientPanel *clientPanel;
	vgui2::SurfacePlat *plat;

	PanelData()
		: alive(false), zpos(0), visible(true), enabled(true), popup(false),
		  keyboardInputEnabled(false), mouseInputEnabled(false), minimized(false),
		  topmost(false), needsSolve(true), parent(vgui2::INVALID_PANEL),
		  clientPanel(NULL), plat(NULL)
	{
		pos[0] = pos[1] = 0;
		size[0] = size[1] = 64;
		absPos[0] = absPos[1] = 0;
		inset[0] = inset[1] = inset[2] = inset[3] = 0;
	}
};

struct ClipRect
{
	int x0;
	int y0;
	int x1;
	int y1;
};

static std::vector<PanelData> s_panels;
static std::vector<vgui2::VPANEL> s_deleteQueue;
static std::vector<ClipRect> s_clipStack;
static std::map<int, FontData> s_fonts;
static vgui2::VPANEL s_embeddedPanel = vgui2::INVALID_PANEL;
static vgui2::VPANEL s_modalPanel = vgui2::INVALID_PANEL;
static vgui2::VPANEL s_topmostPopup = vgui2::INVALID_PANEL;
static vgui2::VPANEL s_mouseFocus = vgui2::INVALID_PANEL;
static vgui2::VPANEL s_mouseCapture = vgui2::INVALID_PANEL;
static vgui2::VPANEL s_keyFocus = vgui2::INVALID_PANEL;
static vgui2::VPANEL s_appModal = vgui2::INVALID_PANEL;
static vgui2::VPANEL s_currentPanel = vgui2::INVALID_PANEL;
static vgui2::HCursor s_cursorOverride = 0;
static int s_nextTextureId = 1;
static int s_nextFontId = 1;
static int s_screenWide = 640;
static int s_screenTall = 480;
static int s_textColor[4] = { 255, 255, 255, 255 };
static int s_drawColor[4] = { 255, 255, 255, 255 };
static int s_textPos[2] = { 0, 0 };
static vgui2::HFont s_currentFont = 0;
static bool s_runtimeInstalled = false;
static cvar_s *s_cl_vgui2_skip_allocpanel = NULL;
static cvar_s *s_cl_vgui2_skip_ipanel_init = NULL;
static int s_cursorX = 0;
static int s_cursorY = 0;
static bool s_keyDown[vgui2::KEY_LAST + 1] = {};
static bool s_keyPressed[vgui2::KEY_LAST + 1] = {};
static bool s_keyReleased[vgui2::KEY_LAST + 1] = {};
static bool s_mouseDown[vgui2::MOUSE_LAST + 1] = {};
static bool s_mousePressed[vgui2::MOUSE_LAST + 1] = {};
static bool s_mouseReleased[vgui2::MOUSE_LAST + 1] = {};

static void ClearPerFrameInputState()
{
	memset(s_keyPressed, 0, sizeof(s_keyPressed));
	memset(s_keyReleased, 0, sizeof(s_keyReleased));
	memset(s_mousePressed, 0, sizeof(s_mousePressed));
	memset(s_mouseReleased, 0, sizeof(s_mouseReleased));
}

static void EnsureRuntimeCvars()
{
	if (!s_cl_vgui2_skip_allocpanel)
	{
		s_cl_vgui2_skip_allocpanel = gEngfuncs.pfnRegisterVariable("cl_vgui2_skip_allocpanel", "0", 0);
	}

	if (!s_cl_vgui2_skip_ipanel_init)
	{
		s_cl_vgui2_skip_ipanel_init = gEngfuncs.pfnRegisterVariable("cl_vgui2_skip_ipanel_init", "0", 0);
	}
}

static PanelData *GetPanelData(vgui2::VPANEL panel)
{
	if (panel == vgui2::INVALID_PANEL || panel == 0)
		return NULL;

	const size_t index = (size_t)panel - 1;
	if (index >= s_panels.size())
		return NULL;

	PanelData *data = &s_panels[index];
	return data->alive ? data : NULL;
}

static const PanelData *GetPanelDataConst(vgui2::VPANEL panel)
{
	return GetPanelData(panel);
}

static vgui2::VPANEL AllocPanelHandle()
{
	for (size_t i = 0; i < s_panels.size(); ++i)
	{
		if (!s_panels[i].alive)
		{
			s_panels[i] = PanelData();
			s_panels[i].alive = true;
			return (vgui2::VPANEL)(i + 1);
		}
	}

	s_panels.push_back(PanelData());
	s_panels.back().alive = true;
	return (vgui2::VPANEL)s_panels.size();
}

static void RemoveChildFromParent(PanelData *parent, vgui2::VPANEL child)
{
	if (!parent)
		return;

	parent->children.erase(
		std::remove(parent->children.begin(), parent->children.end(), child),
		parent->children.end());
}

static void SolvePanelRecursive(vgui2::VPANEL panel)
{
	PanelData *data = GetPanelData(panel);
	if (!data)
		return;

	if (data->parent != vgui2::INVALID_PANEL && data->parent != 0)
	{
		PanelData *parent = GetPanelData(data->parent);
		if (parent)
		{
			if (parent->needsSolve)
				SolvePanelRecursive(data->parent);

			data->absPos[0] = parent->absPos[0] + data->pos[0];
			data->absPos[1] = parent->absPos[1] + data->pos[1];
		}
		else
		{
			data->absPos[0] = data->pos[0];
			data->absPos[1] = data->pos[1];
		}
	}
	else
	{
		data->absPos[0] = data->pos[0];
		data->absPos[1] = data->pos[1];
	}

	data->needsSolve = false;

	for (size_t i = 0; i < data->children.size(); ++i)
	{
		PanelData *child = GetPanelData(data->children[i]);
		if (child)
			child->needsSolve = true;
	}
}

static bool IsVisibleRecursive(vgui2::VPANEL panel)
{
	const PanelData *data = GetPanelDataConst(panel);
	if (!data || !data->visible)
		return false;

	if (data->parent != vgui2::INVALID_PANEL && data->parent != 0)
		return IsVisibleRecursive(data->parent);

	return true;
}

static bool IsPointWithinPanel(vgui2::VPANEL panel, int x, int y)
{
	PanelData *data = GetPanelData(panel);
	if (!data || !IsVisibleRecursive(panel) || !data->enabled)
		return false;

	if (data->needsSolve)
		SolvePanelRecursive(panel);

	return x >= data->absPos[0] && y >= data->absPos[1] &&
		x < data->absPos[0] + data->size[0] &&
		y < data->absPos[1] + data->size[1];
}

static vgui2::VPANEL FindTopmostPanelAt(vgui2::VPANEL panel, int x, int y, bool traversePopups)
{
	PanelData *data = GetPanelData(panel);
	if (!data || !IsVisibleRecursive(panel) || !data->enabled)
		return vgui2::INVALID_PANEL;

	if (data->needsSolve)
		SolvePanelRecursive(panel);

	for (int i = (int)data->children.size() - 1; i >= 0; --i)
	{
		const vgui2::VPANEL child = data->children[i];
		PanelData *childData = GetPanelData(child);
		if (!childData)
			continue;
		if (!traversePopups && childData->popup)
			continue;

		vgui2::VPANEL hit = FindTopmostPanelAt(child, x, y, traversePopups);
		if (hit != vgui2::INVALID_PANEL && hit != 0)
			return hit;
	}

	return IsPointWithinPanel(panel, x, y) ? panel : vgui2::INVALID_PANEL;
}

static vgui2::Panel *GetPanelObject(vgui2::VPANEL panel)
{
	PanelData *data = GetPanelData(panel);
	if (!data || !data->clientPanel)
		return NULL;
	return data->clientPanel->GetPanel();
}

static void UpdateMouseFocusInternal(int x, int y)
{
	const vgui2::VPANEL newFocus = (s_embeddedPanel != vgui2::INVALID_PANEL && s_embeddedPanel != 0)
		? FindTopmostPanelAt(s_embeddedPanel, x, y, true)
		: vgui2::INVALID_PANEL;

	if (newFocus == s_mouseFocus)
	{
		vgui2::Panel *panel = GetPanelObject(newFocus);
		if (panel)
		{
			int absX = 0, absY = 0;
			g_pVGuiPanel->GetAbsPos(newFocus, absX, absY);
			panel->OnCursorMoved(x - absX, y - absY);
		}
		return;
	}

	vgui2::Panel *oldPanel = GetPanelObject(s_mouseFocus);
	if (oldPanel)
		oldPanel->OnCursorExited();

	s_mouseFocus = newFocus;

	vgui2::Panel *newPanel = GetPanelObject(newFocus);
	if (newPanel)
	{
		newPanel->OnCursorEntered();
		int absX = 0, absY = 0;
		g_pVGuiPanel->GetAbsPos(newFocus, absX, absY);
		newPanel->OnCursorMoved(x - absX, y - absY);
	}
}

static vgui2::KeyCode GoldSrcToVGuiKeyCode(int keynum)
{
	if (keynum >= '0' && keynum <= '9')
		return (vgui2::KeyCode)(vgui2::KEY_0 + (keynum - '0'));
	if (keynum >= 'a' && keynum <= 'z')
		return (vgui2::KeyCode)(vgui2::KEY_A + (keynum - 'a'));
	if (keynum >= 'A' && keynum <= 'Z')
		return (vgui2::KeyCode)(vgui2::KEY_A + (keynum - 'A'));

	switch (keynum)
	{
	case K_TAB: return vgui2::KEY_TAB;
	case K_ENTER: return vgui2::KEY_ENTER;
	case K_ESCAPE: return vgui2::KEY_ESCAPE;
	case K_SPACE: return vgui2::KEY_SPACE;
	case K_BACKSPACE: return vgui2::KEY_BACKSPACE;
	case K_UPARROW:
	case K_KP_UPARROW: return vgui2::KEY_UP;
	case K_DOWNARROW:
	case K_KP_DOWNARROW: return vgui2::KEY_DOWN;
	case K_LEFTARROW:
	case K_KP_LEFTARROW: return vgui2::KEY_LEFT;
	case K_RIGHTARROW:
	case K_KP_RIGHTARROW: return vgui2::KEY_RIGHT;
	case K_SHIFT: return vgui2::KEY_LSHIFT;
	case K_CTRL: return vgui2::KEY_LCONTROL;
	case K_ALT: return vgui2::KEY_LALT;
	case K_F1: return vgui2::KEY_F1;
	case K_F2: return vgui2::KEY_F2;
	case K_F3: return vgui2::KEY_F3;
	case K_F4: return vgui2::KEY_F4;
	case K_F5: return vgui2::KEY_F5;
	case K_F6: return vgui2::KEY_F6;
	case K_F7: return vgui2::KEY_F7;
	case K_F8: return vgui2::KEY_F8;
	case K_F9: return vgui2::KEY_F9;
	case K_F10: return vgui2::KEY_F10;
	case K_F11: return vgui2::KEY_F11;
	case K_F12: return vgui2::KEY_F12;
	default:
		return vgui2::BUTTON_CODE_INVALID;
	}
}

static vgui2::MouseCode GoldSrcToVGuiMouseCode(int keynum)
{
	switch (keynum)
	{
	case K_MOUSE1: return vgui2::MOUSE_LEFT;
	case K_MOUSE2: return vgui2::MOUSE_RIGHT;
	case K_MOUSE3: return vgui2::MOUSE_MIDDLE;
	case K_MOUSE4: return vgui2::MOUSE_4;
	case K_MOUSE5: return vgui2::MOUSE_5;
	default:
		return vgui2::MOUSE_LAST;
	}
}

static void DrawSetEngineTextColor()
{
	gEngfuncs.pfnDrawSetTextColor(
		s_textColor[0] / 255.0f,
		s_textColor[1] / 255.0f,
		s_textColor[2] / 255.0f);
}

class ClientVGui final : public vgui2::IVGui
{
public:
	bool Init(CreateInterfaceFn *, int) override { return true; }
	void Shutdown() override {}
	void Start() override {}
	void Stop() override {}
	bool IsRunning() override { return true; }

	void RunFrame() override
	{
		int mouseX = 0;
		int mouseY = 0;
		gEngfuncs.GetMousePosition(&mouseX, &mouseY);
		s_cursorX = mouseX;
		s_cursorY = mouseY;
		UpdateMouseFocusInternal(mouseX, mouseY);

		for (size_t i = 0; i < s_deleteQueue.size(); ++i)
		{
			const vgui2::VPANEL panel = s_deleteQueue[i];
			PanelData *data = GetPanelData(panel);
			if (!data)
				continue;

			if (data->parent != vgui2::INVALID_PANEL && data->parent != 0)
			{
				PanelData *parent = GetPanelData(data->parent);
				RemoveChildFromParent(parent, panel);
			}

			if (data->clientPanel)
			{
				g_pVGuiInput->PanelDeleted(panel);
				data->clientPanel = NULL;
			}

			*data = PanelData();
		}

		s_deleteQueue.clear();
	}

	void ShutdownMessage(unsigned int) override {}
	vgui2::VPANEL AllocPanel() override
	{
		EnsureRuntimeCvars();
		if (s_cl_vgui2_skip_allocpanel && s_cl_vgui2_skip_allocpanel->value != 0.0f)
		{
			gEngfuncs.Con_Printf("[VGUI2-RUNTIME] ClientVGui::AllocPanel SKIP -> INVALID_PANEL\n");
			return vgui2::INVALID_PANEL;
		}

		vgui2::VPANEL panel = AllocPanelHandle();
		gEngfuncs.Con_Printf("[VGUI2-RUNTIME] ClientVGui::AllocPanel -> %u\n", (unsigned int)panel);
		return panel;
	}

	void FreePanel(vgui2::VPANEL panel) override
	{
		PanelData *data = GetPanelData(panel);
		if (!data)
			return;

		if (data->parent != vgui2::INVALID_PANEL && data->parent != 0)
			RemoveChildFromParent(GetPanelData(data->parent), panel);

		*data = PanelData();
	}

	void DPrintf(const char *format, ...) override {}
	void DPrintf2(const char *format, ...) override {}
	void SpewAllActivePanelNames() override {}
	vgui2::HPanel PanelToHandle(vgui2::VPANEL panel) override { return (vgui2::HPanel)panel; }
	vgui2::VPANEL HandleToPanel(vgui2::HPanel index) override { return (vgui2::VPANEL)index; }
	void MarkPanelForDeletion(vgui2::VPANEL panel) override { s_deleteQueue.push_back(panel); }
	void AddTickSignal(vgui2::VPANEL, int) override {}
	void RemoveTickSignal(vgui2::VPANEL) override {}
	void PostMessage(vgui2::VPANEL target, KeyValues *params, vgui2::VPANEL from, float) override
	{
		if (g_pVGuiPanel)
			g_pVGuiPanel->SendMessage(target, params, from);
	}
	vgui2::HContext CreateContext() override { return vgui2::DEFAULT_VGUI_CONTEXT; }
	void DestroyContext(vgui2::HContext) override {}
	void AssociatePanelWithContext(vgui2::HContext, vgui2::VPANEL) override {}
	void ActivateContext(vgui2::HContext) override {}
	void SetSleep(bool) override {}
	bool GetShouldVGuiControlSleep() override { return false; }
};

class ClientPanel final : public vgui2::IPanel
{
public:
	void Init(vgui2::VPANEL vguiPanel, vgui2::IClientPanel *panel) override
	{
		EnsureRuntimeCvars();
		if (s_cl_vgui2_skip_ipanel_init && s_cl_vgui2_skip_ipanel_init->value != 0.0f)
		{
			gEngfuncs.Con_Printf("[VGUI2-RUNTIME] ClientPanel::Init SKIP by cvar handle=%u clientPanel=%p\n",
				(unsigned int)vguiPanel, panel);
			return;
		}

		if (vguiPanel == vgui2::INVALID_PANEL || vguiPanel == 0)
		{
			gEngfuncs.Con_Printf("[VGUI2-RUNTIME] ClientPanel::Init SKIP invalid handle=%u clientPanel=%p\n",
				(unsigned int)vguiPanel, panel);
			return;
		}

		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
			return;

		*data = PanelData();
		data->alive = true;
		data->clientPanel = panel;
	}

	void SetPos(vgui2::VPANEL vguiPanel, int x, int y) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
			return;
		data->pos[0] = x;
		data->pos[1] = y;
		data->needsSolve = true;
	}

	void GetPos(vgui2::VPANEL vguiPanel, int &x, int &y) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		x = data ? data->pos[0] : 0;
		y = data ? data->pos[1] : 0;
	}

	void SetSize(vgui2::VPANEL vguiPanel, int wide, int tall) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
			return;

		const bool changed = data->size[0] != wide || data->size[1] != tall;
		data->size[0] = wide;
		data->size[1] = tall;
		data->needsSolve = true;
		if (changed && data->clientPanel)
			data->clientPanel->OnSizeChanged(wide, tall);
	}

	void GetSize(vgui2::VPANEL vguiPanel, int &wide, int &tall) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		wide = data ? data->size[0] : 0;
		tall = data ? data->size[1] : 0;
	}

	void SetMinimumSize(vgui2::VPANEL, int, int) override {}
	void GetMinimumSize(vgui2::VPANEL, int &wide, int &tall) override { wide = tall = 0; }

	void SetZPos(vgui2::VPANEL vguiPanel, int z) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data)
			data->zpos = z;
	}

	int GetZPos(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->zpos : 0;
	}

	void GetAbsPos(vgui2::VPANEL vguiPanel, int &x, int &y) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
		{
			x = y = 0;
			return;
		}
		if (data->needsSolve)
			Solve(vguiPanel);
		x = data->absPos[0];
		y = data->absPos[1];
	}

	void GetClipRect(vgui2::VPANEL vguiPanel, int &x0, int &y0, int &x1, int &y1) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
		{
			x0 = y0 = x1 = y1 = 0;
			return;
		}

		if (!s_clipStack.empty())
		{
			const ClipRect &clip = s_clipStack.back();
			x0 = clip.x0;
			y0 = clip.y0;
			x1 = clip.x1;
			y1 = clip.y1;
			return;
		}

		if (data->needsSolve)
			Solve(vguiPanel);

		x0 = data->absPos[0];
		y0 = data->absPos[1];
		x1 = data->absPos[0] + data->size[0];
		y1 = data->absPos[1] + data->size[1];
	}

	void SetInset(vgui2::VPANEL vguiPanel, int left, int top, int right, int bottom) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
			return;
		data->inset[0] = left;
		data->inset[1] = top;
		data->inset[2] = right;
		data->inset[3] = bottom;
	}

	void GetInset(vgui2::VPANEL vguiPanel, int &left, int &top, int &right, int &bottom) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
		{
			left = top = right = bottom = 0;
			return;
		}
		left = data->inset[0];
		top = data->inset[1];
		right = data->inset[2];
		bottom = data->inset[3];
	}

	void SetVisible(vgui2::VPANEL vguiPanel, bool state) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data)
			data->visible = state;
	}

	bool IsVisible(vgui2::VPANEL vguiPanel) override { return IsVisibleRecursive(vguiPanel); }

	void SetParent(vgui2::VPANEL vguiPanel, vgui2::VPANEL newParent) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
			return;

		if (data->parent != vgui2::INVALID_PANEL && data->parent != 0)
			RemoveChildFromParent(GetPanelData(data->parent), vguiPanel);

		data->parent = newParent;
		data->needsSolve = true;

		PanelData *parent = GetPanelData(newParent);
		if (parent)
		{
			if (std::find(parent->children.begin(), parent->children.end(), vguiPanel) == parent->children.end())
				parent->children.push_back(vguiPanel);

			if (parent->clientPanel)
				parent->clientPanel->OnChildAdded(vguiPanel);
		}
	}

	int GetChildCount(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? (int)data->children.size() : 0;
	}

	vgui2::VPANEL GetChild(vgui2::VPANEL vguiPanel, int index) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data || index < 0 || index >= (int)data->children.size())
			return vgui2::INVALID_PANEL;
		return data->children[index];
	}

	vgui2::VPANEL GetParent(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->parent : vgui2::INVALID_PANEL;
	}

	void MoveToFront(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data || data->parent == vgui2::INVALID_PANEL || data->parent == 0)
			return;

		PanelData *parent = GetPanelData(data->parent);
		if (!parent)
			return;

		RemoveChildFromParent(parent, vguiPanel);
		parent->children.push_back(vguiPanel);
	}

	void MoveToBack(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data || data->parent == vgui2::INVALID_PANEL || data->parent == 0)
			return;

		PanelData *parent = GetPanelData(data->parent);
		if (!parent)
			return;

		RemoveChildFromParent(parent, vguiPanel);
		parent->children.insert(parent->children.begin(), vguiPanel);
	}

	bool HasParent(vgui2::VPANEL vguiPanel, vgui2::VPANEL potentialParent) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data)
			return false;
		if (data->parent == potentialParent)
			return true;
		if (data->parent == vgui2::INVALID_PANEL || data->parent == 0)
			return false;
		return HasParent(data->parent, potentialParent);
	}

	bool IsPopup(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->popup : false;
	}

	void SetPopup(vgui2::VPANEL vguiPanel, bool state) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data)
			data->popup = state;
	}

	bool Render_GetPopupVisible(vgui2::VPANEL) override { return true; }
	void Render_SetPopupVisible(vgui2::VPANEL, bool) override {}

	vgui2::HScheme GetScheme(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->GetScheme() : 0;
	}

	bool IsProportional(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->IsProportional() : false;
	}

	bool IsAutoDeleteSet(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->IsAutoDeleteSet() : false;
	}

	void DeletePanel(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->DeletePanel();
	}

	void SetKeyBoardInputEnabled(vgui2::VPANEL vguiPanel, bool state) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data)
			data->keyboardInputEnabled = state;
	}

	void SetMouseInputEnabled(vgui2::VPANEL vguiPanel, bool state) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data)
			data->mouseInputEnabled = state;
	}

	bool IsKeyBoardInputEnabled(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->keyboardInputEnabled : false;
	}

	bool IsMouseInputEnabled(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->mouseInputEnabled : false;
	}

	void Solve(vgui2::VPANEL vguiPanel) override { SolvePanelRecursive(vguiPanel); }

	const char *GetName(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->GetName() : "";
	}

	const char *GetClassName(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->GetClassName() : "";
	}

	void SendMessage(vgui2::VPANEL vguiPanel, KeyValues *params, vgui2::VPANEL ifromPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->OnMessage(params, ifromPanel);
	}

	void Think(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->Think();
	}

	void PerformApplySchemeSettings(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->PerformApplySchemeSettings();
	}

	void PaintTraverse(vgui2::VPANEL vguiPanel, bool forceRepaint, bool allowForce = true) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->PaintTraverse(forceRepaint, allowForce);
	}

	void Repaint(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->Repaint();
	}

	vgui2::VPANEL IsWithinTraverse(vgui2::VPANEL vguiPanel, int x, int y, bool traversePopups) override
	{
		return FindTopmostPanelAt(vguiPanel, x, y, traversePopups);
	}

	void OnChildAdded(vgui2::VPANEL vguiPanel, vgui2::VPANEL child) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->OnChildAdded(child);
	}

	void OnSizeChanged(vgui2::VPANEL vguiPanel, int newWide, int newTall) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->OnSizeChanged(newWide, newTall);
	}

	void InternalFocusChanged(vgui2::VPANEL vguiPanel, bool lost) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
			data->clientPanel->InternalFocusChanged(lost);
	}

	bool RequestInfo(vgui2::VPANEL vguiPanel, KeyValues *outputData) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->RequestInfo(outputData) : false;
	}

	void RequestFocus(vgui2::VPANEL vguiPanel, int direction = 0) override
	{
		if (s_keyFocus != vguiPanel)
		{
			PanelData *oldFocus = GetPanelData(s_keyFocus);
			if (oldFocus && oldFocus->clientPanel)
				oldFocus->clientPanel->InternalFocusChanged(true);
		}

		s_keyFocus = vguiPanel;
		PanelData *data = GetPanelData(vguiPanel);
		if (data && data->clientPanel)
		{
			data->clientPanel->InternalFocusChanged(false);
			data->clientPanel->RequestFocus(direction);
		}
	}

	bool RequestFocusPrev(vgui2::VPANEL vguiPanel, vgui2::VPANEL existingPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->RequestFocusPrev(existingPanel) : false;
	}

	bool RequestFocusNext(vgui2::VPANEL vguiPanel, vgui2::VPANEL existingPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->RequestFocusNext(existingPanel) : false;
	}

	vgui2::VPANEL GetCurrentKeyFocus(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->GetCurrentKeyFocus() : s_keyFocus;
	}

	int GetTabPosition(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->GetTabPosition() : 0;
	}

	vgui2::SurfacePlat *Plat(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->plat : NULL;
	}

	void SetPlat(vgui2::VPANEL vguiPanel, vgui2::SurfacePlat *plat) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data)
			data->plat = plat;
	}

	vgui2::Panel *GetPanel(vgui2::VPANEL vguiPanel, const char *destinationModule) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (!data || !data->clientPanel)
			return NULL;

		const char *moduleName = data->clientPanel->GetModuleName();
		if (destinationModule && destinationModule[0] && moduleName && moduleName[0] && stricmp(destinationModule, moduleName))
			return NULL;

		return (vgui2::Panel *)data->clientPanel->GetPanel();
	}

	bool IsEnabled(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->enabled : false;
	}

	void SetEnabled(vgui2::VPANEL vguiPanel, bool state) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		if (data)
			data->enabled = state;
	}

	vgui2::IClientPanel *Client(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return data ? data->clientPanel : NULL;
	}

	const char *GetModuleName(vgui2::VPANEL vguiPanel) override
	{
		PanelData *data = GetPanelData(vguiPanel);
		return (data && data->clientPanel) ? data->clientPanel->GetModuleName() : "";
	}
};

class ClientInputInternal final : public vgui2::IInputInternal
{
public:
	void SetMouseFocus(vgui2::VPANEL newMouseFocus) override { s_mouseFocus = newMouseFocus; }
	void SetMouseCapture(vgui2::VPANEL panel) override { s_mouseCapture = panel; }
	void GetKeyCodeText(vgui2::KeyCode code, char *buf, int buflen) override
	{
		if (buf && buflen > 0)
			snprintf(buf, buflen, "%d", (int)code);
	}
	vgui2::VPANEL GetFocus() override { return s_keyFocus; }
	vgui2::VPANEL GetMouseOver() override { return s_mouseFocus; }
	void SetCursorPos(int x, int y) override { s_cursorX = x; s_cursorY = y; }
	void GetCursorPos(int &x, int &y) override { x = s_cursorX; y = s_cursorY; }
	bool WasMousePressed(vgui2::MouseCode code) override { return code >= 0 && code <= vgui2::MOUSE_LAST ? s_mousePressed[code] : false; }
	bool WasMouseDoublePressed(vgui2::MouseCode) override { return false; }
	bool IsMouseDown(vgui2::MouseCode code) override { return code >= 0 && code <= vgui2::MOUSE_LAST ? s_mouseDown[code] : false; }
	void SetCursorOveride(vgui2::HCursor cursor) override { s_cursorOverride = cursor; }
	vgui2::HCursor GetCursorOveride() override { return s_cursorOverride; }
	bool WasMouseReleased(vgui2::MouseCode code) override { return code >= 0 && code <= vgui2::MOUSE_LAST ? s_mouseReleased[code] : false; }
	bool WasKeyPressed(vgui2::KeyCode code) override { return code >= 0 && code <= vgui2::KEY_LAST ? s_keyPressed[code] : false; }
	bool IsKeyDown(vgui2::KeyCode code) override { return code >= 0 && code <= vgui2::KEY_LAST ? s_keyDown[code] : false; }
	bool WasKeyTyped(vgui2::KeyCode code) override { return code >= 0 && code <= vgui2::KEY_LAST ? s_keyPressed[code] : false; }
	bool WasKeyReleased(vgui2::KeyCode code) override { return code >= 0 && code <= vgui2::KEY_LAST ? s_keyReleased[code] : false; }
	vgui2::VPANEL GetAppModalSurface() override { return s_appModal; }
	void SetAppModalSurface(vgui2::VPANEL panel) override { s_appModal = panel; }
	void ReleaseAppModalSurface() override { s_appModal = vgui2::INVALID_PANEL; }
	void GetCursorPosition(int &x, int &y) override { x = s_cursorX; y = s_cursorY; }

	void RunFrame() override {}
	void UpdateMouseFocus(int x, int y) override { UpdateMouseFocusInternal(x, y); }
	void PanelDeleted(vgui2::VPANEL panel) override
	{
		if (s_mouseFocus == panel) s_mouseFocus = vgui2::INVALID_PANEL;
		if (s_mouseCapture == panel) s_mouseCapture = vgui2::INVALID_PANEL;
		if (s_keyFocus == panel) s_keyFocus = vgui2::INVALID_PANEL;
		if (s_appModal == panel) s_appModal = vgui2::INVALID_PANEL;
	}
	void InternalCursorMoved(int x, int y) override
	{
		s_cursorX = x;
		s_cursorY = y;
		UpdateMouseFocusInternal(x, y);
	}
	void InternalMousePressed(vgui2::MouseCode code) override
	{
		if (code < 0 || code > vgui2::MOUSE_LAST)
			return;

		s_mouseDown[code] = true;
		s_mousePressed[code] = true;
		vgui2::VPANEL target = s_mouseCapture != vgui2::INVALID_PANEL && s_mouseCapture != 0 ? s_mouseCapture : s_mouseFocus;
		vgui2::Panel *panel = GetPanelObject(target);
		if (!panel)
			return;

		g_pVGuiPanel->RequestFocus(target);
		panel->OnMousePressed(code);
	}
	void InternalMouseDoublePressed(vgui2::MouseCode code) override
	{
		vgui2::Panel *panel = GetPanelObject(s_mouseCapture != vgui2::INVALID_PANEL && s_mouseCapture != 0 ? s_mouseCapture : s_mouseFocus);
		if (panel)
			panel->OnMouseDoublePressed(code);
	}
	void InternalMouseReleased(vgui2::MouseCode code) override
	{
		if (code < 0 || code > vgui2::MOUSE_LAST)
			return;

		s_mouseDown[code] = false;
		s_mouseReleased[code] = true;
		vgui2::VPANEL target = s_mouseCapture != vgui2::INVALID_PANEL && s_mouseCapture != 0 ? s_mouseCapture : s_mouseFocus;
		vgui2::Panel *panel = GetPanelObject(target);
		if (panel)
			panel->OnMouseReleased(code);
	}
	void InternalMouseWheeled(int) override {}
	void InternalKeyCodePressed(vgui2::KeyCode code) override
	{
		if (code < 0 || code > vgui2::KEY_LAST)
			return;

		s_keyDown[code] = true;
		s_keyPressed[code] = true;
		vgui2::Panel *panel = GetPanelObject(s_keyFocus);
		if (panel)
		{
			panel->OnKeyCodePressed(code);
			panel->OnKeyCodeTyped(code);
		}
	}
	void InternalKeyCodeTyped(vgui2::KeyCode code) override
	{
		vgui2::Panel *panel = GetPanelObject(s_keyFocus);
		if (panel)
			panel->OnKeyCodeTyped(code);
	}
	void InternalKeyTyped(wchar_t unichar) override
	{
		vgui2::Panel *panel = GetPanelObject(s_keyFocus);
		if (panel)
			panel->OnKeyTyped(unichar);
	}
	void InternalKeyCodeReleased(vgui2::KeyCode code) override
	{
		if (code < 0 || code > vgui2::KEY_LAST)
			return;

		s_keyDown[code] = false;
		s_keyReleased[code] = true;
		vgui2::Panel *panel = GetPanelObject(s_keyFocus);
		if (panel)
			panel->OnKeyCodeReleased(code);
	}
	vgui2::HInputContext CreateInputContext() override { return (vgui2::HInputContext)~0; }
	void DestroyInputContext(vgui2::HInputContext) override {}
	void AssociatePanelWithInputContext(vgui2::HInputContext, vgui2::VPANEL) override {}
	void ActivateInputContext(vgui2::HInputContext) override {}
	vgui2::VPANEL GetMouseCapture() override { return s_mouseCapture; }
	bool IsChildOfModalPanel(vgui2::VPANEL panel) override
	{
		if (s_appModal == vgui2::INVALID_PANEL || s_appModal == 0)
			return false;
		return g_pVGuiPanel ? g_pVGuiPanel->HasParent(panel, s_appModal) : false;
	}
	void ResetInputContext(vgui2::HInputContext) override {}

};

class ClientSurface final : public vgui2::ISurface
{
public:
	void Shutdown() override {}
	void RunFrame() override {}
	vgui2::VPANEL GetEmbeddedPanel() override { return s_embeddedPanel; }
	void SetEmbeddedPanel(vgui2::VPANEL pPanel) override { s_embeddedPanel = pPanel; }

	void PushMakeCurrent(vgui2::VPANEL panel, bool useInsets) override
	{
		PanelData *data = GetPanelData(panel);
		if (!data)
			return;

		if (data->needsSolve)
			g_pVGuiPanel->Solve(panel);

		ClipRect clip;
		clip.x0 = data->absPos[0] + (useInsets ? data->inset[0] : 0);
		clip.y0 = data->absPos[1] + (useInsets ? data->inset[1] : 0);
		clip.x1 = data->absPos[0] + data->size[0] - (useInsets ? data->inset[2] : 0);
		clip.y1 = data->absPos[1] + data->size[1] - (useInsets ? data->inset[3] : 0);
		s_clipStack.push_back(clip);
		s_currentPanel = panel;
	}

	void PopMakeCurrent(vgui2::VPANEL) override
	{
		if (!s_clipStack.empty())
			s_clipStack.pop_back();
		s_currentPanel = vgui2::INVALID_PANEL;
	}

	void DrawSetColor(int r, int g, int b, int a) override
	{
		s_drawColor[0] = r;
		s_drawColor[1] = g;
		s_drawColor[2] = b;
		s_drawColor[3] = a;
	}

	void DrawSetColor(Color col) override
	{
		DrawSetColor(col.r(), col.g(), col.b(), col.a());
	}

	void DrawFilledRect(int x0, int y0, int x1, int y1) override
	{
		gEngfuncs.pfnFillRGBA(x0, y0, x1 - x0, y1 - y0,
			s_drawColor[0], s_drawColor[1], s_drawColor[2], s_drawColor[3]);
	}

	void DrawOutlinedRect(int x0, int y0, int x1, int y1) override
	{
		DrawFilledRect(x0, y0, x1, y0 + 1);
		DrawFilledRect(x0, y1 - 1, x1, y1);
		DrawFilledRect(x0, y0, x0 + 1, y1);
		DrawFilledRect(x1 - 1, y0, x1, y1);
	}

	void DrawLine(int x0, int y0, int x1, int y1) override
	{
		if (x0 == x1)
			DrawFilledRect(x0, std::min(y0, y1), x0 + 1, std::max(y0, y1) + 1);
		else if (y0 == y1)
			DrawFilledRect(std::min(x0, x1), y0, std::max(x0, x1) + 1, y0 + 1);
	}

	void DrawPolyLine(int *px, int *py, int numPoints) override
	{
		for (int i = 1; i < numPoints; ++i)
			DrawLine(px[i - 1], py[i - 1], px[i], py[i]);
	}

	void DrawSetTextFont(vgui2::HFont font) override { s_currentFont = font; }

	void DrawSetTextColor(int r, int g, int b, int a) override
	{
		s_textColor[0] = r;
		s_textColor[1] = g;
		s_textColor[2] = b;
		s_textColor[3] = a;
	}

	void DrawSetTextColor(Color col) override
	{
		DrawSetTextColor(col.r(), col.g(), col.b(), col.a());
	}

	void DrawSetTextPos(int x, int y) override
	{
		s_textPos[0] = x;
		s_textPos[1] = y;
	}

	void DrawGetTextPos(int &x, int &y) override
	{
		x = s_textPos[0];
		y = s_textPos[1];
	}

	void DrawPrintText(const wchar_t *text, int textLen) override
	{
		if (!text)
			return;

		std::string ansi;
		for (int i = 0; i < textLen && text[i]; ++i)
			ansi.push_back(text[i] >= 0 && text[i] <= 0x7f ? (char)text[i] : '?');

		DrawSetEngineTextColor();
		int width = 0;
		int height = 0;
		gEngfuncs.pfnDrawConsoleStringLen(ansi.c_str(), &width, &height);
		gEngfuncs.pfnDrawConsoleString(s_textPos[0], s_textPos[1], (char *)ansi.c_str());
		s_textPos[0] += width;
	}

	void DrawUnicodeChar(wchar_t wch) override
	{
		DrawPrintText(&wch, 1);
	}

	void DrawUnicodeCharAdd(wchar_t wch) override
	{
		DrawUnicodeChar(wch);
	}

	void DrawFlushText() override {}
	vgui2::IHTML *CreateHTMLWindow(vgui2::IHTMLEvents *, vgui2::VPANEL) override { return NULL; }
	void PaintHTMLWindow(vgui2::IHTML *) override {}
	void DeleteHTMLWindow(vgui2::IHTML *) override {}
	void DrawSetTextureFile(int, const char *, int, bool) override {}
	void DrawSetTextureRGBA(int, const unsigned char *, int, int, int, bool) override {}
	void DrawSetTexture(int) override {}
	void DrawGetTextureSize(int, int &wide, int &tall) override { wide = tall = 0; }
	void DrawTexturedRect(int x0, int y0, int x1, int y1) override { DrawFilledRect(x0, y0, x1, y1); }
	bool IsTextureIDValid(int id) override { return id > 0 && id < s_nextTextureId; }
	int CreateNewTextureID(bool = false) override { return s_nextTextureId++; }

	void GetScreenSize(int &wide, int &tall) override
	{
		wide = s_screenWide;
		tall = s_screenTall;
	}

	void SetAsTopMost(vgui2::VPANEL panel, bool state) override
	{
		PanelData *data = GetPanelData(panel);
		if (data)
			data->topmost = state;
	}
	void BringToFront(vgui2::VPANEL panel) override { g_pVGuiPanel->MoveToFront(panel); }
	void SetForegroundWindow(vgui2::VPANEL panel) override { s_topmostPopup = panel; }
	void SetPanelVisible(vgui2::VPANEL panel, bool state) override { g_pVGuiPanel->SetVisible(panel, state); }
	void SetMinimized(vgui2::VPANEL panel, bool state) override
	{
		PanelData *data = GetPanelData(panel);
		if (data)
			data->minimized = state;
	}
	bool IsMinimized(vgui2::VPANEL panel) override
	{
		PanelData *data = GetPanelData(panel);
		return data ? data->minimized : false;
	}
	void FlashWindow(vgui2::VPANEL, bool) override {}
	void SetTitle(vgui2::VPANEL, const wchar_t *) override {}
	void SetAsToolBar(vgui2::VPANEL, bool) override {}

	void CreatePopup(vgui2::VPANEL panel, bool, bool, bool, bool, bool) override
	{
		if (panel != vgui2::INVALID_PANEL && s_embeddedPanel != vgui2::INVALID_PANEL)
			g_pVGuiPanel->SetParent(panel, s_embeddedPanel);
	}
	void SwapBuffers(vgui2::VPANEL) override {}
	void Invalidate(vgui2::VPANEL) override {}
	void SetCursor(vgui2::HCursor cursor) override { s_cursorOverride = cursor; }
	bool IsCursorVisible() override { return true; }
	void ApplyChanges() override {}
	bool IsWithin(int, int) override { return true; }
	bool HasFocus() override { return true; }

	bool SupportsFeature(SurfaceFeature_e feature) override
	{
		return feature == ESCAPE_KEY || feature == FRAME_MINIMIZE_MAXIMIZE;
	}

	void RestrictPaintToSinglePanel(vgui2::VPANEL) override {}
	void SetModalPanel(vgui2::VPANEL panel) override { s_modalPanel = panel; }
	vgui2::VPANEL GetModalPanel() override { return s_modalPanel; }
	void UnlockCursor() override {}
	void LockCursor() override {}
	void SetTranslateExtendedKeys(bool) override {}
	vgui2::VPANEL GetTopmostPopup() override { return s_topmostPopup; }
	void SetTopLevelFocus(vgui2::VPANEL panel) override { s_keyFocus = panel; }

	vgui2::HFont CreateFont() override
	{
		const int id = s_nextFontId++;
		s_fonts[id] = FontData();
		s_fonts[id].valid = true;
		return (vgui2::HFont)id;
	}

	bool AddGlyphSetToFont(vgui2::HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange) override
	{
		FontData &f = s_fonts[(int)font];
		f.valid = true;
		f.name = windowsFontName ? windowsFontName : "";
		f.tall = tall;
		f.weight = weight;
		f.blur = blur;
		f.scanlines = scanlines;
		f.flags = flags;
		f.lowRange = lowRange;
		f.highRange = highRange;
		f.charWidth = std::max(6, tall / 2);
		return true;
	}

	bool AddCustomFontFile(const char *) override { return true; }
	int GetFontTall(vgui2::HFont font) override
	{
		std::map<int, FontData>::iterator it = s_fonts.find((int)font);
		return it != s_fonts.end() ? it->second.tall : 13;
	}

	void GetCharABCwide(vgui2::HFont font, int, int &a, int &b, int &c) override
	{
		std::map<int, FontData>::iterator it = s_fonts.find((int)font);
		a = 0;
		b = it != s_fonts.end() ? it->second.charWidth : 8;
		c = 0;
	}

	int GetCharacterWidth(vgui2::HFont font, int) override
	{
		std::map<int, FontData>::iterator it = s_fonts.find((int)font);
		return it != s_fonts.end() ? it->second.charWidth : 8;
	}

	void GetTextSize(vgui2::HFont font, const wchar_t *text, int &wide, int &tall) override
	{
		int len = 0;
		while (text && text[len])
			++len;
		wide = len * GetCharacterWidth(font, 0);
		tall = GetFontTall(font);
	}

	vgui2::VPANEL GetNotifyPanel() override { return vgui2::INVALID_PANEL; }
	void SetNotifyIcon(vgui2::VPANEL, vgui2::HTexture, vgui2::VPANEL, const char *) override {}
	void PlaySound(const char *) override {}
	int GetPopupCount() override { return 0; }
	vgui2::VPANEL GetPopup(int) override { return vgui2::INVALID_PANEL; }
	bool ShouldPaintChildPanel(vgui2::VPANEL childPanel) override { return g_pVGuiPanel->IsVisible(childPanel); }
	bool RecreateContext(vgui2::VPANEL) override { return false; }
	void AddPanel(vgui2::VPANEL) override {}
	void ReleasePanel(vgui2::VPANEL) override {}
	void MovePopupToFront(vgui2::VPANEL panel) override { s_topmostPopup = panel; }
	void MovePopupToBack(vgui2::VPANEL) override {}

	void SolveTraverse(vgui2::VPANEL panel, bool = false) override
	{
		if (panel == vgui2::INVALID_PANEL || panel == 0)
			return;

		g_pVGuiPanel->Solve(panel);
		const int childCount = g_pVGuiPanel->GetChildCount(panel);
		for (int i = 0; i < childCount; ++i)
			SolveTraverse(g_pVGuiPanel->GetChild(panel, i), false);
	}

	void PaintTraverse(vgui2::VPANEL panel) override
	{
		if (panel != vgui2::INVALID_PANEL && panel != 0)
			g_pVGuiPanel->PaintTraverse(panel, true, true);
	}

	void EnableMouseCapture(vgui2::VPANEL panel, bool state) override
	{
		s_mouseCapture = state ? panel : vgui2::INVALID_PANEL;
	}
	void GetWorkspaceBounds(int &x, int &y, int &wide, int &tall) override { x = y = 0; wide = s_screenWide; tall = s_screenTall; }
	void GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall) override { x = y = 0; wide = s_screenWide; tall = s_screenTall; }
	void GetProportionalBase(int &width, int &height) override { width = 640; height = 480; }
	void CalculateMouseVisible() override {}
	bool NeedKBInput() override { return s_keyFocus != vgui2::INVALID_PANEL && s_keyFocus != 0; }
	bool HasCursorPosFunctions() override { return true; }
	void SurfaceGetCursorPos(int &x, int &y) override { x = 0; y = 0; }
	void SurfaceSetCursorPos(int, int) override {}
	void DrawTexturedPolygon(vgui2::VGuiVertex *, int) override {}
	int GetFontAscent(vgui2::HFont font, wchar_t) override { return std::max(0, GetFontTall(font) - 2); }
	void SetAllowHTMLJavaScript(bool) override {}
	void SetLanguage(const char *pchLang) override { m_language = pchLang ? pchLang : "english"; }
	const char *GetLanguage() override { return m_language.c_str(); }
	bool DeleteTextureByID(int) override { return false; }
	void DrawUpdateRegionTextureBGRA(int, int, int, const unsigned char *, int, int) override {}
	void DrawSetTextureBGRA(int, const unsigned char *, int, int) override {}
	void CreateBrowser(vgui2::VPANEL, IHTMLResponses *, bool, const char *) override {}
	void RemoveBrowser(vgui2::VPANEL, IHTMLResponses *) override {}
	IHTMLChromeController *AccessChromeHTMLController() override { return NULL; }
	void DrawTexturedRectAdd(int x0, int y0, int x1, int y1) override { DrawTexturedRect(x0, y0, x1, y1); }
	void SetSupportsEsc(bool) override {}
	int GetFontBlur(vgui2::HFont font) override
	{
		std::map<int, FontData>::iterator it = s_fonts.find((int)font);
		return it != s_fonts.end() ? it->second.blur : 0;
	}
	bool IsAdditive(vgui2::HFont font) override
	{
		std::map<int, FontData>::iterator it = s_fonts.find((int)font);
		return it != s_fonts.end() ? (it->second.flags & vgui2::ISurface::FONTFLAG_ADDITIVE) != 0 : false;
	}
	void SetProportionalBase(int, int) override {}
	void GetHDProportionalBase(int &width, int &height) override { width = 640; height = 480; }
	void SetHDProportionalBase(int, int) override {}

private:
	std::string m_language = "english";
};

static ClientVGui s_vgui;
static ClientPanel s_panel;
static ClientInputInternal s_input;
static ClientSurface s_surface;

} // namespace

extern IFileSystem *g_pFullFileSystem;
extern vgui2::ISurface *g_pVGuiSurface;
extern vgui2::IInputInternal *g_pVGuiInput;
extern vgui2::IVGui *g_pVGui;
extern vgui2::IPanel *g_pVGuiPanel;

bool VGUI2_ClientRuntimeInstall()
{
	if (s_runtimeInstalled)
		return true;

	EnsureRuntimeCvars();

	g_pVGui = &s_vgui;
	g_pVGuiPanel = &s_panel;
	g_pVGuiSurface = &s_surface;
	g_pVGuiInput = &s_input;

	s_runtimeInstalled = true;
	gEngfuncs.Con_Printf("[VGUI2-RUNTIME] Installed client-owned IVGui/IPanel/ISurface/IInput runtime\n");
	return true;
}

void VGUI2_ClientRuntimeShutdown()
{
	s_deleteQueue.clear();
	s_clipStack.clear();
	s_fonts.clear();
	s_panels.clear();
	s_embeddedPanel = vgui2::INVALID_PANEL;
	s_modalPanel = vgui2::INVALID_PANEL;
	s_topmostPopup = vgui2::INVALID_PANEL;
	s_mouseFocus = vgui2::INVALID_PANEL;
	s_mouseCapture = vgui2::INVALID_PANEL;
	s_keyFocus = vgui2::INVALID_PANEL;
	s_appModal = vgui2::INVALID_PANEL;
	s_currentPanel = vgui2::INVALID_PANEL;
	s_runtimeInstalled = false;
}

void VGUI2_ClientRuntimeOnVidInit(int width, int height)
{
	s_screenWide = width > 0 ? width : 640;
	s_screenTall = height > 0 ? height : 480;

	if (s_embeddedPanel == vgui2::INVALID_PANEL || s_embeddedPanel == 0)
	{
		s_embeddedPanel = s_vgui.AllocPanel();
		s_panel.Init(s_embeddedPanel, NULL);
		s_panel.SetPos(s_embeddedPanel, 0, 0);
		s_panel.SetSize(s_embeddedPanel, s_screenWide, s_screenTall);
		s_panel.SetVisible(s_embeddedPanel, true);
		s_panel.SetParent(s_embeddedPanel, 0);
		s_surface.SetEmbeddedPanel(s_embeddedPanel);
		gEngfuncs.Con_Printf("[VGUI2-RUNTIME] Created client-owned root panel=%u size=%dx%d\n",
			(unsigned int)s_embeddedPanel, s_screenWide, s_screenTall);
	}
	else
	{
		s_panel.SetPos(s_embeddedPanel, 0, 0);
		s_panel.SetSize(s_embeddedPanel, s_screenWide, s_screenTall);
	}
}

void VGUI2_ClientRuntimeRunFrame()
{
	if (!s_runtimeInstalled)
		return;

	s_vgui.RunFrame();
	s_input.RunFrame();

	if (s_embeddedPanel != vgui2::INVALID_PANEL && s_embeddedPanel != 0)
	{
		s_surface.SolveTraverse(s_embeddedPanel, false);
		s_surface.PaintTraverse(s_embeddedPanel);
	}

	ClearPerFrameInputState();
}

bool VGUI2_ClientRuntimeHandleKeyEvent(int down, int keynum)
{
	if (!s_runtimeInstalled)
		return false;

	const vgui2::MouseCode mouseCode = GoldSrcToVGuiMouseCode(keynum);
	if (mouseCode != vgui2::MOUSE_LAST)
	{
		if (down)
			s_input.InternalMousePressed(mouseCode);
		else
			s_input.InternalMouseReleased(mouseCode);
		return true;
	}

	const vgui2::KeyCode code = GoldSrcToVGuiKeyCode(keynum);
	if (code == vgui2::BUTTON_CODE_INVALID)
		return false;

	if (down)
	{
		s_input.InternalKeyCodePressed(code);

		if (keynum >= 32 && keynum < 127)
			s_input.InternalKeyTyped((wchar_t)keynum);
	}
	else
	{
		s_input.InternalKeyCodeReleased(code);
	}

	return true;
}

#endif
