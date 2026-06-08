
#include <algorithm>

#include <vgui_controls/HTML.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/RichText.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>

#include "tier1/interface.h"
#include <vgui/ILocalize.h>
#include "FileSystem.h"
//#include "strtools.h"
#include "IGameUIFuncs.h"

#include "CClientVGUI.h"
#include "CBaseViewport.h"
#include "CHudViewPort.h"
#include "vgui_resource_paths.h"
#include "hud.h"

#include "CClientMOTD.h"

using namespace vgui2;

class CClientMOTDHTML : public vgui2::HTML
{
public:
	using vgui2::HTML::HTML;
};

#if defined( __EMSCRIPTEN__ )
#include <emscripten.h>

static void ShowWebMOTD( const char *title, const char *msg, int x, int y, int w, int h )
{
	EM_ASM({
		const rootId = "xash-web-motd-root";
		const overlayId = "xash-web-motd-overlay";
		let root = document.getElementById( rootId );
		if( !root )
		{
			root = document.createElement( "div" );
			root.id = rootId;
			document.body.appendChild( root );
		}

		root.style.position = 'fixed';
		root.style.left = '0';
		root.style.top = '0';
		root.style.width = '100%';
		root.style.height = '100%';
		root.style.pointerEvents = 'none';
		root.style.zIndex = '2147483647';

		let overlay = document.getElementById( overlayId );
		if( !overlay )
		{
			overlay = document.createElement( "div" );
			overlay.id = overlayId;
			root.appendChild( overlay );
		}

		var canvas = ( Module && Module.canvas ) ? Module.canvas : null;
		var rectLeft = 0;
		var rectTop = 0;
		var rectWidth = window.innerWidth;
		var rectHeight = window.innerHeight;
		var canvasW = window.innerWidth;
		var canvasH = window.innerHeight;

		if( canvas )
		{
			var rect = canvas.getBoundingClientRect();
			rectLeft = rect.left;
			rectTop = rect.top;
			rectWidth = rect.width;
			rectHeight = rect.height;
			canvasW = canvas.width;
			canvasH = canvas.height;
		}

		var scaleX = canvasW > 0 ? rectWidth / canvasW : 1;
		var scaleY = canvasH > 0 ? rectHeight / canvasH : 1;

		var left = Number( $2 );
		var top = Number( $3 );
		var width = Number( $4 );
		var height = Number( $5 );
		overlay.style.position = 'absolute';
		overlay.style.left = ( rectLeft + ( left * scaleX ) ) + "px";
		overlay.style.top = ( rectTop + ( top * scaleY ) ) + "px";
		overlay.style.width = ( width * scaleX ) + "px";
		overlay.style.height = ( height * scaleY ) + "px";
		// overlay.style.background = '#101214';
		overlay.style.border = '0';
		overlay.style.margin = '0';
		overlay.style.padding = '0';
		overlay.style.overflow = 'hidden';
		overlay.style.boxSizing = 'border-box';
		overlay.style.pointerEvents = 'auto';

		overlay.innerHTML = "";

		const iframe = document.createElement( "iframe" );
		iframe.style.width = "100%";
		iframe.style.height = "100%";
		iframe.style.border = "0";
		iframe.style.display = "block";
		// iframe.style.background = "transparent";

		const content = UTF8ToString( $1 );
		if( content.startsWith( "http://" ) || content.startsWith( "https://" ) )
		{
			iframe.src = content;
		}
		else
		{
			iframe.srcdoc = content;
		}

		if( $0 )
		{
			iframe.title = UTF8ToString( $0 );
		}

		overlay.appendChild( iframe );
	}, title, msg, x, y, w, h );
}

static void HideWebMOTD()
{
	EM_ASM({
		const overlay = document.getElementById( "xash-web-motd-overlay" );
		if( overlay )
		{
			overlay.remove();
		}
	});
}
#else
static void ShowWebMOTD( const char *, const char * ) {}
static void HideWebMOTD() {}
#endif

CClientMOTD::CClientMOTD( IViewport* pParent )
	: BaseClass( nullptr, "ClientMOTD" )
	, m_pViewport( pParent )
	, m_bFileWritten( false )
	, m_iScoreBoardKey( 0 )
{
	//Sanity check.
	Assert( ARRAYSIZE( m_szTempFileName ) > strlen( "motd_temp.html" ) );
	strcpy( m_szTempFileName, "motd_temp.html" );

	SetTitle( "", true );
	SetScheme( "ClientScheme" );
	SetMoveable( false );
	SetProportional( true );

	m_pMessage = new vgui2::RichText( this, "TextMessage" );
	m_pMessageHtml = new CClientMOTDHTML( this, "Message" );

	LoadControlSettings( vgui2::resource_paths::kMenuMOTD, "GAME" );
	InvalidateLayout();

	m_pServerName = dynamic_cast<vgui2::Label*>( FindChildByName( "serverName" ) );
	if( !m_pServerName )
	{
		m_pServerName = new vgui2::Label( this, "serverName", "" );
	}

	SetVisible( false );
}

CClientMOTD::~CClientMOTD()
{
	HideWebMOTD();
	RemoveTempFile();
}

void CClientMOTD::SetLabelText( const char* textEntryName, const wchar_t* text )
{
	vgui2::Panel* pChild = FindChildByName( textEntryName );

	if( pChild )
	{
		auto pLabel = dynamic_cast<vgui2::Label*>( pChild );

		if( pLabel )
			pLabel->SetText( text );
	}
}

bool CClientMOTD::IsURL( const char* str )
{
	//TODO: https support
	return strncmp( str, "http://", 7 ) == 0 || strncmp( str, "https://", 8 ) == 0;
}

// void CClientMOTD::PerformLayout()
// {
// 	int screenW, screenH;
// 	surface()->GetScreenSize(screenW, screenH);
// 	
// 	const int offsetX = 640; //(screenW - 640) / 2 - 39;
// 	const int offsetY = 0; //(screenH - 448) / 2;
// 	
// 	SetPos(offsetX, offsetY);
//
// 	BaseClass::PerformLayout();
// 	int x, y;
// 	m_pMessageHtml->GetSize(x, y);
// 	m_pMessage->SetSize(x, y);
// 	m_pMessageHtml->GetPos(x, y);
// 	m_pMessage->SetPos(x, y);
//
// 	if (m_pViewport)
// 		m_pViewport->ShowBackGround(true);
// }

void CClientMOTD::PerformLayout()
{
	int screenW, screenH;
	surface()->GetScreenSize(screenW, screenH);

	const int menuW = scheme()->GetProportionalScaledValue(494);
	const int menuH = scheme()->GetProportionalScaledValue(480);

	const int offsetX = ((screenW - menuW) / 2) + 5;
	const int offsetY = (screenH - menuH) / 2;

	SetPos(offsetX, offsetY);

	BaseClass::PerformLayout();
	int x, y;
	m_pMessageHtml->GetSize(x, y);
	m_pMessage->SetSize(x, y);
	m_pMessageHtml->GetPos(x, y);
	m_pMessage->SetPos(x, y);

	if (m_pViewport)
		m_pViewport->ShowBackGround(true);
}

void CClientMOTD::OnMousePressed( vgui2::MouseCode code )
{
	OnCommand( "okay" );
}

void CClientMOTD::OnKeyCodeTyped( vgui2::KeyCode key )
{
	if( key == KEY_PAD_ENTER || key == KEY_ENTER )
	{
		OnCommand( "okay" );
	}
	else
	{
		if( m_iScoreBoardKey != KEY_NONE && m_iScoreBoardKey == key )
		{
			//TODO
			//if( !gViewPort->IsScoreBoardVisible() )
			{
				//g_pViewport->ShowBackGround( false );
				//g_pViewport->ShowScoreBoard();
				//SetVisible( false );
			}
		}
		else
		{
			BaseClass::OnKeyCodeTyped( key );
		}
	}
}

void CClientMOTD::OnCommand( const char* command )
{
	if( !stricmp( command, "okay" ) )
	{
		gEngfuncs.pfnClientCmd("motd_ok");
		RemoveTempFile();

		Close();
	}

	BaseClass::OnCommand( command );
}

void CClientMOTD::Close()
{
	HideWebMOTD();
	if( auto *input = vgui2::input() )
	{
		if( input->GetAppModalSurface() == GetVPanel() )
			input->ReleaseAppModalSurface();
	}

	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );

	BaseClass::Close();
	m_pViewport->ShowBackGround( false );
	if (auto *viewport = static_cast<CHudViewport *>(m_pViewport))
		viewport->m_bMOTDActive = false;
	gHUD.m_Menu.FlushPendingVGUIMenu();
}

void CClientMOTD::Activate( const char* title, const char* msg )
{
	HideWebMOTD();
	m_pMessage->SetVisible(true);
	m_pMessageHtml->SetVisible(false);
	m_pViewport->ShowBackGround( true );
	BaseClass::Activate();
	SetVisible( true );
	MoveToFront();
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );

	if( auto *input = vgui2::input() )
		input->SetAppModalSurface( GetVPanel() );

	RequestFocus();

	SetTitle( title, false );
	m_pServerName->SetText( title ? title : "" );

	m_pMessage->SetText(msg);
}

void CClientMOTD::ActivateHtml( const char* title, const char* msg )
{
	HideWebMOTD();
	m_pMessage->SetVisible(false);
	m_pMessageHtml->SetVisible(true);
	BaseClass::Activate();
	SetVisible( true );
	MoveToFront();
	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );

	if( auto *input = vgui2::input() )
		input->SetAppModalSurface( GetVPanel() );

	RequestFocus();

	SetTitle( title, false );
	m_pServerName->SetText( title ? title : "" );

// // #if defined( __EMSCRIPTEN__ )
// // 	PerformLayout();
// 	int htmlX = 50, htmlY = 0, htmlW = 0, htmlH = 0;
// 	m_pMessageHtml->GetPos( htmlX, htmlY );
// 	m_pMessageHtml->LocalToScreen( htmlX, htmlY );
// 	m_pMessageHtml->GetSize( htmlW, htmlH );
// // 	if( htmlW <= 0 || htmlH <= 0 )
// // 	{
// // 		htmlW = scheme()->GetProportionalScaledValue( 430 );
// // 		htmlH = scheme()->GetProportionalScaledValue( 360 );
// // 	}

// 	ShowWebMOTD( title, msg, htmlX, htmlY, htmlW, htmlH );
// 	return;
// // #endif


#if defined( __EMSCRIPTEN__ )
	PerformLayout();
	int htmlX = 0, htmlY = 0, htmlW = 0, htmlH = 0;
	int frameX = 0, frameY = 0;
	int childX = 0, childY = 0;
	GetPos( frameX, frameY );
	m_pMessageHtml->GetPos( childX, childY );
	htmlX = frameX + childX;
	htmlY = frameY + childY;
	m_pMessageHtml->GetSize( htmlW, htmlH );
	if( htmlW <= 0 || htmlH <= 0 )
	{
		htmlW = scheme()->GetProportionalScaledValue( 430 );
		htmlH = scheme()->GetProportionalScaledValue( 360 );
	}

	ShowWebMOTD( title, msg, htmlX, htmlY, htmlW, htmlH );
	return;
#endif




	char localURL[ MAX_HTML_FILENAME_LENGTH + 7 ];

	if( !m_pMessageHtml || !m_pMessageHtml->HasBrowser() )
	{
		// The HTML browser backend is unavailable in this build.
		Activate( title, msg );
		return;
	}

	const char* pszURL = msg;

	if( !IsURL( msg ) )
	{
		pszURL = nullptr;
	
		RemoveTempFile();
	
		if( !strstr( msg, "img src=\"view-source:" ) && !strstr( msg, "<style>;@/*" ) )
		{
			FileHandle_t hFile = filesystem()->Open( m_szTempFileName, "w+", "GAMECONFIG" );
	
			if( hFile != FILESYSTEM_INVALID_HANDLE )
			{
				filesystem()->Write( msg, strlen( msg ), hFile );
				filesystem()->Close( hFile );
	
				strcpy( localURL, "file:///" );
	
				const size_t uiURLLength = strlen( localURL );
				filesystem()->GetLocalPath( m_szTempFileName, localURL + uiURLLength, sizeof( localURL ) - uiURLLength );
	
				pszURL = localURL;
			}
		}
	}

	if( pszURL )
		m_pMessageHtml->OpenURL( pszURL, nullptr );

	if( m_iScoreBoardKey == KEY_NONE )
		m_iScoreBoardKey = gameUIFuncs()->GetVGUI2KeyCodeForBind( "showscores" );
}

#if 0
void CClientMOTD::Activate( const wchar_t* title, const wchar_t* msg )
{
	char localURL[ MAX_HTML_FILENAME_LENGTH + 7 ];
	char ansiURL[ MAX_PATH ];

	BaseClass::Activate();
	SetVisible( true );
	MoveToFront();

	SetTitle( title, false );
	m_pServerName->SetText( title );

	localize()->ConvertUnicodeToANSI( msg, ansiURL, sizeof( ansiURL ) );

	if( IsURL( ansiURL ) )
	{
		m_pMessageHtml->OpenURL( ansiURL, nullptr );
	}
	else
	{
		RemoveTempFile();

		FileHandle_t hFile = filesystem()->Open( m_szTempFileName, "w+", "GAMECONFIG" );

		if( hFile )
		{
			//Note: CZero doesn't multiply by sizeof( wchar_t ) - Solokiller
			filesystem()->Write( msg, wcslen( msg ) * sizeof( wchar_t ), hFile );
			filesystem()->Close( hFile );

			strcpy( localURL, "file:///" );
			const size_t uiURLLength = strlen( localURL );

			filesystem()->GetLocalPath( m_szTempFileName, localURL + uiURLLength, sizeof( localURL ) - uiURLLength );
			m_pMessageHtml->OpenURL( localURL, nullptr );
		}
	}

	SetVisible( true );
}
#endif

void CClientMOTD::Reset()
{
	HideWebMOTD();
	m_pMessageHtml->OpenURL( "", nullptr );
	m_pMessage->SetText("");

	RemoveTempFile();

	m_pServerName->SetText( "" );
}

void CClientMOTD::ShowPanel( bool state )
{
	if( BaseClass::IsVisible() == state )
		return;

	if( !state )
		HideWebMOTD();

	m_pViewport->ShowBackGround( state );

	if( state )
	{
		Reset();
		Update();

		BaseClass::Activate();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );

		if( auto *input = vgui2::input() )
			input->SetAppModalSurface( GetVPanel() );

		RequestFocus();
	}
	else
	{
		if( auto *input = vgui2::input() )
		{
			if( input->GetAppModalSurface() == GetVPanel() )
				input->ReleaseAppModalSurface();
		}

		BaseClass::SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}

void CClientMOTD::RemoveTempFile()
{
	if( filesystem()->FileExists( m_szTempFileName ) )
	{
		filesystem()->RemoveFile( m_szTempFileName, "GAMECONFIG" );
	}
}
