//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: lightweight swap helper used by tier1 containers
//
//===========================================================================

#ifndef TIER0_VSWAP_H
#define TIER0_VSWAP_H

#ifdef _WIN32
#pragma once
#endif

#include <utility>

template <typename T>
inline void V_swap( T &x, T &y )
{
	using std::swap;
	swap( x, y );
}

#endif // TIER0_VSWAP_H
