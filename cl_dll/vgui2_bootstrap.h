#pragma once

bool VGUI2_Bootstrap();
void VGUI2_OnShutdown();
bool VGUI2_IsReady();
bool VGUI2_HasScheme();
void VGUI2_OnVidInit();
void VGUI2_RunFrame();
void VGUI2_CreateTestPanel();
void VGUI2_DestroyTestPanel();

#if !defined(VGUI2_STUB_MODE)
void VGUI2_CreateViewport();
void VGUI2_DestroyViewport();
#endif
