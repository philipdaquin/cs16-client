#pragma once

#include <vgui2_stub_types.h>

#if !defined(VGUI2_STUB_MODE)

void VGUI2_CreateClientRootPanel();
void VGUI2_DestroyClientRootPanel();
void VGUI2_UpdateClientRootPanelBounds();
vgui2::VPANEL VGUI2_GetClientRootPanel();

#endif
