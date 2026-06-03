
#include "BaseUISurface.h"

#include <FileSystem.h>
#include "EmbeddedVGUIFonts.h"
#include "render_api.h"
#include "triangleapi.h"

#include "vgui/IPanel.h"
#include "vgui/IInput.h"
#include "vgui/IInputInternal.h"
#include "vgui/Cursor.h"
#include "vgui_controls/controls.h"

#include "vgui_surfacelib/FontManager.h"

#include <tier1/utlbuffer.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

//#define USE_IMGUI_SURFACE
#ifdef USE_IMGUI_SURFACE
#include "client/imgui_surface.h"
#endif

// from engine/common/common.h
typedef struct rgbdata_s
{
    word	width;		// image width
    word	height;		// image height
    word	depth;		// image depth
    uint	type;		// compression type
    uint	flags;		// misc image flags
    word	encode;
    byte	numMips;	// mipmap count
    byte	*palette = nullptr;		// palette if present
    byte	*buffer = nullptr;		// image buffer
    rgba_t	fogParams;	// some water textures in hl1 has info about fog color and alpha
    size_t	size;		// for bounds checking
} rgbdata_t;

void SPR_AdjustSize( float *x, float *y, float *w, float *h );
void SPR_AdjustSizeReverse( float *x, float *y, float *w, float *h );
void TextAdjustSize( int *x, int *y, int *w, int *h );
void TextAdjustSizeReverse( int *x, int *y, int *w, int *h );
// Old builds expected these helpers to come from the engine/client module.
// This repo now provides local wasm-safe fallbacks below so VGUI2 does not
// depend on an unresolved external symbol during TeamMenu paint.

namespace vgui2 {
	extern cl_enginefunc_t gEngfuncs;
}

static constexpr float kSurfaceBaseWidth = 640.0f;
static constexpr float kSurfaceBaseHeight = 480.0f;

static void GetSurfaceScale( float &xscale, float &yscale )
{
	SCREENINFO screenInfo;
	screenInfo.iSize = sizeof( SCREENINFO );

	if ( !vgui2::gEngfuncs.pfnGetScreenInfo( &screenInfo ) || screenInfo.iWidth <= 0 || screenInfo.iHeight <= 0 )
	{
		xscale = 1.0f;
		yscale = 1.0f;
		return;
	}

	// Keep the historical 640x480 VGUI base size visible in the code.
	xscale = screenInfo.iWidth / kSurfaceBaseWidth;
	yscale = screenInfo.iHeight / kSurfaceBaseHeight;
}

static void FreeSurfaceImage( rgbdata_t *image )
{
	if ( !image )
		return;

	if ( vgui2::gEngfuncs.FS_FreeImage )
	{
		vgui2::gEngfuncs.FS_FreeImage( image );
		return;
	}

	std::fprintf( stderr, "[VGUI2-TRACE] BaseUISurface::FreeSurfaceImage missing freer image=%p\n", static_cast<void *>( image ) );
}

void SPR_AdjustSize( float *x, float *y, float *w, float *h )
{
	float xscale = 1.0f;
	float yscale = 1.0f;

	GetSurfaceScale( xscale, yscale );

	if ( x )
		*x *= xscale;
	if ( y )
		*y *= yscale;
	if ( w )
		*w *= xscale;
	if ( h )
		*h *= yscale;
}

void SPR_AdjustSizeReverse( float *x, float *y, float *w, float *h )
{
	float xscale = 1.0f;
	float yscale = 1.0f;

	GetSurfaceScale( xscale, yscale );

	if ( x && xscale != 0.0f )
		*x /= xscale;
	if ( y && yscale != 0.0f )
		*y /= yscale;
	if ( w && xscale != 0.0f )
		*w /= xscale;
	if ( h && yscale != 0.0f )
		*h /= yscale;
}

void TextAdjustSize( int *x, int *y, int *w, int *h )
{
	float fx = x ? (float)*x : 0.0f;
	float fy = y ? (float)*y : 0.0f;
	float fw = w ? (float)*w : 0.0f;
	float fh = h ? (float)*h : 0.0f;

	SPR_AdjustSize( x ? &fx : nullptr, y ? &fy : nullptr, w ? &fw : nullptr, h ? &fh : nullptr );

	if ( x )
		*x = (int)( fx + 0.5f );
	if ( y )
		*y = (int)( fy + 0.5f );
	if ( w )
		*w = (int)( fw + 0.5f );
	if ( h )
		*h = (int)( fh + 0.5f );
}

void TextAdjustSizeReverse( int *x, int *y, int *w, int *h )
{
	float fx = x ? (float)*x : 0.0f;
	float fy = y ? (float)*y : 0.0f;
	float fw = w ? (float)*w : 0.0f;
	float fh = h ? (float)*h : 0.0f;

	SPR_AdjustSizeReverse( x ? &fx : nullptr, y ? &fy : nullptr, w ? &fw : nullptr, h ? &fh : nullptr );

	if ( x )
		*x = (int)( fx + 0.5f );
	if ( y )
		*y = (int)( fy + 0.5f );
	if ( w )
		*w = (int)( fw + 0.5f );
	if ( h )
		*h = (int)( fh + 0.5f );
}

bool BaseUISurface::m_bTranslateExtendedKeys;
#if defined(LINUX) || defined(OSX) || defined(_WIN32)
CUtlDict< BaseUISurface::font_entry, unsigned short > BaseUISurface::m_FontData;
#endif

namespace vgui2 {

extern vguiapi_t *g_api;
extern cl_enginefunc_t gEngfuncs;

static wrect_t g_ScissorRect;
qboolean g_bScissor;

}

static int white_color[4] = { 255, 255, 255, 255 };

#define VGUI_MAX_TEXTURES	2048	// a half of total textures count
extern int	g_textures[VGUI_MAX_TEXTURES];
extern int	g_textureId;
extern int	g_iBoundTexture;

using namespace vgui2;

static BaseUISurface g_BaseUISurface;

namespace
{
static bool g_FontBootstrapLoaded = false;
static std::unordered_map<std::string, std::string> g_BundledFontFiles;
static std::unordered_map<std::string, std::string> g_ResourceFontFiles;

static std::string ToLowerAscii( const char *text )
{
    std::string out;
    if ( !text )
        return out;

    for ( const unsigned char *p = reinterpret_cast<const unsigned char *>( text ); *p; ++p )
    {
        out.push_back( static_cast<char>( std::tolower( *p ) ) );
    }

    return out;
}

static std::string NormalizeFaceName( const char *text )
{
    std::string out;
    bool pendingSpace = false;

    if ( !text )
        return out;

    for ( const unsigned char *p = reinterpret_cast<const unsigned char *>( text ); *p; ++p )
    {
        const char ch = static_cast<char>( *p );
        if ( std::isspace( *p ) || ch == '-' || ch == '_' )
        {
            pendingSpace = !out.empty();
            continue;
        }

        if ( pendingSpace )
        {
            out.push_back( ' ' );
            pendingSpace = false;
        }

        out.push_back( static_cast<char>( std::tolower( *p ) ) );
    }

    return out;
}

static std::string FontFileNameKey( const char *path )
{
    if ( !path )
        return {};

    const char *fileName = path;
    for ( const char *p = path; *p; ++p )
    {
        if ( *p == '/' || *p == '\\' )
            fileName = p + 1;
    }

    return ToLowerAscii( fileName );
}

static std::string MakeFontPath( const char *dir, const char *filename )
{
    if ( !filename || !filename[0] )
        return {};

    if ( std::strchr( filename, '/' ) || std::strchr( filename, '\\' ) )
        return filename;

    std::string path = dir ? dir : "";
    if ( !path.empty() && path.back() != '/' )
        path.push_back( '/' );
    path += filename;
    return path;
}

static bool FontPathExists( const char *path )
{
    if ( !path || !path[0] )
        return false;

    return vgui2::filesystem()->FileExists( path );
}

static void IndexFontFile( std::unordered_map<std::string, std::string> &index, const std::string &path )
{
    const std::string key = FontFileNameKey( path.c_str() );
    if ( key.empty() || index.find( key ) != index.end() )
        return;

    index.emplace( key, path );
}

static void ScanFontDirectory( const char *dir, std::unordered_map<std::string, std::string> &index )
{
    char pattern[256];
    Q_snprintf( pattern, sizeof( pattern ), "%s/*.ttf", dir );

    FileFindHandle_t findHandle = NULL;
    const char *pszFilename = vgui2::filesystem()->FindFirst( pattern, &findHandle );
    while ( pszFilename )
    {
        const std::string path = MakeFontPath( dir, pszFilename );
        IndexFontFile( index, path );
        std::fprintf( stderr, "[VGUI2-FONT] indexed font: %s\n", path.c_str() );
        g_BaseUISurface.AddCustomFontFile( path.c_str() );
        pszFilename = vgui2::filesystem()->FindNext( findHandle );
    }
    vgui2::filesystem()->FindClose( findHandle );
}

static const char *KnownFontFileForFace( const std::string &normalizedFace, bool symbol )
{
    if ( symbol || normalizedFace == "marlett" )
        return "marlett.ttf";

    struct KnownFont
    {
        const char *face;
        const char *file;
    };

    static const KnownFont kKnownFonts[] = {
        { "default", "Verdana.ttf" },
        { "defaultsmall", "Verdana.ttf" },
        { "default very small", "Verdana.ttf" },
        { "defaultverysmall", "Verdana.ttf" },
        { "default very small fallback", "Verdana.ttf" },
        { "defaultverysmallfallback", "Verdana.ttf" },
        { "menutitle", "Verdana.ttf" },
        { "brightcontroltext", "Verdana.ttf" },
        { "basetext", "Verdana.ttf" },
        { "label.textcolor", "Verdana.ttf" },
        { "label.textbrightcolor", "Verdana.ttf" },
        { "label textcolor", "Verdana.ttf" },
        { "label textbrightcolor", "Verdana.ttf" },
        { "tahoma", "Tahoma.ttf" },
        { "tahoma regular", "Tahoma.ttf" },
        { "tahoma bold", "TahomaBd.ttf" },
        { "arial", "Arial.ttf" },
        { "arial regular", "Arial.ttf" },
        { "arial bold", "ArialBd.ttf" },
        { "arial italic", "ArialIt.ttf" },
        { "arial bold italic", "ArialBdIt.ttf" },
        { "verdana", "Verdana.ttf" },
        { "verdana regular", "Verdana.ttf" },
        { "verdana bold", "VerdanaBd.ttf" },
        { "verdana italic", "VerdanaIt.ttf" },
        { "verdana bold italic", "VerdanaBdIt.ttf" },
        { "firasans", "FiraSans-Regular.ttf" },
        { "firasans regular", "FiraSans-Regular.ttf" },
        { "firasans bold", "FiraSans-Bold.ttf" },
        { "firasans semibold", "FiraSans-SemiBold.ttf" },
        { "firasans medium", "FiraSans-Medium.ttf" },
        { "firasans light", "FiraSans-Light.ttf" },
        { "fira sans", "FiraSans-Regular.ttf" },
        { "fira sans regular", "FiraSans-Regular.ttf" },
        { "fira sans bold", "FiraSans-Bold.ttf" },
        { "fira sans semibold", "FiraSans-SemiBold.ttf" },
        { "fira sans medium", "FiraSans-Medium.ttf" },
        { "fira sans light", "FiraSans-Light.ttf" },
    };

    for ( const KnownFont &knownFont : kKnownFonts )
    {
        if ( normalizedFace == knownFont.face )
            return knownFont.file;
    }

    return nullptr;
}

static bool IsBoldWeight( int weight )
{
    return weight >= 600;
}

static const char *StyledKnownFontFileForFace( const std::string &normalizedFace, int weight, int flags, bool symbol )
{
    if ( symbol || normalizedFace == "marlett" )
        return "marlett.ttf";

    const bool bold = IsBoldWeight( weight ) || normalizedFace.find( "bold" ) != std::string::npos;
    const bool italic = ( flags & vgui2::ISurface::FONTFLAG_ITALIC ) != 0 ||
        normalizedFace.find( "italic" ) != std::string::npos;

    if ( normalizedFace == "tahoma" || normalizedFace == "tahoma regular" ||
        normalizedFace == "default" || normalizedFace == "defaultsmall" ||
        normalizedFace == "defaultbold" || normalizedFace == "defaulttitle" ||
        normalizedFace == "defaultlarge" || normalizedFace == "defaultmedium" ||
        normalizedFace == "defaultverylarge" || normalizedFace == "defaultunderline" ||
        normalizedFace == "default very small" || normalizedFace == "defaultverysmall" ||
        normalizedFace == "default very small fallback" || normalizedFace == "defaultverysmallfallback" ||
        normalizedFace == "menutitle" || normalizedFace == "brightcontroltext" ||
        normalizedFace == "basetext" || normalizedFace == "label.textcolor" ||
        normalizedFace == "label.textbrightcolor" || normalizedFace == "label textcolor" ||
        normalizedFace == "label textbrightcolor" || normalizedFace == "trebuchet ms" )
    {
        return bold ? "VerdanaBd.ttf" : "Verdana.ttf";
    }

    if ( normalizedFace == "arial" || normalizedFace == "arial regular" )
    {
        if ( bold && italic )
            return "ArialBdIt.ttf";
        if ( bold )
            return "ArialBd.ttf";
        if ( italic )
            return "ArialIt.ttf";
        return "Arial.ttf";
    }

    if ( normalizedFace == "verdana" || normalizedFace == "verdana regular" )
    {
        if ( bold && italic )
            return "VerdanaBdIt.ttf";
        if ( bold )
            return "VerdanaBd.ttf";
        if ( italic )
            return "VerdanaIt.ttf";
        return "Verdana.ttf";
    }

    if ( normalizedFace == "firasans" || normalizedFace == "fira sans" ||
        normalizedFace == "firasans regular" || normalizedFace == "fira sans regular" )
    {
        if ( bold && italic )
            return "FiraSans-BoldItalic.ttf";
        if ( bold )
            return "FiraSans-Bold.ttf";
        if ( italic )
            return "FiraSans-Italic.ttf";
        return "FiraSans-Regular.ttf";
    }

    return KnownFontFileForFace( normalizedFace, symbol );
}

static std::string FontFaceCacheKey( const char *faceName, int weight, int flags )
{
    const std::string normalizedFace = NormalizeFaceName( faceName );
    const bool symbol = ( flags & vgui2::ISurface::FONTFLAG_SYMBOL ) != 0;
    const bool italic = ( flags & vgui2::ISurface::FONTFLAG_ITALIC ) != 0;
    return "face:" + normalizedFace +
        ":w" + std::to_string( weight ) +
        ":i" + ( italic ? "1" : "0" ) +
        ":s" + ( symbol ? "1" : "0" );
}

static bool FindIndexedFont( const std::unordered_map<std::string, std::string> &index, const char *filename, std::string &path )
{
    if ( !filename || !filename[0] )
        return false;

    const auto it = index.find( FontFileNameKey( filename ) );
    if ( it == index.end() )
        return false;

    path = it->second;
    return true;
}

static bool FindBundledFont( const char *filename, std::string &path )
{
    if ( FindIndexedFont( g_BundledFontFiles, filename, path ) )
        return true;

    const std::array<const char *, 2> dirs = { "game/font", "font" };
    for ( const char *dir : dirs )
    {
        const std::string candidate = MakeFontPath( dir, filename );
        std::fprintf( stderr, "[VGUI2-FONT] trying bundled font: %s\n", candidate.c_str() );
        if ( FontPathExists( candidate.c_str() ) )
        {
            IndexFontFile( g_BundledFontFiles, candidate );
            path = candidate;
            return true;
        }
    }

    return false;
}

static bool FindResourceFont( const char *filename, std::string &path )
{
    if ( FindIndexedFont( g_ResourceFontFiles, filename, path ) )
        return true;

    const std::array<const char *, 2> dirs = { "resource/fonts", "resource/font" };
    for ( const char *dir : dirs )
    {
        const std::string candidate = MakeFontPath( dir, filename );
        if ( FontPathExists( candidate.c_str() ) )
        {
            IndexFontFile( g_ResourceFontFiles, candidate );
            path = candidate;
            return true;
        }
    }

    return false;
}

static const EmbeddedVGUIFontFile *FindEmbeddedFont( const char *filename )
{
    if ( !filename || !filename[0] )
        return nullptr;

    size_t embeddedFontCount = 0;
    const EmbeddedVGUIFontFile *embeddedFonts = VGUI2_GetEmbeddedFontFiles( &embeddedFontCount );
    const std::string key = FontFileNameKey( filename );

    for ( size_t i = 0; i < embeddedFontCount; ++i )
    {
        if ( key == FontFileNameKey( embeddedFonts[i].filename ) )
            return &embeddedFonts[i];
    }

    return nullptr;
}

static bool ResolveFontFaceToFile( const char *requestedFaceName, int weight, int flags, std::string &path )
{
    const bool symbol = ( flags & vgui2::ISurface::FONTFLAG_SYMBOL ) != 0;
    const std::string normalizedFace = NormalizeFaceName( requestedFaceName );
    const bool explicitBold = normalizedFace.find( "bold" ) != std::string::npos;
    const bool explicitItalic = normalizedFace.find( "italic" ) != std::string::npos;

    std::fprintf( stderr,
        "[VGUI2-FONT] normalized face=%s explicitBold=%d explicitItalic=%d weight=%d symbol=%d\n",
        normalizedFace.empty() ? "<empty>" : normalizedFace.c_str(),
        explicitBold ? 1 : 0,
        explicitItalic ? 1 : 0,
        weight,
        symbol ? 1 : 0 );

    const char *knownFile = StyledKnownFontFileForFace( normalizedFace, weight, flags, symbol );
    if ( knownFile )
    {
        if ( FindBundledFont( knownFile, path ) )
        {
            std::fprintf( stderr, "[VGUI2-FONT] loaded %s font: %s\n", symbol ? "symbol" : "bundled", path.c_str() );
            return true;
        }
    }

    const std::string faceFile = normalizedFace.empty() ? std::string() : normalizedFace + ".ttf";
    if ( !faceFile.empty() && FindBundledFont( faceFile.c_str(), path ) )
    {
        std::fprintf( stderr, "[VGUI2-FONT] loaded bundled font: %s\n", path.c_str() );
        return true;
    }

    if ( !knownFile && !faceFile.empty() && FindResourceFont( faceFile.c_str(), path ) )
    {
        std::fprintf( stderr, "[VGUI2-FONT] loaded resource font: %s\n", path.c_str() );
        return true;
    }

    const char *fallbackFile = symbol ? "marlett.ttf" : StyledKnownFontFileForFace( "tahoma", weight, flags, false );
    if ( FindBundledFont( fallbackFile, path ) )
    {
        std::fprintf( stderr,
            "[VGUI2-FONT] missing requested face=%s weight=%d symbol=%d\n",
            requestedFaceName ? requestedFaceName : "<null>",
            weight,
            symbol ? 1 : 0 );
        std::fprintf( stderr, "[VGUI2-FONT] fallback to bundled font: %s\n", path.c_str() );
        return true;
    }

    return false;
}

static void BootstrapFontFiles()
{
    if ( g_FontBootstrapLoaded )
        return;

    g_FontBootstrapLoaded = true;

    std::fprintf(stderr, "[VGUI2-FONT] BootstrapFontFiles start\n");
    ScanFontDirectory( "game/font", g_BundledFontFiles );
    ScanFontDirectory( "font", g_BundledFontFiles );
    ScanFontDirectory( "resource/fonts", g_ResourceFontFiles );
    ScanFontDirectory( "resource/font", g_ResourceFontFiles );

#ifndef DISABLE_MOE_VGUI2_EXT
    std::fprintf(stderr, "[VGUI2-FONT] BootstrapFontFiles add emoji result=%d\n", g_BaseUISurface.AddCustomFontFile( "resource/font/Apple Color Emoji.ttc" ) ? 1 : 0);
#endif

    std::fprintf(stderr, "[VGUI2-FONT] BootstrapFontFiles end bundled=%zu resource=%zu\n",
        g_BundledFontFiles.size(), g_ResourceFontFiles.size());
}
}

BaseUISurface *BaseUISurfaceSingleton()
{
	return &g_BaseUISurface;
}

// TODO: implement vgui2::Dar
std::vector<vgui2::VPANEL> staticPopupList;

#define VPANEL_NORMAL	((vgui2::SurfacePlat *) NULL)
#define VPANEL_MINIMIZED ((vgui2::SurfacePlat *) 0x00000001)

#if defined( _MSC_VER ) || defined( WIN32 )
inline std::basic_string<uchar32> UnicodeToUTF32(std::wstring in) {
    auto size_in_bytes = Q_UTF16ToUTF32(in.c_str(), nullptr, 0);
    std::basic_string<uchar32> out(size_in_bytes / sizeof(uchar32), '\0');
    Q_UTF16ToUTF32(in.c_str(), out.data(), size_in_bytes);
    return out;
}
#else
inline std::basic_string<uchar32> UnicodeToUTF32(std::wstring in) {
    return in;
}
#endif

BaseUISurface::BaseUISurface() {
	_restrictedPanel = 0;
	_embeddedPanel = 0;
	m_hCurrentFont = 0;
	_cursorLocked = 0;
	m_iCurrentTexture = 0;
	m_pChromeController = NULL;
	_needMouse = true;
	_needKB = true;
	m_bVGUI2MouseControl = false;
	m_bFullScreen = false;
	m_deltay = 0;
	m_deltax = 0;
	m_bTranslateExtendedKeys = true;
	_drawColor[0] = _drawColor[1] = _drawColor[2] = _drawColor[3] = 255;
	_drawTextColor[0] = _drawTextColor[1] = _drawTextColor[2] = _drawTextColor[3] = 255;
	m_iSurfaceBounds[0] = m_iSurfaceBounds[1] = m_iSurfaceBounds[2] = m_iSurfaceBounds[3] = 0;
	_drawTextPos[0] = _drawTextPos[1] = 0;
	_translateX = _translateY = 0;

    FontManager().SetLanguage("english");
#if defined(LINUX) || defined(OSX) || defined(WIN32)
    FontManager().SetFontDataHelper( &BaseUISurface::FontDataHelper );
#endif
}

BaseUISurface::~BaseUISurface() {
	m_bAllowJavaScript = false;
}

void BaseUISurface::Init(vgui2::VPANEL embeddedPanel, IHTMLChromeController *pChromeController) {
    auto *panelInterface = vgui2::ipanel();
    if ( !panelInterface || !embeddedPanel )
        return;

    if ( !panelInterface->Client(embeddedPanel) ) {
        gEngfuncs.Con_Printf("[VGUI2-CLIENT] BaseUISurface::Init skipped: embedded panel has no client\n");
        return;
    }

    SetEmbeddedPanel(embeddedPanel);

    // Keep bootstrap minimal and deterministic for now.
    // The old font scan and HTML controller init paths can be restored once
    // we confirm they are safe in the current wasm bootstrap flow.
    m_pChromeController = pChromeController;

    BootstrapFontFiles();

    if (!m_pChromeController)
        return;

    if (pChromeController) {
        m_pChromeController->Init("htmlcache", "htmlcookies");
        m_pChromeController->SetCefThreadTargetFrameRate(60);
    }
}

void BaseUISurface::Shutdown() {
	if (m_pChromeController) {
		m_pChromeController->Shutdown();
	}
    FontManager().ClearAllFonts();
}

void BaseUISurface::RunFrame() {
	if (m_pChromeController) {
		m_pChromeController->RunFrame();
	}
}

vgui2::VPANEL BaseUISurface::GetEmbeddedPanel() {
    //gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::GetEmbeddedPanel this=%p embedded=%p\n",
    //    (void *)this, (void *)_embeddedPanel);
	return _embeddedPanel;
}

void BaseUISurface::SetEmbeddedPanel(vgui2::VPANEL panel) {
    // gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::SetEmbeddedPanel this=%p old=%p new=%p\n",
    //     (void *)this, (void *)_embeddedPanel, (void *)panel);
	_embeddedPanel = panel;
}

void SetScissorRect(int left, int top, int right, int bottom) {
	g_ScissorRect.left = left;
	g_ScissorRect.top = top;
	g_ScissorRect.right = right;
	g_ScissorRect.bottom = bottom;
}

void BaseUISurface::SetupPaintState(const PaintState_t &paintState) {
	_translateX = paintState.iTranslateX;
	_translateY = paintState.iTranslateY;
	SetScissorRect(paintState.iScissorLeft, paintState.iScissorTop, paintState.iScissorRight, paintState.iScissorBottom);
}

void BaseUISurface::PushMakeCurrent(vgui2::VPANEL panel, bool useInsets) {
	int inSets[4] = { 0, 0, 0, 0 };
	int absExtents[4];
	int clipRect[4];
	int wide, tall;

	if (useInsets) {
		vgui2::ipanel()->GetInset(panel, inSets[0], inSets[1], inSets[2], inSets[3]);
	}

	vgui2::ipanel()->GetAbsPos(panel, absExtents[0], absExtents[1]);
	vgui2::ipanel()->GetSize(panel, wide, tall);
	absExtents[2] = absExtents[0] + wide;
	absExtents[3] = absExtents[1] + tall;
	vgui2::ipanel()->GetClipRect(panel, clipRect[0], clipRect[1], clipRect[2], clipRect[3]);

	_paintStack.emplace_back();
	PaintState_t &paintState = _paintStack.back();
	paintState.vPanel = panel;

	paintState.iTranslateX = inSets[0] + absExtents[0] - m_iSurfaceBounds[0];
	paintState.iTranslateY = inSets[1] + absExtents[1] - m_iSurfaceBounds[1];

	paintState.iScissorLeft = clipRect[0] - m_iSurfaceBounds[0];
	paintState.iScissorTop = clipRect[1] - m_iSurfaceBounds[1];
	paintState.iScissorRight = clipRect[2] - m_iSurfaceBounds[0];
	paintState.iScissorBottom = clipRect[3] - m_iSurfaceBounds[1];

	SetupPaintState(paintState);
#ifdef USE_IMGUI_SURFACE
    ImGui_Surface_EnableFullScreenScissor();
#endif
}

void BaseUISurface::PopMakeCurrent(vgui2::VPANEL panel) {
    DrawFlushText();

	if (!_paintStack.size()) {
		return;
	}

	_paintStack.pop_back();

	if (_paintStack.size()) {
		SetupPaintState(_paintStack.back());
	}
#ifdef USE_IMGUI_SURFACE
    ImGui_Surface_DisableScissor();
#endif
}

void BaseUISurface::DrawSetColor(int r, int g, int b, int a) {
	_drawColor[0] = r;
	_drawColor[1] = g;
	_drawColor[2] = b;
	_drawColor[3] = 255 - a;
}

void BaseUISurface::DrawSetColor(Color col) {
	col.GetColor(_drawColor[0], _drawColor[1], _drawColor[2], _drawColor[3]);
	_drawColor[3] = 255 - _drawColor[3];
}

void BaseUISurface::InitVertex(vpoint_t &vertex, int x, int y, float u, float v) {
	vertex.point[0] = x + _translateX;
	vertex.point[1] = y + _translateY;
	vertex.coord[0] = u;
	vertex.coord[1] = v;
}

inline float InterpTCoord(float val, float mins, float maxs, float tMin, float tMax) {
	float	flPercent;

	if (mins != maxs) {
		flPercent = (float)(val - mins) / (maxs - mins);
	} else {
		flPercent = 0.5f;
	}

	return tMin + (tMax - tMin) * flPercent;
}

qboolean ClipRect(const vpoint_t &inUL, const vpoint_t &inLR, vpoint_t *pOutUL, vpoint_t *pOutLR) {
	if (g_bScissor) {
		if (g_ScissorRect.left > inUL.point[0]) {
			pOutUL->point[0] = g_ScissorRect.left;
		} else {
			pOutUL->point[0] = inUL.point[0];
		}

		if (g_ScissorRect.right <= inLR.point[0]) {
			pOutLR->point[0] = g_ScissorRect.right;
		} else {
			pOutLR->point[0] = inLR.point[0];
		}

		if (g_ScissorRect.top > inUL.point[1]) {
			pOutUL->point[1] = g_ScissorRect.top;
		} else {
			pOutUL->point[1] = inUL.point[1];
		}

		if (g_ScissorRect.bottom <= inLR.point[1]) {
			pOutLR->point[1] = g_ScissorRect.bottom;
		} else {
			pOutLR->point[1] = inLR.point[1];
		}

		if ((pOutUL->point[0] > pOutLR->point[0]) || (pOutUL->point[1] > pOutLR->point[1])) {
			return false;
		}

		pOutUL->coord[0] = InterpTCoord(pOutUL->point[0], inUL.point[0], inLR.point[0], inUL.coord[0], inLR.coord[0]);
		pOutLR->coord[0] = InterpTCoord(pOutLR->point[0], inUL.point[0], inLR.point[0], inUL.coord[0], inLR.coord[0]);
		pOutUL->coord[1] = InterpTCoord(pOutUL->point[1], inUL.point[1], inLR.point[1], inUL.coord[1], inLR.coord[1]);
		pOutLR->coord[1] = InterpTCoord(pOutLR->point[1], inUL.point[1], inLR.point[1], inUL.coord[1], inLR.coord[1]);
	} else {
		*pOutUL = inUL;
		*pOutLR = inLR;
	}

	return true;
}

void BaseUISurface::DrawFilledRect(int x0, int y0, int x1, int y1) {
	vpoint_t rect[2];
	vpoint_t clippedRect[2];

	if (_drawColor[3] >= 255) {
		return;
	}

	InitVertex(rect[0], x0, y0, 0, 0);
	InitVertex(rect[1], x1, y1, 0, 0);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1])) {
		return;
	}
#ifdef USE_IMGUI_SURFACE
    ImGui_Surface_SetRenderMode(kRenderTransAlpha);
    SPR_AdjustSize(&rect[0].point[0], &rect[0].point[1], &rect[1].point[0], &rect[1].point[1]);
    ImGui_Surface_DrawRectangle(rect[0].point[0], rect[0].point[1], rect[1].point[0] - rect[0].point[0], rect[1].point[1] - rect[0].point[1], _drawColor[0], _drawColor[1], _drawColor[2],  255 - _drawColor[3]);
#else
    g_api->SetupDrawingRect(_drawColor);
    g_api->EnableTexture(false);
    g_api->DrawQuad(&clippedRect[0], &clippedRect[1]);
    g_api->EnableTexture(true);
#endif
}
void BaseUISurface::DrawOutlinedRect(int x0, int y0, int x1, int y1) {
	if (_drawColor[3] >= 255) {
		return;
	}

	DrawFilledRect(x0, y0, x1, y0 + 1);
	DrawFilledRect(x0, y1 - 1, x1, y1);
	DrawFilledRect(x0, y0 + 1, x0 + 1, y1 - 1);
	DrawFilledRect(x1 - 1, y0 + 1, x1, y1 - 1);
}

void BaseUISurface::DrawLine(int, int, int, int) {
	vgui2::gEngfuncs.Con_DPrintf((char *)"DrawLine ");
}

void BaseUISurface::DrawPolyLine(int *, int *, int) {
	vgui2::gEngfuncs.Con_DPrintf((char *)"DrawPolyLine ");
}

void BaseUISurface::DrawSetTextFont(vgui2::HFont font) {
    DrawFlushText();
	m_hCurrentFont = font;
}

void BaseUISurface::DrawSetTextColor(int r, int g, int b, int a) {
    DrawFlushText();
	_drawTextColor[0] = r;
	_drawTextColor[1] = g;
	_drawTextColor[2] = b;
	_drawTextColor[3] = a;
}

void BaseUISurface::DrawSetTextColor(Color col) {
    DrawFlushText();
	col.GetColor(_drawTextColor[0], _drawTextColor[1], _drawTextColor[2], _drawTextColor[3]);
}

void BaseUISurface::DrawSetTextPos(int x, int y) {
	_drawTextPos[0] = x;
	_drawTextPos[1] = y;
}

void BaseUISurface::DrawGetTextPos(int &x, int &y) {
	x = _drawTextPos[0];
	y = _drawTextPos[1];
}

#if defined( _MSC_VER ) || defined( WIN32 )
void BaseUISurface::DrawPrintText(const wchar_t *text, int textlen) {
    std::basic_string<uchar32> out = UnicodeToUTF32(std::wstring(text, text + textlen));
    return DrawPrintText(out.c_str(), out.size());
}
#endif

void BaseUISurface::DrawPrintText(const uchar32 *text, int textlen) {
    if (!text)
        return;

    if (!m_hCurrentFont)
        return;

    int x, y;
    DrawGetTextPos(x, y);

    const int textStartX = x;
    const int textStartY = y;
    int drawX = x + _translateX;
    int drawY = y + _translateY;

    int iTall = GetFontTall(m_hCurrentFont);
    int iLastTexId = -1;

    int iCount = 0;
    vpoint_t *pQuads = (vpoint_t *)stackalloc((2 * textlen) * sizeof(vpoint_t));

    int iTotalWidth = 0;

    for (int i = 0; i < textlen; ++i)
    {
        uchar32 ch = text[i];

        bool bUnderlined = FontManager().GetFontUnderlined(m_hCurrentFont);
        int abcA, abcB, abcC;
        GetCharABCwide(m_hCurrentFont, ch, abcA, abcB, abcC);
        iTotalWidth += abcA;

        int iWide = abcB;

        if (bUnderlined)
        {
            iWide += (abcA + abcC);
            drawX-= abcA;
        }
#ifndef DISABLE_MOE_VGUI2_EXT
        if(IsEmojiChar(ch))
        {
            int iTexId = 0;
            float texCoords[4];

            if (!m_FontTextureCache.GetTextureForChar(m_hCurrentFont, ch, &iTexId, texCoords))
            {
                continue;
            }

            DrawSetTexture(iTexId);

            vpoint_t ul;
            vpoint_t lr;

            ul.point[0] = drawX + iTotalWidth;
            ul.point[1] = drawY + iTall / 2 - iWide / 2;
            lr.point[0] = ul.point[0] + iWide;
            lr.point[1] = ul.point[1] + iWide;

            ul.coord[0] = texCoords[0];
            ul.coord[1] = texCoords[1];
            lr.coord[0] = texCoords[2];
            lr.coord[1] = texCoords[3];

            DrawQuadBlend(ul, lr, white_color);
        }
        else
#endif
        if (!iswspace(ch) || bUnderlined)
        {
            int iTexId = 0;
            float texCoords[4];

            if (!m_FontTextureCache.GetTextureForChar(m_hCurrentFont, ch, &iTexId, texCoords))
            {
                continue;
            }

            Assert(texCoords);

            if (iTexId != iLastTexId)
            {
                if (iCount)
                {
                    DrawSetTexture(iLastTexId);
                    DrawQuadArray(iCount, pQuads, _drawTextColor);
                    iCount = 0;
                }

                iLastTexId = iTexId;
            }

            vpoint_t &ul = pQuads[2 * iCount];
            vpoint_t &lr = pQuads[2 * iCount + 1];
            iCount++;

            ul.point[0] = drawX + iTotalWidth;
            ul.point[1] = drawY;
            lr.point[0] = ul.point[0] + iWide;
            lr.point[1] = ul.point[1] + iTall;

            ul.coord[0] = texCoords[0];
            ul.coord[1] = texCoords[1];
            lr.coord[0] = texCoords[2];
            lr.coord[1] = texCoords[3];
        }

        iTotalWidth += iWide + abcC;

        if (bUnderlined)
        {
            iTotalWidth -= abcC;
        }
    }

    if (iCount)
    {
        DrawSetTexture(iLastTexId);
        DrawQuadArray(iCount, pQuads, _drawTextColor);
    }

    DrawSetTextPos(textStartX + iTotalWidth, textStartY);
    stackfree(pQuads);
}

void BaseUISurface::DrawUnicodeChar(uchar32 ch) {
    CharRenderInfo info;
    info.additive = false;

    if (DrawGetUnicodeCharRenderInfo(ch, info))
        DrawRenderCharFromInfo(info);
}

void BaseUISurface::DrawUnicodeCharAdd(uchar32 ch) {
    CharRenderInfo info;
    info.additive = true;

    if (DrawGetUnicodeCharRenderInfo(ch, info))
        DrawRenderCharFromInfo(info);
}

class vgui2::IHTML *BaseUISurface::CreateHTMLWindow(class vgui2::IHTMLEvents *, vgui2::VPANEL) {
	return NULL;
}

void BaseUISurface::PaintHTMLWindow(class vgui2::IHTML *) {
	// TODO
}

void BaseUISurface::DeleteHTMLWindow(class vgui2::IHTML *) {
	// TODO
}

void BaseUISurface::DrawSetTextureFile(int id, const char  * filename, int hardwareFilter, bool forceReload) {
	char name[512];
	if (filename && (Q_stristr(filename, ".tga") || Q_stristr(filename, ".bmp") || Q_stristr(filename, ".pic")))
		snprintf(name, sizeof(name), "%s", filename);
	else
		snprintf(name, sizeof(name), "%s.tga", filename ? filename : "");
	rgbdata_t *pic = nullptr;
	std::fprintf(stderr,
		"[VGUI2-TRACE] BaseUISurface::DrawSetTextureFile enter id=%d filename='%s' tga='%s' forceReload=%d hardwareFilter=%d\n",
		id,
		filename ? filename : "<null>",
		name,
		forceReload ? 1 : 0,
		hardwareFilter);


	// image_ref pic;
    // pic = FS_LoadImage( name, nullptr, 0 );


	if ( !vgui2::gEngfuncs.FS_LoadImage )
	{
		std::fprintf( stderr, "[VGUI2-TRACE] BaseUISurface::DrawSetTextureFile missing FS_LoadImage filename='%s'\n", name );
		DrawSetTexture(id);
		return;
	}

	pic = vgui2::gEngfuncs.FS_LoadImage( name, nullptr, 0 );
	if (!pic)
	{
		char lowerName[512];
		Q_strncpy(lowerName, name, sizeof(lowerName));
		for (char *p = lowerName; *p; ++p)
			*p = (char)tolower((unsigned char)*p);

		if (Q_stricmp(lowerName, name))
		{
			std::fprintf(stderr,
				"[VGUI2-TRACE] BaseUISurface::DrawSetTextureFile retry-lower id=%d filename='%s'\n",
				id,
				lowerName);
			pic = vgui2::gEngfuncs.FS_LoadImage(lowerName, nullptr, 0);
		}
	}

	if (!pic)
	{
		std::fprintf(stderr,
			"[VGUI2-TRACE] BaseUISurface::DrawSetTextureFile tga miss id=%d filename='%s'\n",
			id,
			name);
		static const unsigned char s_TransparentPixel[4] = {0, 0, 0, 0};
		DrawSetTextureRGBA(id, s_TransparentPixel, 1, 1, hardwareFilter, forceReload);
		return;
	}

	std::fprintf(stderr,
		"[VGUI2-TRACE] BaseUISurface::DrawSetTextureFile loaded id=%d filename='%s' size=%dx%d\n",
		id,
		name,
		pic->width,
		pic->height);

	DrawSetTextureRGBA(id, pic->buffer, pic->width, pic->height, hardwareFilter, forceReload);
}

void BaseUISurface::DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload) {
	if (!g_api || !g_api->UploadTexture)
	{
		std::fprintf(stderr, "[VGUI2-TRACE] BaseUISurface::DrawSetTextureRGBA missing uploader id=%d size=%dx%d\n", id, wide, tall);
		return;
	}

	DrawFlushText();
#ifdef USE_IMGUI_SURFACE
	ImGui_Surface_Flush();
#endif

	g_api->UploadTexture(id, (const char *)rgba, wide, tall);
	DrawSetTexture(id);
}

void BaseUISurface::DrawSetTexture(int id) {
    if(m_iCurrentTexture != id)
    {
        m_iCurrentTexture = id;
        DrawFlushText();
#ifdef USE_IMGUI_SURFACE
        ImGui_Surface_Flush();
#endif
    }
#ifdef USE_IMGUI_SURFACE
    // nothing
#else
	if (g_api && g_api->BindTexture)
	{
		g_api->BindTexture(id);
	}
#endif
}

void BaseUISurface::DrawGetTextureSize(int id, int &wide, int &tall) {
	int width, height;
	g_api->BindTexture(id);
	g_api->GetTextureSizes(&width, &height);
	g_api->BindTexture(m_iCurrentTexture);
	wide = width;
	tall = height;
}
void BaseUISurface::DrawTexturedRect(int x0, int y0, int x1, int y1) {
	vpoint_t rect[2];
	vpoint_t clippedRect[2];

	InitVertex(rect[0], x0, y0, 0, 0);
	InitVertex(rect[1], x1, y1, 1, 1);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1])) {
		return;
	}
	// Old behaviour:
	// g_api->SetupDrawingImage(_drawColor);
	// g_api->DrawQuad(&clippedRect[0], &clippedRect[1]);

#ifdef USE_IMGUI_SURFACE
    int id = m_iCurrentTexture;
    if( id > 0 && id < VGUI_MAX_TEXTURES && g_textures[id] )
    {
        ImGui_Surface_SetRenderMode(kRenderTransTexture);
        SPR_AdjustSize(&clippedRect[0].point[0], &clippedRect[0].point[1], &clippedRect[1].point[0], &clippedRect[1].point[1]);
        ImGui_Surface_DrawImage(g_textures[id], clippedRect[0].point[0], clippedRect[0].point[1], clippedRect[1].point[0], clippedRect[1].point[1], clippedRect[0].coord[0], clippedRect[0].coord[1], clippedRect[1].coord[0], clippedRect[1].coord[1], _drawColor[0], _drawColor[1], _drawColor[2],  255 - _drawColor[3]);
    }
#else
	if (!g_api || !g_api->SetupDrawingImage || !g_api->DrawQuad)
	{
		// std::fprintf(stderr, "[VGUI2-TRACE] BaseUISurface::DrawTexturedRect fallback x0=%d y0=%d x1=%d y1=%d gapi=%p\n",
		// 	x0, y0, x1, y1, (void *)g_api);
		return;
	}

	g_api->SetupDrawingImage(_drawColor);
	g_api->DrawQuad(&clippedRect[0], &clippedRect[1]);
#endif
}

bool BaseUISurface::IsTextureIDValid(int) {
	return true;
}

int BaseUISurface::CreateNewTextureID(bool procedural) {
	static int s_NextFallbackTextureId = 1;

	if ( g_api && g_api->GenerateTexture )
	{
		return g_api->GenerateTexture();
	}

	// Some wasm bootstrap paths do not wire the texture factory callback yet.
	// Hand out a local unique id so scheme image uploads can continue without
	// trapping on a null function pointer.
	// std::fprintf(stderr, "[VGUI2-TRACE] BaseUISurface::CreateNewTextureID fallback procedural=%d next=%d\n",
	// 	procedural ? 1 : 0, s_NextFallbackTextureId);
	return s_NextFallbackTextureId++;
}

void BaseUISurface::GetScreenSize(int &wide, int &tall) {
    SCREENINFO screenInfo;
    screenInfo.iSize = sizeof( SCREENINFO );
    vgui2::gEngfuncs.pfnGetScreenInfo( &screenInfo );
    wide = screenInfo.iWidth;
    tall = screenInfo.iHeight;
}

void BaseUISurface::SetAsTopMost(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::BringToFront(vgui2::VPANEL panel) {
	vgui2::ipanel()->MoveToFront(panel);

	if (vgui2::ipanel()->IsPopup(panel)) {
		MovePopupToFront(panel);
	}
}

void BaseUISurface::SetForegroundWindow(vgui2::VPANEL panel) {
	BringToFront(panel);
}

void BaseUISurface::SetPanelVisible(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::SetMinimized(vgui2::VPANEL panel, bool state) {
	if (state) {
		vgui2::ipanel()->SetPlat(panel, VPANEL_MINIMIZED);
		vgui2::ipanel()->SetVisible(panel, false);
	} else {
		vgui2::ipanel()->SetPlat(panel, VPANEL_NORMAL);
	}
}

bool BaseUISurface::IsMinimized(vgui2::VPANEL panel) {
	return vgui2::ipanel()->Plat(panel) == VPANEL_MINIMIZED;
}

void BaseUISurface::FlashWindow(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::SetTitle(vgui2::VPANEL, const wchar_t  *) {
	//
}

void BaseUISurface::SetAsToolBar(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::CreatePopup(vgui2::VPANEL panel, bool minimised, bool showTaskbarIcon, bool disabled, bool mouseInput, bool kbInput) {
    gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::CreatePopup enter panel=%p embedded=%p parent=%p popup=%d mouse=%d kb=%d\n",
        (void *)panel,
        (void *)GetEmbeddedPanel(),
        (void *)vgui2::ipanel()->GetParent(panel),
        vgui2::ipanel()->IsPopup(panel),
        mouseInput,
        kbInput);
	if (!vgui2::ipanel()->GetParent(panel)) {
		vgui2::ipanel()->SetParent(panel, GetEmbeddedPanel());
	}

	vgui2::ipanel()->SetPopup(panel, true);
	vgui2::ipanel()->SetKeyBoardInputEnabled(panel, kbInput);
	vgui2::ipanel()->SetMouseInputEnabled(panel, mouseInput);
    gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::CreatePopup after setup panel=%p parent=%p popup=%d\n",
        (void *)panel,
        (void *)vgui2::ipanel()->GetParent(panel),
        vgui2::ipanel()->IsPopup(panel));

	// TODO: implement vgui2::Dar
    std::vector<vgui2::VPANEL>::iterator it = std::find(staticPopupList.begin(), staticPopupList.end(), panel);
    if(it == staticPopupList.end())
	    staticPopupList.push_back(panel);
}

void BaseUISurface::SwapBuffers(vgui2::VPANEL) {
	//
}

void BaseUISurface::Invalidate(vgui2::VPANEL) {
	//
}

void BaseUISurface::SetCursor(vgui2::HCursor cursor) {
	_currentCursor = cursor;
	// g_api->CursorSelect((VGUI_DefaultCursor)cursor);
	if (!g_api)
	{
		//gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::SetCursor skip because g_api is null cursor=%d\n", (int)cursor);
		return;
	}

	if (!g_api->CursorSelect)
	{
		//gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::SetCursor skip because CursorSelect is null cursor=%d api=%p\n",
		//	(int)cursor, (void *)g_api);
		return;
	}

	g_api->CursorSelect((VGUI_DefaultCursor)cursor);
}

bool BaseUISurface::IsCursorVisible() {
	return _currentCursor != vgui2::dc_none;
}

void BaseUISurface::ApplyChanges() {
	//
}

bool BaseUISurface::IsWithin(int, int) {
	return true;
}

bool BaseUISurface::HasFocus() {
	return true;
}

bool BaseUISurface::SupportsFeature(SurfaceFeature_e feature) {
	return feature < ESCAPE_KEY;
}

void BaseUISurface::RestrictPaintToSinglePanel(vgui2::VPANEL panel) {
	_restrictedPanel = panel;
	vgui2::input()->SetAppModalSurface(panel);
}

void BaseUISurface::SetModalPanel(vgui2::VPANEL) {
	//
}

vgui2::VPANEL BaseUISurface::GetModalPanel() {
	return _restrictedPanel;
}

void BaseUISurface::UnlockCursor() {
	_cursorLocked = false;
}

void BaseUISurface::LockCursor() {
	_cursorLocked = true;
}

void BaseUISurface::SetTranslateExtendedKeys(bool state) {
	m_bTranslateExtendedKeys = state;
}

vgui2::VPANEL BaseUISurface::GetTopmostPopup() {
	if (staticPopupList.size() > 0) {
		return staticPopupList.back();
	}

	return 0;
}

void BaseUISurface::SetTopLevelFocus(vgui2::VPANEL subFocus) {
	while (subFocus) {
		if (vgui2::ipanel()->IsPopup(subFocus)) {
			BringToFront(subFocus);
			break;
		}

		subFocus = vgui2::ipanel()->GetParent(subFocus);
	}
}

vgui2::HFont BaseUISurface::CreateFont() {
    BootstrapFontFiles();
    return FontManager().CreateFont();
}

bool BaseUISurface::AddGlyphSetToFont(vgui2::HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange) {
    BootstrapFontFiles();
    const bool added = FontManager().SetFontGlyphSet(font, windowsFontName, tall, weight, blur, scanlines, flags, lowRange, highRange);
    if (added)
        m_FontTextureCache.InvalidateFont(font);
    return added;
}

bool BaseUISurface::AddCustomFontFile(const char *fontFileName) {
    char fullPath[ 4096 ];

    vgui2::filesystem()->GetLocalPath( fontFileName, fullPath, sizeof( fullPath ) );

    CUtlSymbol symbol( fontFileName );

    m_CustomFontFileNames[ m_CustomFontFileNames.AddToTail() ] = symbol;

#if defined(LINUX) || defined(OSX) || defined(WIN32)
    int size;
    const bool ok = BaseUISurface::FontDataHelper( nullptr, size, fontFileName, 0, 0 ) != nullptr;
    std::fprintf(stderr, "[VGUI2-TRACE] BaseUISurface::AddCustomFontFile fontFile='%s' fullPath='%s' loaded=%d size=%d\n",
        fontFileName ? fontFileName : "<null>", fullPath, ok ? 1 : 0, ok ? size : 0);
    if ( ok )
        return true;
    return false;
#elif defined WIN32
    return AddFontResourceExA(fullPath, FR_PRIVATE, nullptr) > 0 ||
        AddFontResourceA(fullPath) > 0;
#endif
}

int BaseUISurface::GetFontTall(vgui2::HFont font) {
    return FontManager().GetFontTall(font);
}

void BaseUISurface::GetCharABCwide(vgui2::HFont font, int ch, int &a, int &b, int &c) {
    return FontManager().GetCharABCwide(font, ch, a, b, c);
}

int BaseUISurface::GetCharacterWidth(vgui2::HFont font, int ch) {
    return FontManager().GetCharacterWidth(font, ch);
}

#if defined( _MSC_VER ) || defined( WIN32 )
void BaseUISurface::GetTextSize(vgui2::HFont font, const wchar_t *text, int &wide, int &tall) {
    return GetTextSize(font, UnicodeToUTF32(text).c_str(), wide, tall);
}
#endif

void BaseUISurface::GetTextSize(vgui2::HFont font, const uchar32 *text, int &wide, int &tall) {
    return FontManager().GetTextSize(font, text, wide, tall);
}

vgui2::VPANEL BaseUISurface::GetNotifyPanel() {
	return 0;
}

void BaseUISurface::SetNotifyIcon(vgui2::VPANEL, vgui2::HTexture, vgui2::VPANEL, const char  *) {
	//
}

void BaseUISurface::PlaySound(const char *fileName) {
	vgui2::gEngfuncs.pfnPlaySoundByName((char *)fileName, VOL_NORM);
}

int BaseUISurface::GetPopupCount() {
	// TODO: implement vgui2::Dar
	return staticPopupList.size();
}

vgui2::VPANEL BaseUISurface::GetPopup(int index) {
	if (index >= 0 && index < GetPopupCount()) {
		// TODO: implement vgui2::Dar
		return staticPopupList[index];
	}

	return 0;
}

bool BaseUISurface::ShouldPaintChildPanel(vgui2::VPANEL panel) {
	if (_restrictedPanel && !vgui2::ipanel()->HasParent(panel, _restrictedPanel)) {
		return false;
	}

	bool isPopup = vgui2::ipanel()->IsPopup(panel);

	if (isPopup) {
		for (int i = 0; i < GetPopupCount(); ++i) {
			if (panel == GetPopup(i)) {
				vgui2::ipanel()->Render_SetPopupVisible(panel, true);
				break;
			}
		}
	}

	return !isPopup;
}

bool BaseUISurface::RecreateContext(vgui2::VPANEL) {
	return true;
}

void BaseUISurface::AddPanel(vgui2::VPANEL panel) {
    gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::AddPanel panel=%p embedded=%p popup=%d parent=%p client=%p\n",
        (void *)panel,
        (void *)GetEmbeddedPanel(),
        vgui2::ipanel()->IsPopup(panel),
        (void *)vgui2::ipanel()->GetParent(panel),
        (void *)vgui2::ipanel()->Client(panel));
	if (vgui2::ipanel()->IsPopup(panel)) {
		CreatePopup(panel, false, false, false, true, true);
	}
}

void BaseUISurface::ReleasePanel(vgui2::VPANEL panel) {
    gEngfuncs.Con_Printf("[VGUI2-TRACE] BaseUISurface::ReleasePanel panel=%p embedded=%p popup=%d parent=%p\n",
        (void *)panel,
        (void *)GetEmbeddedPanel(),
        vgui2::ipanel()->IsPopup(panel),
        (void *)vgui2::ipanel()->GetParent(panel));
	// TODO: implement vgui2::Dar
	std::vector<vgui2::VPANEL>::iterator it = std::find(staticPopupList.begin(), staticPopupList.end(), panel);

	if (it != staticPopupList.end()) {
		staticPopupList.erase(it);
	}

	if (_restrictedPanel == panel) {
		_restrictedPanel = 0;
	}
}

void BaseUISurface::MovePopupToFront(vgui2::VPANEL panel) {
	// TODO: implement vgui2::Dar
	std::vector<vgui2::VPANEL>::iterator it = std::find(staticPopupList.begin(), staticPopupList.end(), panel);

	if (it != staticPopupList.end()) {
		staticPopupList.erase(it);
	}

	staticPopupList.push_back(panel);
}

void BaseUISurface::MovePopupToBack(vgui2::VPANEL panel) {
	// TODO: implement vgui2::Dar
	std::vector<vgui2::VPANEL>::iterator it = std::find(staticPopupList.begin(), staticPopupList.end(), panel);

	if (it != staticPopupList.end()) {
		staticPopupList.erase(it);
	}

	staticPopupList.insert(staticPopupList.begin(), panel);
}

void BaseUISurface::SolveTraverse(vgui2::VPANEL panel, bool forceApplySchemeSettings) {
	// Original trace:
	// std::fprintf(stderr, "[VGUI2-TRACE] surface->SolveTraverse enter panel=%p forceApplySchemeSettings=%d\n",
	// 	(void *)panel, forceApplySchemeSettings ? 1 : 0);
	InternalSchemeSettingsTraverse(panel, forceApplySchemeSettings);
	InternalThinkTraverse(panel);
	InternalSolveTraverse(panel);
	// Original trace:
	// std::fprintf(stderr, "[VGUI2-TRACE] surface->SolveTraverse exit panel=%p\n", (void *)panel);
}

void BaseUISurface::PaintTraverse(vgui2::VPANEL panel) {
	if (!vgui2::ipanel()->IsVisible(panel)) {
		return;
	}

	if (panel != GetEmbeddedPanel()) {
		vgui2::ipanel()->PaintTraverse(panel, true, true);
		return;
	}

	if (_restrictedPanel) {
		panel = _restrictedPanel;
	}

	for (int i = 0; i < GetPopupCount(); ++i) {
		vgui2::ipanel()->Render_SetPopupVisible(GetPopup(i), false);
	}

	vgui2::ipanel()->PaintTraverse(panel, true, true);

	for (int i = 0; i < GetPopupCount(); ++i) {
		vgui2::VPANEL pop = GetPopup(i);

		if (vgui2::ipanel()->Render_GetPopupVisible(pop)) {
			vgui2::ipanel()->PaintTraverse(pop, true, true);
		}
	}
}
void BaseUISurface::EnableMouseCapture(vgui2::VPANEL, bool) {
	//
}

void BaseUISurface::GetWorkspaceBounds(int &x, int &y, int &wide, int &tall) {
	x = 0;
	y = 0;
	vgui2::ipanel()->GetSize(_embeddedPanel, wide, tall);
}

void BaseUISurface::GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall) {
	x = m_iSurfaceBounds[0];
	y = m_iSurfaceBounds[1];
	wide = m_iSurfaceBounds[2];
	tall = m_iSurfaceBounds[3];
}

void BaseUISurface::GetProportionalBase(int &width, int &height) {
	width = BASE_WIDTH;
	height = BASE_HEIGHT;
}

void BaseUISurface::CalculateMouseVisible() {
	if (m_bIgnoreMouseVisCalc) {
		return;
	}

	_needMouse = false;
	_needKB = false;

	for (int i = 0; i < GetPopupCount(); i++) {
		vgui2::VPANEL pop = GetPopup(i);

		if (!pop) {
			continue;
		}

		bool isVisible = vgui2::ipanel()->IsVisible(pop);
		vgui2::VPANEL p = vgui2::ipanel()->GetParent(pop);

		while (p && isVisible) {
			if (vgui2::ipanel()->IsVisible(p) == false) {
				isVisible = false;
				break;
			}

			p = vgui2::ipanel()->GetParent(p);
		}

		if (isVisible) {
			_needMouse = _needMouse || vgui2::ipanel()->IsMouseInputEnabled(pop);
			_needKB = _needKB || vgui2::ipanel()->IsKeyBoardInputEnabled(pop);
		}
	}

	UnlockCursor();

	if (_needMouse) {
		SetCursor(vgui2::dc_arrow);
	} else {
		SetCursor(vgui2::dc_none);
		LockCursor();
	}

	g_api->SetVisible(_needMouse);
}

bool BaseUISurface::NeedKBInput() {
	return _needKB;
}

bool BaseUISurface::HasCursorPosFunctions() {
	return true;
}

void BaseUISurface::SurfaceGetCursorPos(int &x, int &y) {
	int _x, _y;
	g_api->GetCursorPos(&_x, &_y);
	x = _x;
	y = _y;
}

void BaseUISurface::SurfaceSetCursorPos(int x, int y) {
	vgui2::gEngfuncs.Con_DPrintf((char *)"SurfaceSetCursorPos ");
}

bool BaseUISurface::VGUI2MouseControl() {
	return m_bVGUI2MouseControl;
}

void BaseUISurface::SetVGUI2MouseControl(bool state) {
	m_bVGUI2MouseControl = state;
}

void BaseUISurface::DrawTexturedPolygon(vgui2::Vertex_t *pVertices, int n) {
    if( _drawColor[ 3 ] == 255 )
        return;

    g_api->EnableTexture(false);
    g_api->SetupDrawingImage(_drawColor);

    auto pNext = pVertices;

    vgui2::gEngfuncs.pTriAPI->Begin( TRI_POLYGON );

    for( int i = 0; i < n; ++i, ++pNext )
    {
        vpoint_t point;
        InitVertex(point, pNext->m_Position[0], pNext->m_Position[1], pNext->m_TexCoord[0], pNext->m_TexCoord[1]);
        SPR_AdjustSize(&point.point[0], &point.point[1], nullptr, nullptr);

        vgui2::gEngfuncs.pTriAPI->TexCoord2f( point.coord[0], point.coord[1] );
        vgui2::gEngfuncs.pTriAPI->Vertex3f( point.point[0], point.point[1], 0 );
    }

    vgui2::gEngfuncs.pTriAPI->End();
    g_api->EnableTexture(true);
}

int BaseUISurface::GetFontAscent(vgui2::HFont font, uchar32 ch) {
	return FontManager().GetFontAscent(font, ch);
}

void BaseUISurface::SetAllowHTMLJavaScript(bool state) {
	m_bAllowJavaScript = state;
}

void BaseUISurface::SetLanguage(const char *pchLang) {
	if (pchLang) {
		strncpy(m_szLanguage, pchLang, sizeof(m_szLanguage));
	} else {
		strcpy(m_szLanguage, "english");
	}
}

const char *BaseUISurface::GetLanguage() {
	return m_szLanguage;
}

bool BaseUISurface::DeleteTextureByID(int) {
	return false;
}

void BaseUISurface::DrawUpdateRegionTextureBGRA(int, int, int, const unsigned char  *, int, int) {
	vgui2::gEngfuncs.Con_DPrintf((char *)"DrawUpdateRegionTextureBGRA ");
}

void BaseUISurface::DrawSetTextureBGRA(int id, const unsigned char *bgra, int wide, int tall) {
	unsigned char *rgba = (unsigned char *)calloc(wide * tall, sizeof(unsigned char) * 4);

	for (int i = 0; i < tall; ++i) {
		for (int j = 0; j < wide * 4; j += 4) {
			*(rgba + wide * i * 4 + j) = *(bgra + wide * i * 4 + j + 2);
			*(rgba + wide * i * 4 + j + 1) = *(bgra + wide * i * 4 + j + 1);
			*(rgba + wide * i * 4 + j + 2) = *(bgra + wide * i * 4 + j);
			*(rgba + wide * i * 4 + j + 3) = *(bgra + wide * i * 4 + j + 3);
		}
	}

	DrawSetTextureRGBA(id, (const unsigned char *)rgba, wide, tall, false, false);
	free(rgba);
}

void BaseUISurface::CreateBrowser(vgui2::VPANEL panel, class IHTMLResponses *pBrowser, bool bPopupWindow, const char *pchUserAgentIdentifier) {
	if (m_pChromeController) {
		m_pChromeController->CreateBrowser(pBrowser, bPopupWindow, pchUserAgentIdentifier);
	}
}

void BaseUISurface::RemoveBrowser(vgui2::VPANEL panel, class IHTMLResponses *pBrowser) {
	if (m_pChromeController) {
		m_pChromeController->RemoveBrowser(pBrowser);
	}
}

void BaseUISurface::IgnoreMouseVisibility(bool state) {
	m_bIgnoreMouseVisCalc = state;
}

class IHTMLChromeController *BaseUISurface::AccessChromeHTMLController() {
	return m_pChromeController;
}

void BaseUISurface::SetScreenBounds(int x, int y, int wide, int tall) {
	m_iSurfaceBounds[0] = x;
	m_iSurfaceBounds[1] = y;
	m_iSurfaceBounds[2] = wide;
	m_iSurfaceBounds[3] = tall;
}

void BaseUISurface::InternalSchemeSettingsTraverse(vgui2::VPANEL panel, bool forceApplySchemeSettings) {
	for (int i = 0; i < vgui2::ipanel()->GetChildCount(panel); ++i) {
		vgui2::VPANEL child = vgui2::ipanel()->GetChild(panel, i);

		if (vgui2::ipanel()->IsVisible(child) || forceApplySchemeSettings) {
			InternalSchemeSettingsTraverse(child, forceApplySchemeSettings);
		}
	}

	vgui2::ipanel()->PerformApplySchemeSettings(panel);
}

void BaseUISurface::InternalThinkTraverse(vgui2::VPANEL panel) {
	vgui2::ipanel()->Think(panel);

	for (int i = 0; i < vgui2::ipanel()->GetChildCount(panel); ++i) {
		vgui2::VPANEL child = vgui2::ipanel()->GetChild(panel, i);

		if (vgui2::ipanel()->IsVisible(child)) {
			InternalThinkTraverse(child);
		}
	}
}

void BaseUISurface::InternalSolveTraverse(vgui2::VPANEL panel) {
	vgui2::ipanel()->Solve(panel);

	for (int i = 0; i < vgui2::ipanel()->GetChildCount(panel); ++i) {
		vgui2::VPANEL child = vgui2::ipanel()->GetChild(panel, i);

		if (vgui2::ipanel()->IsVisible(child)) {
			InternalSolveTraverse(child);
		}
	}
}

void BaseUISurface::DrawSetTextureRGBAWithAlphaChannel(int id, const byte* rgba, int wide, int tall, int hardwareFilter)
{
	DrawSetTextureRGBA(id, rgba, wide, tall, hardwareFilter, true);
}

void BaseUISurface::DrawSetSubTextureRGBA(int textureID, int drawX, int drawY, const byte* rgba, int subTextureWide, int subTextureTall)
{
	//DrawSetSubTextureRGBA(textureID, drawX, drawY, rgba, subTextureWide, subTextureTall);

	/*
	qglTexSubImage2D(
		GL_TEXTURE_2D,
		0,
		drawX, drawY,
		subTextureWide, subTextureTall,
		GL_RGBA, GL_UNSIGNED_BYTE,
		rgba
	);
	*/
	if (g_api && g_api->UploadTextureBlock)
	{
		DrawFlushText();
#ifdef USE_IMGUI_SURFACE
		ImGui_Surface_Flush();
#endif

		g_api->BindTexture(textureID);
		m_iCurrentTexture = textureID;

		std::fprintf(stderr, "[VGUI2-TRACE] BaseUISurface::DrawSetSubTextureRGBA upload texture=%d dst=%d,%d size=%dx%d rgba=%p\n",
			textureID, drawX, drawY, subTextureWide, subTextureTall, (const void *)rgba);
		g_api->UploadTextureBlock(textureID, drawX, drawY, rgba, subTextureWide, subTextureTall);
	}
}

bool BaseUISurface::DrawGetUnicodeCharRenderInfo(uchar32 ch, CharRenderInfo &info)
{
    info.valid = false;

    if (!m_hCurrentFont)
    {
        return info.valid;
    }

    info.valid = true;
    info.ch = ch;

    info.currentFont = m_hCurrentFont;
    info.fontTall = GetFontTall(m_hCurrentFont);

    DrawGetTextPos(info.x, info.y);
    GetCharABCwide(m_hCurrentFont, ch, info.abcA, info.abcB, info.abcC);

    bool bUnderlined = FontManager().GetFontUnderlined(m_hCurrentFont);

    if (!bUnderlined)
    {
        info.x += info.abcA;
    }

    info.textureId = 0;
    float texCoords[4];

    if (!m_FontTextureCache.GetTextureForChar(m_hCurrentFont, ch, &info.textureId, texCoords))
    {
        info.valid = false;
        return info.valid;
    }

    int fontWide = info.abcB;

    if (bUnderlined)
    {
        fontWide += (info.abcA + info.abcC);
        info.x-= info.abcA;
    }

    // Because CharRenderInfo has a pointer to the verts, we need to keep m_BatchedCharVerts in sync, so if we
    //  will be flushing the text when we get to this char, flush it now instead.
    if ( info.textureId != m_iCurrentTexture )
    {
        DrawFlushText();
    }
#ifndef DISABLE_MOE_VGUI2_EXT
    if(IsEmojiChar(ch))
    {
        InitVertex(info.verts[0], info.x, info.y + info.fontTall / 2 - fontWide / 2, texCoords[0], texCoords[1]);
        InitVertex(info.verts[1], info.x + fontWide, info.y + info.fontTall / 2 + fontWide / 2, texCoords[2], texCoords[3]);
    }
    else
#endif
    {
        InitVertex(info.verts[0], info.x, info.y, texCoords[0], texCoords[1]);
        InitVertex(info.verts[1], info.x + fontWide, info.y + info.fontTall, texCoords[2], texCoords[3]);
    }

    info.shouldclip = true;
    return info.valid;
}

void BaseUISurface::DrawRenderCharInternal(const CharRenderInfo &info)
{
    if (_drawTextColor[3] == 0)
        return;

    vpoint_t clippedRect[2];

    if (info.shouldclip)
    {
        if (!ClipRect(info.verts[0], info.verts[1], &clippedRect[0], &clippedRect[1]))
            return;
    }
    else
    {
        clippedRect[0] = info.verts[0];
        clippedRect[1] = info.verts[1];
    }

    if (m_BatchedCharInfos.size() > MAX_BATCHED_CHAR_VERTS)
        DrawFlushText();
#ifndef DISABLE_MOE_VGUI2_EXT
    if(IsEmojiChar(info.ch))
    {
        DrawSetTexture(info.textureId);
        DrawQuadBlend(clippedRect[0], clippedRect[1], white_color);
    }
    else
#endif
#ifdef USE_IMGUI_SURFACE
    if(true)
#else
    if (info.additive)
#endif
    {
        DrawSetTexture(info.textureId);
        DrawQuadBlend(clippedRect[0], clippedRect[1], _drawTextColor);
    }
    else
    {
        CBatchedCharInfo batchedCharInfo;
        batchedCharInfo.verts[0] = clippedRect[0];
        batchedCharInfo.verts[1] = clippedRect[1];
        batchedCharInfo.textureId = info.textureId;
        m_BatchedCharInfos.emplace_back(batchedCharInfo);
    }
}

void BaseUISurface::DrawRenderCharFromInfo(const CharRenderInfo &info)
{
    if (!info.valid)
        return;

    DrawRenderCharInternal(info);
    DrawSetTextPos(info.x + (info.abcB + info.abcC), info.y);
}

void BaseUISurface::DrawFlushText(void)
{
    if (m_BatchedCharInfos.empty())
        return;

    for (int i = 0; i < m_BatchedCharInfos.size(); i++)
    {
        //DrawSetTexture(m_BatchedCharInfos[i].textureId);
        m_iCurrentTexture = m_BatchedCharInfos[i].textureId;
#ifdef USE_IMGUI_SURFACE
        // nothing
#else
        g_api->BindTexture(m_BatchedCharInfos[i].textureId);
#endif
        DrawQuad(m_BatchedCharInfos[i].verts[0], m_BatchedCharInfos[i].verts[1], _drawTextColor);
    }

    m_BatchedCharInfos.clear();
}

void BaseUISurface::DrawQuad(vpoint_t ul, vpoint_t lr, int *pColor)
{
#ifdef USE_IMGUI_SURFACE
    int id = m_iCurrentTexture;
    if( id > 0 && id < VGUI_MAX_TEXTURES && g_textures[id] )
    {
        ImGui_Surface_SetRenderMode(kRenderTransTexture);
        SPR_AdjustSize(&ul.point[0], &ul.point[1], &lr.point[0], &lr.point[1]);
        ImGui_Surface_DrawImage(g_textures[id], ul.point[0], ul.point[1], lr.point[0], lr.point[1], ul.coord[0], ul.coord[1], lr.coord[0], lr.coord[1], pColor[0], pColor[1], pColor[2], pColor[3]);
    }
    else
    {
        vgui2::gEngfuncs.Con_DPrintf("BaseUISurface::DrawQuad invalid texture %d", id);
    }
#else
    int pColor2[4] = { pColor[0], pColor[1], pColor[2], 255 - pColor[3] };
    g_api->SetupDrawingImage(pColor2);
    g_api->DrawQuad(&ul, &lr);
#endif
}

void BaseUISurface::DrawQuadBlend(vpoint_t ul, vpoint_t lr, int *pColor)
{
#ifdef USE_IMGUI_SURFACE
    int id = m_iCurrentTexture;
    if( id > 0 && id < VGUI_MAX_TEXTURES && g_textures[id] )
    {
        ImGui_Surface_SetRenderMode(kRenderTransAlpha);
        SPR_AdjustSize(&ul.point[0], &ul.point[1], &lr.point[0], &lr.point[1]);
        ImGui_Surface_DrawImage(g_textures[id], ul.point[0], ul.point[1], lr.point[0], lr.point[1], ul.coord[0], ul.coord[1], lr.coord[0], lr.coord[1], pColor[0], pColor[1], pColor[2], pColor[3]);
    }
    else
    {
        vgui2::gEngfuncs.Con_DPrintf("BaseUISurface::DrawQuadBlend invalid texture %d", id);
    }
#else
    int pColor2[4] = { pColor[0], pColor[1], pColor[2], 255 - pColor[3] };
    g_api->SetupDrawingImage(pColor2);
    g_api->DrawQuad(&ul, &lr);
#endif
}

void BaseUISurface::DrawQuadArray(int quadCount, vpoint_t *pVerts, int *pColor)
{
#ifdef USE_IMGUI_SURFACE
    int id = m_iCurrentTexture;
    if( id > 0 && id < VGUI_MAX_TEXTURES && g_textures[id] )
    {
        ImGui_Surface_SetRenderMode(kRenderTransTexture);

        for (int i = 0; i < quadCount; ++i)
        {
            vpoint_t ulc, lrc;
            vpoint_t &ul = pVerts[2 * i];
            vpoint_t &lr = pVerts[2 * i + 1];

            if (!ClipRect(ul, lr, &ulc, &lrc))
                continue;

            SPR_AdjustSize(&ulc.point[0], &ulc.point[1], &lrc.point[0], &lrc.point[1]);
            ImGui_Surface_DrawImage(g_textures[id], ulc.point[0], ulc.point[1], lrc.point[0], lrc.point[1], ulc.coord[0], ulc.coord[1], lrc.coord[0], lrc.coord[1], pColor[0], pColor[1], pColor[2], pColor[3]);
        }
        ImGui_Surface_Flush();
    }
    else
    {
        vgui2::gEngfuncs.Con_DPrintf("BaseUISurface::DrawQuadArray invalid texture %d", id);
    }
#else
    int pColor2[4] = { pColor[0], pColor[1], pColor[2], 255 - pColor[3] };
    g_api->SetupDrawingImage(pColor2);
    for (int i = 0; i < quadCount; ++i)
    {
        vpoint_t ulc, lrc;
        vpoint_t &ul = pVerts[2 * i];
        vpoint_t &lr = pVerts[2 * i + 1];

        if (!ClipRect(ul, lr, &ulc, &lrc))
            continue;

        g_api->DrawQuad(&ulc, &lrc);
    }
#endif
}
#ifndef DISABLE_MOE_VGUI2_EXT
bool BaseUISurface::IsEmojiChar(uchar32 ch)
{
    return FontManager().IsEmojiChar(ch);
}
#endif

#if defined(LINUX) || defined(OSX) || defined(WIN32)

const void *BaseUISurface::FontDataHelper( const char *pchFontName, int &size, const char *fontFileName, int weight, int flags )
{
    size = 0;
    std::fprintf(stderr, "[VGUI2-FONT] FontDataHelper request fontName='%s' fontFile='%s' weight=%d flags=%d\n",
        pchFontName ? pchFontName : "<null>",
        fontFileName ? fontFileName : "<null>",
        weight,
        flags);

    auto findCachedFont = [&]( const std::string &cacheKey ) -> const void *
    {
        if ( cacheKey.empty() )
            return nullptr;

        int iIndex = m_FontData.Find( cacheKey.c_str() );
        if ( iIndex == m_FontData.InvalidIndex() )
        {
            return nullptr;
        }

        size = m_FontData[ iIndex ].size;
        return m_FontData[ iIndex ].data;
    };

    auto insertCacheAlias = [&]( const std::string &cacheKey, const font_entry &entry )
    {
        if ( cacheKey.empty() || m_FontData.Find( cacheKey.c_str() ) != m_FontData.InvalidIndex() )
            return;

        m_FontData.Insert( cacheKey.c_str(), entry );
    };

    auto loadFontFile = [&]( const char *path, const std::string &primaryCacheKey, const char *requestedFaceName ) -> const void *
    {
        if ( const void *cached = findCachedFont( primaryCacheKey ) )
            return cached;

        FileHandle_t file = vgui2::filesystem()->Open( path, "rb" );
        if ( !file )
        {
            std::fprintf( stderr, "[VGUI2-FONT] failed to open font file: %s\n", path ? path : "<null>" );
            return nullptr;
        }

        const unsigned fileSize = vgui2::filesystem()->Size( file );
        if ( !fileSize )
        {
            std::fprintf( stderr, "[VGUI2-FONT] empty font file: %s\n", path ? path : "<null>" );
            vgui2::filesystem()->Close( file );
            return nullptr;
        }

        void *buffer = malloc( fileSize );
        if ( !buffer )
        {
            vgui2::filesystem()->Close( file );
            return nullptr;
        }

        const int bytesRead = vgui2::filesystem()->Read( buffer, fileSize, file );
        vgui2::filesystem()->Close( file );
        if ( bytesRead != static_cast<int>( fileSize ) )
        {
            std::fprintf( stderr, "[VGUI2-FONT] short read font file: %s read=%d size=%u\n",
                path ? path : "<null>",
                bytesRead,
                fileSize );
            free( buffer );
            return nullptr;
        }

        FT_Face face;
        const FT_Error error = FT_New_Memory_Face( FontManager().GetFontLibraryHandle(), (const FT_Byte *)buffer, fileSize, 0, &face );
        if ( error )
        {
            std::fprintf( stderr, "[VGUI2-FONT] freetype failed error=%d fontFile=%s face=%s\n",
                static_cast<int>( error ),
                path ? path : "<null>",
                requestedFaceName ? requestedFaceName : "<null>" );
            free( buffer );
            return nullptr;
        }

        font_entry entry;
        entry.size = fileSize;
        entry.data = buffer;

        insertCacheAlias( primaryCacheKey, entry );
        insertCacheAlias( std::string( "path:" ) + ToLowerAscii( path ), entry );
        insertCacheAlias( std::string( "file:" ) + FontFileNameKey( path ), entry );
        if ( requestedFaceName && requestedFaceName[0] )
            insertCacheAlias( FontFaceCacheKey( requestedFaceName, weight, flags ), entry );

        std::fprintf( stderr, "[VGUI2-FONT] cached font path=%s family=%s size=%d\n",
            path ? path : "<null>",
            face->family_name ? face->family_name : "<null>",
            entry.size );
        FT_Done_Face( face );

        size = entry.size;
        return entry.data;
    };

    auto loadEmbeddedFontFile = [&]( const char *filename, const std::string &primaryCacheKey, const char *requestedFaceName ) -> const void *
    {
        if ( const void *cached = findCachedFont( primaryCacheKey ) )
            return cached;

        const EmbeddedVGUIFontFile *embeddedFont = FindEmbeddedFont( filename );
        if ( !embeddedFont )
            return nullptr;

        FT_Face face;
        const FT_Error error = FT_New_Memory_Face(
            FontManager().GetFontLibraryHandle(),
            (const FT_Byte *)embeddedFont->data,
            embeddedFont->size,
            0,
            &face );
        if ( error )
        {
            std::fprintf( stderr, "[VGUI2-FONT] freetype failed error=%d embeddedFont=%s face=%s\n",
                static_cast<int>( error ),
                embeddedFont->filename ? embeddedFont->filename : "<null>",
                requestedFaceName ? requestedFaceName : "<null>" );
            return nullptr;
        }

        font_entry entry;
        entry.size = static_cast<int>( embeddedFont->size );
        entry.data = embeddedFont->data;

        insertCacheAlias( primaryCacheKey, entry );
        insertCacheAlias( std::string( "embedded:" ) + FontFileNameKey( embeddedFont->filename ), entry );
        insertCacheAlias( std::string( "file:" ) + FontFileNameKey( embeddedFont->filename ), entry );
        if ( requestedFaceName && requestedFaceName[0] )
            insertCacheAlias( FontFaceCacheKey( requestedFaceName, weight, flags ), entry );

        std::fprintf( stderr, "[VGUI2-FONT] loaded embedded bundled font: %s family=%s size=%d\n",
            embeddedFont->filename ? embeddedFont->filename : "<null>",
            face->family_name ? face->family_name : "<null>",
            entry.size );
        FT_Done_Face( face );

        size = entry.size;
        return entry.data;
    };

    if( fontFileName && fontFileName[0] )
    {
        const std::string pathCacheKey = std::string( "path:" ) + ToLowerAscii( fontFileName );
        if ( const void *loaded = loadFontFile( fontFileName, pathCacheKey, pchFontName ) )
            return loaded;

        return loadEmbeddedFontFile( fontFileName, pathCacheKey, pchFontName );
    }

    const std::string faceCacheKey = FontFaceCacheKey( pchFontName, weight, flags );
    if ( const void *cached = findCachedFont( faceCacheKey ) )
        return cached;

    std::string resolvedPath;
    if ( ResolveFontFaceToFile( pchFontName, weight, flags, resolvedPath ) )
    {
        if ( const void *loaded = loadFontFile( resolvedPath.c_str(), faceCacheKey, pchFontName ) )
            return loaded;

        if ( const void *embedded = loadEmbeddedFontFile( resolvedPath.c_str(), faceCacheKey, pchFontName ) )
            return embedded;
    }

    const bool symbol = ( flags & vgui2::ISurface::FONTFLAG_SYMBOL ) != 0;
    const std::string normalizedFace = NormalizeFaceName( pchFontName );
    if ( const char *knownFile = StyledKnownFontFileForFace( normalizedFace, weight, flags, symbol ) )
    {
        if ( const void *embedded = loadEmbeddedFontFile( knownFile, faceCacheKey, pchFontName ) )
            return embedded;
    }

    const std::string faceFile = normalizedFace.empty() ? std::string() : normalizedFace + ".ttf";
    if ( const void *embedded = loadEmbeddedFontFile( faceFile.c_str(), faceCacheKey, pchFontName ) )
        return embedded;

    const char *fallbackFile = symbol ? "marlett.ttf" : StyledKnownFontFileForFace( "tahoma", weight, flags, false );
    if ( const void *embedded = loadEmbeddedFontFile( fallbackFile, faceCacheKey, pchFontName ) )
    {
        std::fprintf( stderr, "[VGUI2-FONT] fallback to embedded bundled font: %s\n", fallbackFile );
        return embedded;
    }

    return NULL;
}

// TODO : m_FontData never frees, possibly leaked.

#endif // LINUX
