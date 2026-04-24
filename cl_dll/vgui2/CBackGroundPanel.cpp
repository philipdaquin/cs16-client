
#include "CBackGroundPanel.h"
#include <vgui/ISurface.h>
#include "hud.h"

CBackGroundPanel::CBackGroundPanel( vgui2::Panel* pParent )
	: BaseClass( pParent, "ViewPortBackGround" )
{
	gEngfuncs.Con_Printf("[phase1][VGUI2-CLIENT] CBackGroundPanel ctor this=%p parent=%p vparent=%p\n",
		this, (void *)pParent, (void *)GetVParent());
	SetScheme( "ClientScheme" );

	SetTitleBarVisible( false );
	SetMoveable( false );
	SetSizeable( false );
	SetProportional( true );
}

void CBackGroundPanel::ApplySchemeSettings( vgui2::IScheme* pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	const auto color = pScheme->GetColor( "ViewportBG", Color( 0, 0, 0, 0 ) );

	SetBgColor( color );
}

void CBackGroundPanel::PerformLayout()
{
	//Resize ourselves to the screen's size to fill the entire viewport.
	int w, h;

	vgui2::surface()->GetScreenSize( w, h );

	SetBounds( 0, 0, w, h );

	BaseClass::PerformLayout();
}
