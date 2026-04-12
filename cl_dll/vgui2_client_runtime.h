#pragma once

#if !defined(VGUI2_STUB_MODE)

bool VGUI2_ClientRuntimeInstall();
void VGUI2_ClientRuntimeShutdown();
void VGUI2_ClientRuntimeOnVidInit(int width, int height);
void VGUI2_ClientRuntimeRunFrame();
bool VGUI2_ClientRuntimeHandleKeyEvent(int down, int keynum);

#endif
