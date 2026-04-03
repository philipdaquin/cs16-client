#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__)
#include <malloc/malloc.h>
#endif

#include "tier0/memalloc.h"

class CSimpleMemAlloc : public IMemAlloc
{
public:
	virtual void *Alloc(size_t nSize) { return malloc(nSize); }
	virtual void *Realloc(void *pMem, size_t nSize) { return realloc(pMem, nSize); }
	virtual void Free(void *pMem) { free(pMem); }
	virtual void *Expand_NoLongerSupported(void *pMem, size_t nSize) { return realloc(pMem, nSize); }

	virtual void *Alloc(size_t nSize, const char *pFileName, int nLine) { (void)pFileName; (void)nLine; return malloc(nSize); }
	virtual void *Realloc(void *pMem, size_t nSize, const char *pFileName, int nLine) { (void)pFileName; (void)nLine; return realloc(pMem, nSize); }
	virtual void Free(void *pMem, const char *pFileName, int nLine) { (void)pFileName; (void)nLine; free(pMem); }
	virtual void *Expand_NoLongerSupported(void *pMem, size_t nSize, const char *pFileName, int nLine) { (void)pFileName; (void)nLine; return realloc(pMem, nSize); }

	virtual size_t GetSize(void *pMem)
	{
#if defined(__APPLE__)
		return pMem ? malloc_size(pMem) : 0;
#else
		return 0;
#endif
	}

	virtual void PushAllocDbgInfo(const char *pFileName, int nLine) { (void)pFileName; (void)nLine; }
	virtual void PopAllocDbgInfo() {}

	virtual long CrtSetBreakAlloc(long lNewBreakAlloc) { return lNewBreakAlloc; }
	virtual int CrtSetReportMode(int nReportType, int nReportMode) { (void)nReportType; return nReportMode; }
	virtual int CrtIsValidHeapPointer(const void *pMem) { return pMem != NULL; }
	virtual int CrtIsValidPointer(const void *pMem, unsigned int size, int access) { (void)size; (void)access; return pMem != NULL; }
	virtual int CrtCheckMemory(void) { return 1; }
	virtual int CrtSetDbgFlag(int nNewFlag) { return nNewFlag; }
	virtual void CrtMemCheckpoint(_CrtMemState *pState) { (void)pState; }

	virtual void DumpStats() {}

	virtual void* CrtSetReportFile(int nRptType, void* hFile) { (void)nRptType; return hFile; }
	virtual void* CrtSetReportHook(void* pfnNewHook) { return pfnNewHook; }
	virtual int CrtDbgReport(int nRptType, const char *szFile, int nLine, const char *szModule, const char *pMsg)
	{
		(void)nRptType;
		(void)szFile;
		(void)nLine;
		(void)szModule;
		(void)pMsg;
		return 0;
	}

	virtual int heapchk() { return 0; }
	virtual bool IsDebugHeap() { return false; }
	virtual void GetActualDbgInfo(const char *&pFileName, int &nLine) { pFileName = ""; nLine = 0; }
	virtual void RegisterAllocation(const char *pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime)
	{
		(void)pFileName; (void)nLine; (void)nLogicalSize; (void)nActualSize; (void)nTime;
	}
	virtual void RegisterDeallocation(const char *pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime)
	{
		(void)pFileName; (void)nLine; (void)nLogicalSize; (void)nActualSize; (void)nTime;
	}

	virtual int GetVersion() { return MEMALLOC_VERSION; }
	virtual void CompactHeap() {}
	virtual MemAllocFailHandler_t SetAllocFailHandler(MemAllocFailHandler_t pfnMemAllocFailHandler)
	{
		MemAllocFailHandler_t oldHandler = m_pFailHandler;
		m_pFailHandler = pfnMemAllocFailHandler;
		return oldHandler;
	}

private:
	MemAllocFailHandler_t m_pFailHandler = NULL;
};

static CSimpleMemAlloc g_SimpleMemAlloc;
IMemAlloc *g_pMemAlloc = &g_SimpleMemAlloc;
