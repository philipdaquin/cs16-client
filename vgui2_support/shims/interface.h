#pragma once

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "tier1/interface.h"

void *Sys_GetProcAddress(void *pModuleHandle, const char *pName);

class CSysModule;

extern CreateInterfaceFn Sys_GetFactoryThis(void);
extern CreateInterfaceFn Sys_GetFactory(const char *pModuleName);
extern CSysModule *Sys_LoadModule(const char *pModuleName);
extern void Sys_UnloadModule(CSysModule *pModule);
extern CreateInterfaceFn Sys_GetFactory(CSysModule *pModule);

#endif
