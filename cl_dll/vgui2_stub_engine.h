#pragma once

#include <stdint.h>

typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

#define CREATEINTERFACE_PROCNAME "CreateInterface"

#if defined(__cplusplus)
extern "C" {
#endif

int VGui2_IsInitialized(void);
void VGui2_Init(void);
void VGui2_Shutdown(void);
void VGui2_Frame(void);
void VGui2_GetInterfaces(CreateInterfaceFn *pFactory);
CreateInterfaceFn VGui2_GetFactory(void);
void *VGui2_GetInterface(const char *pName, int *pReturnCode);

#if defined(__cplusplus)
}
#endif

extern CreateInterfaceFn VGui2_GetFactory(void);