

### End to End implementation 


```

---
END-TO-END FLOW: Engine → TeamMenu
PHASE 1: VGUI2 Initialization (vgui2_support) (xash3d-fwgs)
Step 1 - Engine calls InitAPI() (vgui_int.cpp:126-135)
Engine
  → EXPORT InitAPI(vguiapi_t *api)    [vgui_int.cpp:126]
      → Sets g_api->Startup = VGui_Startup
      → Sets g_api->Shutdown, Paint, Mouse, Key, MouseMove
Step 2 - Engine calls g_api->Startup() (vgui_int.cpp:27-31)
Engine → g_api->Startup(VGui_Startup)
  → void VGui_Startup(int width, int height)    [vgui_int.cpp:27]
      → VGuiWrap2_Startup();                    [vgui_int.cpp:31]
Step 3 - VGuiWrap2_Startup() calls CBaseUI::Initialize() (vgui2_int.cpp:88-99)
VGuiWrap2_Startup()    [vgui2_int.cpp:88]
  → RegisterInterface();                        [vgui2_int.cpp:92]
  → RegisterControls();                         [vgui2_int.cpp:93]
  → CreateInterface("BaseUI") → CBaseUI
  → staticUIFuncs->Initialize(&pEngineFactory, 1);   [vgui2_int.cpp:97] ← CBaseUI::Initialize()
  → staticUIFuncs->Start(NULL, 0);                    [vgui2_int.cpp:98]
---
PHASE 2: CHudViewport Creation via CBaseUI::Initialize/Start
Step 4 - CBaseUI::Initialize() (CBaseUI.cpp:105-161)
CBaseUI::Initialize(CreateInterfaceFn* factories, int count)    [CBaseUI.cpp:105]
  → VGuiControls_Init("BaseUI", m_FactoryList, m_iNumFactories);  [CBaseUI.cpp:127]
  → Gets IClientVGUI via factory[4]("VClientVGUI001")            [CBaseUI.cpp:159]
  → staticClient->Initialize(m_FactoryList, m_iNumFactories);    [CBaseUI.cpp:251]
Step 5 - CClientVGUI::Initialize() (CClientVGUI.cpp:59-96)
CClientVGUI::Initialize()    [CClientVGUI.cpp:59]
  → new CHudViewport();      [CClientVGUI.cpp:93] ← CHudViewport/CBaseViewport constructed HERE
  → g_pViewport->Initialize(pFactories, iNumFactories);  [CClientVGUI.cpp:95]
Step 6 - CHudViewport constructor → CBaseViewport constructor (CBaseViewport.cpp:23-46)
CBaseViewport::CBaseViewport()    [CBaseViewport.cpp:23]
  → g_pViewport = this;
  → SetScheme("ClientScheme");
  → m_pAnimController = new AnimationController(this);
Step 7 - CBaseUI::Start() (CBaseUI.cpp:163-287)
CBaseUI::Start()    [CBaseUI.cpp:163]
  → staticClientDLLPanel = new CEnginePanel()     [CBaseUI.cpp:207]
  → staticClient->Initialize(m_FactoryList, ...); [CBaseUI.cpp:251]
  → staticClient->Start();                       [CBaseUI.cpp:264]
  → staticClient->SetParent(...);                 [CBaseUI.cpp:265]
---
PHASE 3: TeamMenu Panel Created via CHudViewport::CreateDefaultPanels
Step 8 - CClientVGUI::Start() (CClientVGUI.cpp:98-100)
CClientVGUI::Start()
  → g_pViewport->Start();    [CClientVGUI.cpp:100]
Step 9 - CBaseViewport::Start() (CBaseViewport.cpp:64-77)
CBaseViewport::Start()    [CBaseViewport.cpp:64]
  → RemoveAllPanels();
  → m_pBackGround = new CBackGroundPanel(nullptr);    [CBaseViewport.cpp:69]
  → CreateDefaultPanels();   [CBaseViewport.cpp:74] ← virtual
Step 10 - CHudViewport::CreateDefaultPanels() (CHudViewPort.cpp:82-91)
CHudViewport::CreateDefaultPanels()    [CHudViewPort.cpp:82]
  → AddNewPanel(CreatePanelByName("ClientMOTD"));
  → AddNewPanel(CreatePanelByName(PANEL_TEAM));    [CHudViewPort.cpp:85]
  → AddNewPanel(CreatePanelByName(PANEL_CLASS));
  → AddNewPanel(CreatePanelByName(PANEL_BUY));
Step 11 - CHudViewport::CreatePanelByName(PANEL_TEAM) (CHudViewPort.cpp:103-110)
CHudViewport::CreatePanelByName("TeamMenu")    [CHudViewPort.cpp:103]
  → if (!m_pTeamMenu)
        m_pTeamMenu = new CCSTeamMenu(this);   [CHudViewPort.cpp:107]
        m_pTeamMenu->UpdateGameMode();
Step 12 - CCSTeamMenu → CTeamMenu constructors (cstriketeammenu.cpp:29, teammenu.cpp:38)
CCSTeamMenu::CCSTeamMenu(IViewport* pViewPort)
  : CTeamMenu(pViewPort)    [cstriketeammenu.cpp:29]
CTeamMenu::CTeamMenu(IViewport* pViewPort)
  : Frame(NULL, PANEL_TEAM)    [teammenu.cpp:38]
  → SetTitle(), SetScheme(), SetMoveable(false)
  → m_pPanel = new EditablePanel(this, "ClassInfo");
  → m_pMapInfo = new RichText(this, "MapInfo");
  → LoadControlSettings("Resource/UI/TeamMenu.res", "GAME");    [teammenu.cpp:53]
---
PHASE 4: TeamMenu Shown (Trigger)
Step 13 - Engine or game sends team menu message
Engine/Server → HUD_Message or VGUIMenu → CHudMenu::ShowVGUIMenu(MENU_TEAM)
Step 14 - CHudMenu::ShowVGUIMenu() (hud_menu.cpp:299-320)
CHudMenu::ShowVGUIMenu(int menuType)    [hud_menu.cpp:299]
  → case MENU_TEAM:
        if(g_pViewport->ShowVGUIMenu(menuType))   [hud_menu.cpp:316]
            return;
Step 15 - CHudViewport::ShowVGUIMenu() (CHudViewPort.cpp:164-252)
CHudViewport::ShowVGUIMenu(MENU_TEAM)    [CHudViewPort.cpp:164]
  → panel = m_pTeamMenu;    [CHudViewPort.cpp:180]
  → ShowPanel(panel, true); [CHudViewPort.cpp:247]
Step 16 - CBaseViewport::ShowPanel(IViewportPanel*, bool) (CBaseViewport.cpp:350-395)
CBaseViewport::ShowPanel(IViewportPanel* pPanel, bool bState)    [CBaseViewport.cpp:350]
  → if (bState && pPanel->HasInputElements())
        m_pActivePanel = pPanel;
  → pPanel->ShowPanel(bState);   [CBaseViewport.cpp:392]
  → UpdateAllPanels();
Step 17 - CCSTeamMenu::ShowPanel() → CTeamMenu::ShowPanel() (cstriketeammenu.cpp:44-53, teammenu.cpp:98-123)
CCSTeamMenu::ShowPanel(bool bShow)    [cstriketeammenu.cpp:44]
  → if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly()) return;
  → BaseClass::ShowPanel(bShow);
CTeamMenu::ShowPanel(bool bShow)    [teammenu.cpp:98]
  → Activate();              [teammenu.cpp:105] ← Frame::Activate()
  → SetMouseInputEnabled(true);
  → for mouseover buttons: ShowPage/HidePage
  → m_pViewPort->ShowBackGround(bShow);
Step 18 - vgui2::Frame::Activate() (vgui2_support/vgui_controls/Frame.cpp:1005-1019)
Frame::Activate()    [Frame.cpp:1005]
  → MoveToFront();
  → RequestFocus();
  → SetVisible(true);
  → SetEnabled(true);
---
PHASE 5: TeamMenu Rendered (Per Frame)
Step 19 - Engine calls CBaseUI::Paint() per frame (CBaseUI.cpp:363-378)
CBaseUI::Paint(int x, int y, int right, int bottom)    [CBaseUI.cpp:363]
  → vgui2::ivgui()->RunFrame();         [CBaseUI.cpp:369]
  → staticSurface->PaintTraverse(embeddedPanel);   [CBaseUI.cpp:377]
Step 20 - VGUI2 paint traverse calls CBaseViewport::OnThink() (CBaseViewport.cpp:160-205)
CBaseViewport::OnThink()    [CBaseViewport.cpp:160]
  → for each panel in m_Panels:
        if (panel->NeedsUpdate() && panel->IsVisible())
            panel->Update();    [CBaseViewport.cpp:190]
  → BaseClass::OnThink();
Step 21 - CCSTeamMenu::Update() (cstriketeammenu.cpp:55-86)
CCSTeamMenu::Update()    [cstriketeammenu.cpp:55]
  → BaseClass::Update();  // CTeamMenu::Update()
      → LoadMapPage(mapname)
  → Update button visibility (specbutton, vipbutton, CancelButton)
Step 22 - VGUI2 paint traverse to TeamMenu (Frame.cpp:1662-1761)
PaintTraverse → Frame::PaintBackground()    [Frame.cpp:1662]
  → BaseClass::PaintBackground();   // EditablePanel
  → if (_drawTitleBar) Draw title bar
  → RichText, buttons, child controls paint themselves
---
Complete Call Chain Summary
Engine
  │
  ├─► InitAPI()                              [vgui_int.cpp:126]
  │       └─► Sets g_api->Startup = VGui_Startup
  │
  ├─► g_api->Startup()                       [called via function pointer]
  │       └─► VGui_Startup()                 [vgui_int.cpp:27]
  │               └─► VGuiWrap2_Startup()   [vgui_int.cpp:31]
  │                       └─► CBaseUI::Initialize()    [vgui2_int.cpp:97]
  │                               └─► CClientVGUI::Initialize()    [CBaseUI.cpp:251]
  │                                       └─► new CHudViewport()    [CClientVGUI.cpp:93]
  │                                               └─► CBaseViewport::CBaseViewport()
  │
  ├─► g_api->Paint() per frame               [CBaseUI.cpp:363]
  │       └─► ivgui()->RunFrame()
  │       └─► surface()->PaintTraverse(embeddedPanel)
  │               └─► CBaseViewport::OnThink()    [CBaseViewport.cpp:160]
  │                       └─► TeamMenu::Update()
  │               └─► Frame::PaintBackground()    [Frame.cpp:1662]
  │                       └─► TeamMenu paints its controls
  │
  └─► TeamMenu shown via game event → ShowVGUIMenu(MENU_TEAM)
          └─► CHudViewport::ShowVGUIMenu()    [CHudViewPort.cpp:164]
                  └─► CBaseViewport::ShowPanel()    [CBaseViewport.cpp:350]
                          └─► CTeamMenu::ShowPanel()    [teammenu.cpp:98]
                                  └─► Frame::Activate()    [Frame.cpp:1005]
```






### Calling the Teammenu Panel 
So the full chain is:
1. Server sends "VGUIMenu" message with menu type
2. Engine receives network packet
3. Engine looks up hooked messages
4. Engine calls MsgFunc_VGUIMenu
5. Which calls ShowVGUIMenu(menuType)
6. Which calls g_pViewport->ShowVGUIMenu(MENU_TEAM)
7. Which calls ShowPanel(panel, true)
8. Which calls CTeamMenu::ShowPanel(true)
9. Which calls Frame::Activate()
10. Which sets visible, moves to front
11. Per frame rendering paints it


Here's the complete trigger chain:
How CTeamMenu::ShowPanel Gets Triggered
---
Step 1: Server Sends VGUIMenu Message
Server sends a network message named "VGUIMenu" with a byte indicating menu type (e.g., MENU_TEAM = 2)
---
Step 2: Engine Receives Network Message → Dispatches to Client
The engine (via pfnHookUserMsg) has registered a callback for "VGUIMenu":
cl_util.h:35
#define HOOK_MESSAGE(x) gEngfuncs.pfnHookUserMsg(#x, __MsgFunc_##x );
So HOOK_MESSAGE(VGUIMenu) becomes:
gEngfuncs.pfnHookUserMsg("VGUIMenu", __MsgFunc_VGUIMenu);
---
Step 3: MsgFunc_VGUIMenu Called
hud_menu.cpp:253-261
int CHudMenu::MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
    BufferReader reader(pszName, pbuf, iSize);
    int menuType = reader.ReadByte();  // e.g., MENU_TEAM = 2
    m_bitsValidSlots = reader.ReadShort();
    ShowVGUIMenu(menuType);  // ← Step 4
    return 1;
}
---
Step 4: ShowVGUIMenu Called
hud_menu.cpp:306-319 (inside ShowVGUIMenu() function)
case MENU_TEAM:
    if(g_pViewport->ShowVGUIMenu(menuType))  // ← Step 5
        return;
---
Step 5: CHudViewport::ShowVGUIMenu
CHudViewPort.cpp:170-182
case MENU_TEAM:
    if (m_pClassMenu->CheckShowType())
        panel = m_pClassMenu;
    else
        panel = m_pTeamMenu;    // ← CCSTeamMenu
    break;
// ...
if (panel)
    ShowPanel(panel, true);  // ← Step 6
---
Step 6: ShowPanel Called on TeamMenu
CBaseViewport.cpp:350-395
pPanel->ShowPanel(bState);  // ← calls CCSTeamMenu::ShowPanel
---
Step 7: CCSTeamMenu::ShowPanel
cstriketeammenu.cpp:44-53
void CCSTeamMenu::ShowPanel(bool bShow)
{
    if (bShow)
    {
        if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
            return;  // Blocked during intermission
    }
    BaseClass::ShowPanel(bShow);  // ← Calls CTeamMenu::ShowPanel
}
---
Step 8: CTeamMenu::ShowPanel
teammenu.cpp:98-123
void CTeamMenu::ShowPanel(bool bShow)
{
    if (bShow)
    {
        Activate();              // ← Frame::Activate() - Step 9
        SetMouseInputEnabled(true);
        // ... button handling ...
    }
    else
    {
        SetVisible(false);
        SetMouseInputEnabled(false);
    }
}
---
Step 9: Frame::Activate (Popup Becomes Visible)
Frame.cpp:1005-1019
void Frame::Activate()
{
    MoveToFront();       // Reorder popup to front
    RequestFocus();       // Keyboard focus
    SetVisible(true);    // Visible flag set
    SetEnabled(true);
}
---
COMPLETE TRIGGER CHAIN
Server ──► Network Message "VGUIMenu" ──► Engine
                                          │
                                          ▼
                              gEngfuncs.pfnHookUserMsg()
                              (registered callback)
                                          │
                                          ▼
                              CHudMenu::MsgFunc_VGUIMenu()
                                          │
                                          ▼
                              ShowVGUIMenu(MENU_TEAM)
                                          │
                                          ▼
                              g_pViewport->ShowVGUIMenu(MENU_TEAM)
                                          │
                                          ▼
                              CHudViewport::ShowVGUIMenu()
                                          │
                                          ▼
                              panel = m_pTeamMenu (CCSTeamMenu)
                              ShowPanel(panel, true)
                                          │
                                          ▼
                              CBaseViewport::ShowPanel()
                                          │
                                          ▼
                              CCSTeamMenu::ShowPanel()
                                          │
                                          ▼
                              CTeamMenu::ShowPanel()
                                          │
                                          ▼
                              Frame::Activate()
                                  MoveToFront()  // popup list reorder
                                  SetVisible(true)  // visibility flag
Per Frame Rendering Then Paints It:
PaintTraverse() → iterates staticPopupList → TeamMenu::PaintBackground()
