#include <cstdarg>
#include <cstdio>

#include "tier1/KeyValues.h"

#include <vgui/IClientPanel.h>
#include <vgui/IPanel.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Controls.h>

#include "vgui.h"
#include "VPanel.h"

namespace vgui2
{
	inline VPANEL VPanelToHandle(VPanel *vguiPanel)
	{
		return (VPANEL)vguiPanel;
	}

	inline VPanel *VHandleToPanel(VPANEL vguiPanel)
	{
		return (VPanel *)vguiPanel;
	}
}

using vgui2::IVGui;

static CVGui g_VGui;

vgui2::IVGui *VGuiInterfaceSingleton()
{
	std::fprintf(stderr, "[VGUI2-TRACE] VGuiInterfaceSingleton this=%p\n", (void *)&g_VGui);
	return &g_VGui;
}


bool PriorityQueueComp( const MessageItem_t& lhs, const MessageItem_t& rhs )
{
	if( lhs._arrivalTime > rhs._arrivalTime )
		return true;

	if( lhs._arrivalTime < rhs._arrivalTime )
		return false;

	return lhs._messageID >  rhs._messageID;
}

CVGui::CVGui()
	: m_PanelList( 1024, 1024 )
	, m_DelayedMessageQueue( 0, 4, &PriorityQueueComp )
{
}

CVGui::~CVGui()
{
}

bool CVGui::Init( CreateInterfaceFn *factoryList, int numFactories )
{
	if( m_bInitialized )
		return true;

	m_bInitialized = true;

	//TODO: the original library uses a different function - Solokiller
	return vgui2::VGui_InitInterfacesList( "VGUI2", factoryList, numFactories );
}

void CVGui::Shutdown()
{
	for( auto index = m_Contexts.Head();
		 index != m_Contexts.InvalidIndex(); )
	{
		auto next = m_Contexts.Next( index );

		DestroyContext( index );

		index = next;
	}

	ClearMessageQueues();
}

void CVGui::Start()
{
	m_bRunning = true;
}

void CVGui::Stop()
{
	m_bRunning = false;
}

bool CVGui::IsRunning()
{
	return m_bRunning;
}

void CVGui::RunFrame()
{
	// Original trace:
	// std::fprintf(stderr, "[VGUI2-TRACE] ivgui->RunFrame enter this=%p running=%d panels=%d\n",
	// 	(void *)this, m_bRunning ? 1 : 0, m_TickSignalVec.Count());
	vgui2::surface()->RunFrame();

	vgui2::system()->RunFrame();

	int cursorX, cursorY;
	vgui2::input()->GetCursorPos( cursorX, cursorY );
	vgui2::input()->UpdateMouseFocus( cursorX, cursorY );
	vgui2::input()->RunFrame();

	DispatchMessages();

	const auto time = vgui2::system()->GetTimeMillis();

	for( auto i = m_TickSignalVec.Count() - 1; i >= 0; --i )
	{
		auto pTick = m_TickSignalVec[ i ];

		if( !pTick->panel )
		{
			//Invalid panel, remove.
			delete pTick;
			m_TickSignalVec.Remove( i );
			continue;
		}

		if( pTick->interval )
		{
			if( time < pTick->nexttick )
				continue;

			pTick->nexttick = time + pTick->interval;
		}

		vgui2::ivgui()->PostMessage(
			pTick->panel,
			new KeyValues( "Tick" ),
			NULL_HANDLE
		);
	}

	// Old path:
	// vgui2::surface()->SolveTraverse( vgui2::surface()->GetEmbeddedPanel() );
	// vgui2::surface()->ApplyChanges();
	auto surface = vgui2::surface();
	if( surface )
	{
		// Original trace:
		// std::fprintf(stderr, "[VGUI2-TRACE] ivgui->RunFrame SolveTraverse embedded=%p\n",
		// 	(void *)surface->GetEmbeddedPanel());
		surface->SolveTraverse( surface->GetEmbeddedPanel() );
		surface->ApplyChanges();
	}
	// Original trace:
	// std::fprintf(stderr, "[VGUI2-TRACE] ivgui->RunFrame exit this=%p\n", (void *)this);
}

void CVGui::ShutdownMessage( unsigned int shutdownID )
{
	// Old path:
	// auto embeddedPanel = reinterpret_cast<vgui2::VPanel*>( vgui2::surface()->GetEmbeddedPanel() );
	auto surface = vgui2::surface();
	auto embeddedPanel = reinterpret_cast<vgui2::VPanel*>( surface ? surface->GetEmbeddedPanel() : 0 );

	for( int i = 0; i < embeddedPanel->GetChildCount(); ++i )
	{
		vgui2::ivgui()->PostMessage(
			reinterpret_cast<vgui2::VPANEL>( embeddedPanel->GetChild( i ) ),
			new KeyValues( "ShutdownRequest", "id", shutdownID ),
			NULL_HANDLE
		);
	}

	vgui2::ivgui()->PostMessage(
		reinterpret_cast<vgui2::VPANEL>( embeddedPanel ),
		new KeyValues( "ShutdownRequest", "id", shutdownID ),
		NULL_HANDLE
	);
}

vgui2::VPANEL CVGui::AllocPanel()
{
	std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel enter this=%p running=%d panelCount=%d\n",
		(void *)this,
		m_bRunning,
		m_PanelList.Count());
	std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel pre-surface this=%p surface=%p\n",
		(void *)this,
		(void *)vgui2::surface());

	vgui2::VPANEL embeddedPanel = 0;
	if (vgui2::surface())
	{
		std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel before GetEmbeddedPanel surface=%p\n",
			(void *)vgui2::surface());
		embeddedPanel = vgui2::surface()->GetEmbeddedPanel();
		std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel after GetEmbeddedPanel embedded=%p surface=%p\n",
			(void *)embeddedPanel,
			(void *)vgui2::surface());
	}
	else
	{
		std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel surface is null before embedded lookup\n");
	}

	std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel before new VPanel embedded=%p\n",
		(void *)embeddedPanel);
	auto panel = new vgui2::VPanel();
	std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel after new VPanel panel=%p embedded=%p\n",
		(void *)panel,
		(void *)embeddedPanel);
	PanelCreated( panel );
	std::fprintf(stderr, "[VGUI2-TRACE] AllocPanel after PanelCreated panel=%p listEntry=%u embedded=%p\n",
		(void *)panel,
		panel ? panel->GetListEntry() : 0xFFFF,
		(void *)embeddedPanel);

	return reinterpret_cast<vgui2::VPANEL>( panel );
}

void CVGui::FreePanel( vgui2::VPANEL panel )
{
	PanelDeleted( reinterpret_cast<vgui2::VPanel*>( panel ) );

	if( panel )
		delete reinterpret_cast<vgui2::VPanel*>( panel );
}

void CVGui::DPrintf( const char *format, ... )
{
	char buf[ 2048 ];

	va_list va;

	va_start( va, format );
	vsnprintf( buf, ARRAYSIZE( buf ), format, va );
	va_end( va );

	fputs( buf, stderr );
}

void CVGui::DPrintf2( const char *format, ... )
{
	static int ctr = 0;

	char buf[ 2048 ];

	snprintf( buf, ARRAYSIZE( buf ), "%d:", ctr++ );

	const auto length = strlen( buf );

	auto pBuf = buf + length;

	va_list va;

	va_start( va, format );
	vsnprintf( pBuf, ARRAYSIZE( buf ) - length, format, va );
	va_end( va );

	fputs( buf, stderr );
}

void CVGui::SpewAllActivePanelNames()
{
	for( int index = m_PanelList.Head();
		 index != m_PanelList.InvalidIndex();
		 index = m_PanelList.Next( index ) )
	{

		vgui2::ivgui()->DPrintf2(
			"\tpanel '%s' of type '%s' leaked\n",
			vgui2::ipanel()->GetName( m_PanelList[ index ].m_pPanel ),
			reinterpret_cast<vgui2::VPanel*>( m_PanelList[ index ].m_pPanel )->GetClassName()
		);
	}
}

vgui2::HPanel CVGui::PanelToHandle( vgui2::VPANEL panel )
{
	if( !panel )
		return vgui2::INVALID_PANEL;

	auto listEntry = reinterpret_cast<vgui2::VPanel*>( panel )->GetListEntry();

	return ( listEntry << 16 ) + m_PanelList[ listEntry ].m_iSerialNumber;
}

vgui2::VPANEL CVGui::HandleToPanel( vgui2::HPanel index )
{
	if( index != vgui2::INVALID_PANEL )
	{
		const vgui2::HPanel listEntry = index >> 16;

		if( m_PanelList.IsValidIndex( listEntry ) )
		{
			auto& entry = m_PanelList[ listEntry ];

			if( ( index & 0xFFFF ) == entry.m_iSerialNumber )
			{
				auto panel = vgui2::VHandleToPanel( entry.m_pPanel );
				return entry.m_pPanel;
			}
		}
	}

	return NULL_HANDLE;
}

void CVGui::MarkPanelForDeletion( vgui2::VPANEL panel )
{
	PostMessage( panel, new KeyValues( "Delete" ), NULL_HANDLE );
}

void CVGui::AddTickSignal( vgui2::VPANEL panel, int intervalMilliseconds )
{
	//See if it's in the list already.
	for( int i = 0; i < m_TickSignalVec.Count(); ++i )
	{
		auto pTick = m_TickSignalVec[ i ];

		if( pTick->panel == panel )
		{
			pTick->interval = intervalMilliseconds;
			pTick->nexttick = pTick->interval + vgui2::system()->GetTimeMillis();
			return;
		}
	}

	//Insert it.
	auto pTick = new Tick_t;

	pTick->panel = panel;
	pTick->interval = intervalMilliseconds;
	pTick->nexttick = pTick->interval + vgui2::system()->GetTimeMillis();

	auto pClient = reinterpret_cast<vgui2::VPanel*>( panel )->Client();

	const char* pszName;

	if( pClient->GetName() )
	{
		pszName = pClient->GetName();
	}
	else
	{
		pszName = pClient->GetClassName();
	}

	strncpy( pTick->panelname, pszName, ARRAYSIZE( pTick->panelname ) );

	m_TickSignalVec.AddToTail( pTick );

}

void CVGui::RemoveTickSignal( vgui2::VPANEL panel )
{
	for( int i = 0; i < m_TickSignalVec.Count(); ++i )
	{
		auto pTick = m_TickSignalVec[ i ];

		if( pTick->panel == panel )
		{
			m_TickSignalVec.Remove( i );
			delete pTick;
			return;
		}
	}
}

void CVGui::PostMessage( vgui2::VPANEL target, KeyValues *params, vgui2::VPANEL from, float delaySeconds )
{
	if( !target )
	{
		if( params )
			params->deleteThis();

		return;
	}

	auto hTarget = vgui2::ivgui()->PanelToHandle( target );
	auto hFrom = vgui2::ivgui()->PanelToHandle( from );

	auto messageID = m_iCurrentMessageID++;

	if( delaySeconds > 0.0 )
	{
		MessageItem_t item;

		item._params = params;
		item._messageTo = hTarget;
		item._from = hFrom;
		item._arrivalTime = vgui2::system()->GetTimeMillis() + delaySeconds * 1000.0;
		item._messageID = messageID;

		m_DelayedMessageQueue.Insert( item );
	}
	else
	{
		decltype( m_MessageQueue )* pList;

		if( m_InDispatcher )
		{
			pList = &m_SecondaryQueue;
		}
		else
		{
			pList = &m_MessageQueue;
		}

		MessageItem_t item;

		item._params = params;
		item._messageTo = hTarget;
		item._from = hFrom;
		item._arrivalTime = 0;
		item._messageID = messageID;

		pList->AddToTail( item );
	}
}

vgui2::HContext CVGui::CreateContext()
{
	auto index = m_Contexts.AddToTail();

	auto& context = m_Contexts[ index ];

	context.m_hInputContext = vgui2::input()->CreateInputContext();

	return static_cast<vgui2::HContext>( index );
}

void CVGui::DestroyContext( vgui2::HContext context )
{
	if( m_hContext == context )
		ActivateContext( vgui2::DEFAULT_VGUI_CONTEXT );

	Context_t* pContext;

	if( context == vgui2::DEFAULT_VGUI_CONTEXT )
	{
		pContext = &m_DefaultContext;
	}
	else
	{
		pContext = &m_Contexts[ context ];
	}

	vgui2::input()->DestroyInputContext( pContext->m_hInputContext );

	if( context != vgui2::DEFAULT_VGUI_CONTEXT )
	{
		m_Contexts.Remove( context );
	}
}

void CVGui::AssociatePanelWithContext( vgui2::HContext context, vgui2::VPANEL pRoot )
{
	Context_t* pContext;

	if( context == vgui2::DEFAULT_VGUI_CONTEXT )
	{
		pContext = &m_DefaultContext;
	}
	else
	{
		pContext = &m_Contexts[ context ];
	}

	vgui2::input()->AssociatePanelWithInputContext( pContext->m_hInputContext, pRoot );
}

void CVGui::ActivateContext( vgui2::HContext context )
{
	if( m_hContext == context )
		return;

	DispatchMessages();

	m_hContext = context;

	if( context == vgui2::DEFAULT_VGUI_CONTEXT )
	{
		vgui2::input()->ActivateInputContext( m_DefaultContext.m_hInputContext );
	}
	else
	{
		vgui2::input()->ActivateInputContext( m_Contexts[ context ].m_hInputContext );
		vgui2::input()->RunFrame();
	}
}

void CVGui::SetSleep( bool state )
{
	m_bDoSleep = state;
}

bool CVGui::GetShouldVGuiControlSleep()
{
	return m_bDoSleep;
}

void CVGui::PanelCreated( vgui2::VPanel* panel )
{

	std::fprintf(stderr, "calling PanelCreated\n");
	std::fprintf(stderr, "[VGUI2-TRACE] PanelCreated enter panel=%p client=%p popup=%d parent=%p embedded=%p\n",
		(void *)panel,
		panel ? (void *)panel->Client() : nullptr,
		panel ? panel->IsPopup() : -1,
		panel ? (void *)panel->GetParent() : nullptr,
		// Old path:
		// (void *)vgui2::surface()->GetEmbeddedPanel()
		(void *)(
			vgui2::surface() ? vgui2::surface()->GetEmbeddedPanel() : 0
		));


	const auto listEntry = m_PanelList.AddToTail();
	std::fprintf(stderr, "[VGUI2-TRACE] PanelCreated listEntry=%u count=%d\n", listEntry, m_PanelList.Count());

	auto& entry = m_PanelList[ listEntry ];

	entry.m_iSerialNumber = m_iNextSerialNumber++;
	entry.m_pPanel = reinterpret_cast<vgui2::VPANEL>( panel );
	std::fprintf(stderr, "[VGUI2-TRACE] PanelCreated stored serial=%u handle=%p\n",
		entry.m_iSerialNumber,
		(void *)entry.m_pPanel);

	panel->SetListEntry( listEntry );
	std::fprintf(stderr, "[VGUI2-TRACE] PanelCreated after SetListEntry panel=%p listEntry=%u popup=%d parent=%p client=%p\n",
		(void *)panel,
		panel->GetListEntry(),
		panel->IsPopup(),
		(void *)panel->GetParent(),
		(void *)panel->Client());

	// During bootstrap the root panel is created before BaseUISurface::Init()
	// assigns an embedded panel. Defer surface registration until that link
	// exists so the startup panel doesn't walk popup bookkeeping too early.
	// Old path:
	// if( vgui2::surface()->GetEmbeddedPanel() )
	auto surface = vgui2::surface();
	if( surface && surface->GetEmbeddedPanel() )
	{
		surface->AddPanel( entry.m_pPanel );
		std::fprintf(stderr, "[VGUI2-TRACE] PanelCreated after AddPanel panel=%p popup=%d parent=%p embedded=%p\n",
			(void *)panel,
			panel->IsPopup(),
			(void *)panel->GetParent(),
			(void *)(surface ? surface->GetEmbeddedPanel() : 0));
	}
	else
	{
		std::fprintf(stderr, "[VGUI2-TRACE] PanelCreated skipped AddPanel during bootstrap panel=%p embedded=%p\n",
			(void *)panel,
			(void *)(surface ? surface->GetEmbeddedPanel() : 0));
	}
}

void CVGui::PanelDeleted( vgui2::VPanel* panel )
{
	auto vPanel = reinterpret_cast<vgui2::VPANEL>( panel );

	vgui2::surface()->ReleasePanel( vPanel );
	vgui2::input()->PanelDeleted( vPanel );

	auto listEntry = panel->GetListEntry();

	if( m_PanelList.IsValidIndex( listEntry ) )
	{
		m_PanelList.Remove( listEntry );
	}

	panel->SetListEntry( 0xFFFF );
	RemoveTickSignal( vPanel );

	// MoeMod fix : remove all wild pointer in msg queue
	for (int index = m_MessageQueue.Head();
		index != m_MessageQueue.InvalidIndex();
		index = m_MessageQueue.Next(index))
	{
		auto& element = m_MessageQueue[index];

		if (auto pRoot = element._params)
		{
			for (KeyValues* pValue = pRoot->GetFirstValue(); pValue; pValue = pValue->GetNextValue())
			{
				if (pValue->GetPtr(nullptr) == panel->Client())
				{
					// invalidate param
					pValue->SetPtr(nullptr, nullptr);
				}
			}
		}
	}
}

void CVGui::ClearMessageQueues()
{
	for( int index = m_MessageQueue.Head();
		 index != m_MessageQueue.InvalidIndex();
		 index = m_MessageQueue.Next( index ) )
	{
		auto& element = m_MessageQueue[ index ];

		if( element._params )
		{
			element._params->deleteThis();
		}
	}

	m_MessageQueue.RemoveAll();

	for( int index = m_SecondaryQueue.Head();
		 index != m_SecondaryQueue.InvalidIndex();
		 index = m_SecondaryQueue.Next( index ) )
	{
		auto& element = m_SecondaryQueue[ index ];

		if( element._params )
		{
			element._params->deleteThis();
		}
	}

	m_SecondaryQueue.RemoveAll();

	while( m_DelayedMessageQueue.Count() > 0 )
	{
		auto& element = m_DelayedMessageQueue.ElementAtHead();

		if( element._params )
		{
			element._params->deleteThis();
		}

		m_DelayedMessageQueue.RemoveAtHead();
	}
}

bool CVGui::DispatchMessages()
{
	const auto time = vgui2::system()->GetTimeMillis();

	m_InDispatcher = true;

	bool doneWork = m_MessageQueue.Count() > 12;
	bool bUsingDelayedQueue = m_DelayedMessageQueue.Count() > 0;

	int messageIndex;

	while( m_MessageQueue.Count() > 0 ||
		   m_SecondaryQueue.Count() > 0 ||
		   bUsingDelayedQueue )
	{
		const bool bUsingSecondaryQueue = m_SecondaryQueue.Count() > 0;

		const MessageItem_t* pMsg;

		if( bUsingSecondaryQueue )
		{
			doneWork = true;
			messageIndex = m_SecondaryQueue.Head();
			pMsg = &m_SecondaryQueue[ messageIndex ];
		}
		else if( bUsingDelayedQueue )
		{
			if( m_DelayedMessageQueue.Count() <= 0 ||
				m_DelayedMessageQueue.ElementAtHead()._arrivalTime > time )
			{
				bUsingDelayedQueue = false;
				continue;
			}

			pMsg = &m_DelayedMessageQueue.ElementAtHead();
			messageIndex = 0;
		}
		else
		{
			messageIndex = m_MessageQueue.Head();
			pMsg = &m_MessageQueue[ messageIndex ];
		}

		auto pKeyValues = pMsg->_params;

		auto panel = reinterpret_cast<vgui2::VPanel*>( vgui2::ivgui()->HandleToPanel( pMsg->_messageTo ) );

		if( panel )
		{
			panel->SendMessage( pKeyValues, vgui2::ivgui()->HandleToPanel( pMsg->_from ) );
		}

		if( pKeyValues )
			pKeyValues->deleteThis();

		if( bUsingSecondaryQueue )
		{
			m_SecondaryQueue.Remove( messageIndex );
		}
		else if( bUsingDelayedQueue )
		{
			m_DelayedMessageQueue.RemoveAtHead();
		}
		else
		{
			m_MessageQueue.Remove( messageIndex );
		}
	}

	m_InDispatcher = false;

	return doneWork;
}

namespace vgui2
{
void vgui_strcpy( char* dst, int dstLen, const char* src )
{
	int iLength = strlen( src ) + 1;

	if( iLength > dstLen )
		iLength = dstLen;

	memcpy( dst, src, iLength );

	dst[ iLength - 1 ] = '\0';
}
}
