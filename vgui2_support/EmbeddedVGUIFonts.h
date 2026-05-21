#pragma once

#include <stddef.h>

struct EmbeddedVGUIFontFile
{
	const char *filename;
	const unsigned char *data;
	size_t size;
};

const EmbeddedVGUIFontFile *VGUI2_GetEmbeddedFontFiles( size_t *count );
