#pragma once

void VGui_Startup();
void VGui_Shutdown();
void VGui_ShowPanel(int menuType, bool show);
void VGui_HidePanel(int menuType);
bool VGui_IsAnyPanelVisible();
bool VGui_IsPanelVisible(int menuType);
bool VGui_IsSpectatorGUIVisible();
void VGui_Reset();
