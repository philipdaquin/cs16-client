#pragma once

#if defined(__arm64__) || defined(__aarch64__) || defined(__ARM64_ARCH_8__) || defined(ARM64)

#define VGUI2_STUB_MODE 1

class IBaseInterface;
typedef IBaseInterface *(*CreateInterfaceFn)(const char *, int *);

namespace vgui2
{
typedef unsigned int VPANEL;
typedef unsigned long HScheme;
typedef unsigned long HFont;
typedef unsigned long HPanel;
typedef unsigned long HTexture;
typedef unsigned long HCursor;
const HPanel INVALID_PANEL = 0xffffffff;
const HFont INVALID_FONT = 0;

class ISurface { public: virtual ~ISurface() {} };
class IVGui { public: virtual ~IVGui() {} };
class IPanel { public: virtual ~IPanel() {} };
class ISchemeManager { public: virtual ~ISchemeManager() {} };
class ISystem { public: virtual ~ISystem() {} };
class IInputInternal { public: virtual ~IInputInternal() {} };
class ILocalize { public: virtual ~ILocalize() {} };

extern ISurface *Stub_g_pVGuiSurface;
extern IInputInternal *Stub_g_pVGuiInput;
extern IVGui *Stub_g_pVGui;
extern IPanel *Stub_g_pVGuiPanel;
extern ILocalize *Stub_g_pVGuiLocalize;
extern ISchemeManager *Stub_g_pVGuiSchemeManager;
extern ISystem *Stub_g_pVGuiSystem;
}

bool Stub_ConnectTier1Libraries(CreateInterfaceFn *, int);
bool Stub_ConnectTier2Libraries(CreateInterfaceFn *, int);
bool Stub_VGui_InitInterfacesList(const char *moduleName, CreateInterfaceFn *, int);

#define ConnectTier1Libraries Stub_ConnectTier1Libraries
#define ConnectTier2Libraries Stub_ConnectTier2Libraries
#define VGui_InitInterfacesList Stub_VGui_InitInterfacesList
#define g_pVGui Stub_g_pVGui
#define g_pVGuiPanel Stub_g_pVGuiPanel
#define g_pVGuiSurface Stub_g_pVGuiSurface
#define g_pVGuiSchemeManager Stub_g_pVGuiSchemeManager
#define g_pVGuiSystem Stub_g_pVGuiSystem
#define g_pVGuiInput Stub_g_pVGuiInput
#define g_pVGuiLocalize Stub_g_pVGuiLocalize

#else

#include <vgui_controls/Controls.h>
#include <tier1/tier1.h>
#include <tier2/tier2.h>

#endif