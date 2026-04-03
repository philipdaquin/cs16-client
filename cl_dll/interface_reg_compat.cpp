#include "../common/interface.h"

InterfaceReg::InterfaceReg(InstantiateInterfaceFn fn, const char *pName) :
	m_CreateFn(fn),
	m_pName(pName),
	m_pNext(s_pInterfaceRegs)
{
	s_pInterfaceRegs = this;
}
