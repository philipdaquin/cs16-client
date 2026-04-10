#define TIER0_DLL_EXPORT
#include "tier0/memalloc.h"

#include <cstdlib>
#include <cstring>

namespace
{
struct MemHeader
{
	size_t size;
};

class CWasmMemAlloc final : public IMemAlloc
{
public:
	void *Alloc( size_t nSize ) override
	{
		return AllocInternal( nSize );
	}

	void *Realloc( void *pMem, size_t nSize ) override
	{
		return ReallocInternal( pMem, nSize );
	}

	void Free( void *pMem ) override
	{
		FreeInternal( pMem );
	}

	void *Expand_NoLongerSupported( void *pMem, size_t nSize ) override
	{
		return ReallocInternal( pMem, nSize );
	}

	void *Alloc( size_t nSize, const char *pFileName, int nLine ) override
	{
		(void)pFileName;
		(void)nLine;
		return AllocInternal( nSize );
	}

	void *Realloc( void *pMem, size_t nSize, const char *pFileName, int nLine ) override
	{
		(void)pFileName;
		(void)nLine;
		return ReallocInternal( pMem, nSize );
	}

	void Free( void *pMem, const char *pFileName, int nLine ) override
	{
		(void)pFileName;
		(void)nLine;
		FreeInternal( pMem );
	}

	void *Expand_NoLongerSupported( void *pMem, size_t nSize, const char *pFileName, int nLine ) override
	{
		(void)pFileName;
		(void)nLine;
		return ReallocInternal( pMem, nSize );
	}

	size_t GetSize( void *pMem ) override
	{
		if( !pMem )
			return 0;

		return HeaderFromPayload( pMem )->size;
	}

	void PushAllocDbgInfo( const char *pFileName, int nLine ) override
	{
		(void)pFileName;
		(void)nLine;
	}

	void PopAllocDbgInfo() override {}

	long CrtSetBreakAlloc( long lNewBreakAlloc ) override
	{
		return lNewBreakAlloc;
	}

	int CrtSetReportMode( int nReportType, int nReportMode ) override
	{
		(void)nReportType;
		return nReportMode;
	}

	int CrtIsValidHeapPointer( const void *pMem ) override
	{
		return pMem != nullptr;
	}

	int CrtIsValidPointer( const void *pMem, unsigned int size, int access ) override
	{
		(void)size;
		(void)access;
		return pMem != nullptr;
	}

	int CrtCheckMemory( void ) override
	{
		return 1;
	}

	int CrtSetDbgFlag( int nNewFlag ) override
	{
		return nNewFlag;
	}

	void CrtMemCheckpoint( _CrtMemState *pState ) override
	{
		(void)pState;
	}

	void DumpStats() override {}

	void *CrtSetReportFile( int nRptType, void *hFile ) override
	{
		(void)nRptType;
		return hFile;
	}

	void *CrtSetReportHook( void *pfnNewHook ) override
	{
		return pfnNewHook;
	}

	int CrtDbgReport( int nRptType, const char *szFile, int nLine, const char *szModule, const char *pMsg ) override
	{
		(void)nRptType;
		(void)szFile;
		(void)nLine;
		(void)szModule;
		(void)pMsg;
		return 0;
	}

	int heapchk() override
	{
		return 1;
	}

	bool IsDebugHeap() override
	{
		return false;
	}

	void GetActualDbgInfo( const char *&pFileName, int &nLine ) override
	{
		pFileName = "";
		nLine = 0;
	}

	void RegisterAllocation( const char *pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime ) override
	{
		(void)pFileName;
		(void)nLine;
		(void)nLogicalSize;
		(void)nActualSize;
		(void)nTime;
	}

	void RegisterDeallocation( const char *pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime ) override
	{
		(void)pFileName;
		(void)nLine;
		(void)nLogicalSize;
		(void)nActualSize;
		(void)nTime;
	}

	int GetVersion() override
	{
		return MEMALLOC_VERSION;
	}

	void CompactHeap() override {}

	MemAllocFailHandler_t SetAllocFailHandler( MemAllocFailHandler_t pfnMemAllocFailHandler ) override
	{
		MemAllocFailHandler_t oldHandler = m_pFailHandler;
		m_pFailHandler = pfnMemAllocFailHandler;
		return oldHandler;
	}

private:
	static MemHeader *HeaderFromPayload( void *pMem )
	{
		return reinterpret_cast<MemHeader *>( pMem ) - 1;
	}

	void *AllocInternal( size_t nSize )
	{
		MemHeader *header = AllocateBlock( nSize );
		if( !header && m_pFailHandler )
		{
			m_pFailHandler( nSize );
			header = AllocateBlock( nSize );
		}

		return header ? header + 1 : nullptr;
	}

	void *ReallocInternal( void *pMem, size_t nSize )
	{
		if( !pMem )
			return AllocInternal( nSize );

		MemHeader *header = HeaderFromPayload( pMem );
		MemHeader *resized = reinterpret_cast<MemHeader *>( std::realloc( header, sizeof( MemHeader ) + nSize ) );
		if( !resized && m_pFailHandler )
		{
			m_pFailHandler( nSize );
			resized = reinterpret_cast<MemHeader *>( std::realloc( header, sizeof( MemHeader ) + nSize ) );
		}

		if( !resized )
			return nullptr;

		resized->size = nSize;
		return resized + 1;
	}

	void FreeInternal( void *pMem )
	{
		if( !pMem )
			return;

		std::free( HeaderFromPayload( pMem ) );
	}

	static MemHeader *AllocateBlock( size_t nSize )
	{
		MemHeader *header = reinterpret_cast<MemHeader *>( std::malloc( sizeof( MemHeader ) + nSize ) );
		if( !header )
			return nullptr;

		header->size = nSize;
		return header;
	}

	MemAllocFailHandler_t m_pFailHandler = nullptr;
};

CWasmMemAlloc g_WasmMemAlloc;
}

IMemAlloc *g_pMemAlloc = &g_WasmMemAlloc;
