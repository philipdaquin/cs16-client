#include "BaseUISurface.h"
#include "FontTextureCache.h"
#include "vgui_surfacelib/FontManager.h"

#include <vgui/ISurface.h>

extern BaseUISurface* staticSurface;

static constexpr int FONT_GLYPH_PADDING = 1;

CFontTextureCache::CFontTextureCache()
{

}

CFontTextureCache::~CFontTextureCache()
{
}

void CFontTextureCache::InvalidateFont(vgui2::HFont font)
{
	for (auto it = m_CharCache.begin(); it != m_CharCache.end(); )
	{
		if (it->first.first == font)
			it = m_CharCache.erase(it);
		else
			++it;
	}
}

bool CFontTextureCache::AllocatePageForChar(int charWide, int charTall, int& pageIndex, int& drawX, int& drawY, int& twide, int& ttall)
{
	int iHeight;

	if (charTall <= 16)
	{
		iHeight = 16;
	}
	else
	{
		iHeight = 0;

		for (int i = 0; i < 32 && iHeight < charTall; ++i)
		{
			iHeight = 1 << i;
		}
	}

	if (iHeight > 256)
	{
		return false;
	}

    int iWidth;

    if (charWide <= 16)
    {
        iWidth = 16;
    }
    else
    {
        iWidth = 0;

        for (int i = 0; i < 32 && iWidth < charWide; ++i)
        {
            iWidth = 1 << i;
        }
    }

	if (iWidth > 256)
	{
		return false;
	}

	auto CreatePage = [&]()
	{
		m_PageList.push_back({});
		pageIndex = static_cast<int>( m_PageList.size() ) - 1;

		page_t *pNewPage = &m_PageList[pageIndex];
		pNewPage->textureID = staticSurface->CreateNewTextureID(false);
		pNewPage->fontHeight = iHeight;
		pNewPage->wide = 512;
		pNewPage->tall = 512;
		pNewPage->nextX = 0;
		pNewPage->nextY = 0;

		static byte rgba[512 * 512 * 4];
		memset(rgba, 0, sizeof(rgba));

		staticSurface->DrawSetTextureRGBAWithAlphaChannel(
			pNewPage->textureID,
			rgba,
			pNewPage->wide, pNewPage->tall,
			false
		);

		return pNewPage;
	};

	pageIndex = static_cast<int>( m_PageList.size() ) - 1;
	page_t *pPage = ( pageIndex >= 0 ) ? &m_PageList[pageIndex] : nullptr;

	if( !pPage || pPage->fontHeight != iHeight )
	{
		pPage = CreatePage();
	}

	if( pPage->nextX + iWidth > pPage->wide )
	{
		pPage->nextX = 0;
		pPage->nextY += pPage->fontHeight;
	}

	if( pPage->nextY + iHeight > pPage->tall )
	{
		pPage = CreatePage();
	}

	drawX = pPage->nextX;
	drawY = pPage->nextY;
	twide = pPage->wide;
	ttall = pPage->tall;

	pPage->nextX += iWidth;

	return true;
}

bool CFontTextureCache::GetTextureForChar(vgui2::HFont font, uchar32 wch, int* textureID, float* texCoords)
{
	auto index = m_CharCache.find(std::make_pair(font, wch));

	if (index == m_CharCache.end())
	{
		auto pFont = FontManager().GetFontForChar(font, wch);

		if (!pFont)
			return false;

		const auto fontTall = pFont->GetHeight();

		int a, b, c;

		pFont->GetCharABCWidths(wch, a, b, c);

		int fontWide = b;

		if (pFont->GetUnderlined())
		{
			fontWide = a + b + c;
		}

		const int paddedWide = fontWide + FONT_GLYPH_PADDING * 2;
		const int paddedTall = fontTall + FONT_GLYPH_PADDING * 2;

		int page, drawX, drawY, twide, ttall;

		if (!AllocatePageForChar(paddedWide, paddedTall, page, drawX, drawY, twide, ttall))
			return false;

		const auto size = 4 * fontWide * fontTall;
		const auto paddedSize = 4 * paddedWide * paddedTall;

		auto pDest = reinterpret_cast<byte*>(MemAlloc_AllocAligned(size, 16));
		auto pPaddedDest = reinterpret_cast<byte*>(MemAlloc_AllocAligned(paddedSize, 16));

		if (size >= 4)
			memset(pDest, 0, size);
		if (paddedSize >= 4)
			memset(pPaddedDest, 0, paddedSize);

		pFont->GetCharRGBA(wch, fontWide, fontTall, pDest);

		for (int y = 0; y < fontTall; ++y)
		{
			memcpy(
				pPaddedDest + 4 * ((y + FONT_GLYPH_PADDING) * paddedWide + FONT_GLYPH_PADDING),
				pDest + 4 * y * fontWide,
				4 * fontWide
			);
		}

		auto& pageData = m_PageList[page];

		staticSurface->DrawSetSubTextureRGBA(
			pageData.textureID,
			drawX, drawY,
			pPaddedDest,
			paddedWide, paddedTall
		);

        MemAlloc_FreeAligned(pDest);
        MemAlloc_FreeAligned(pPaddedDest);

        cacheentry_t cacheitem;

        memset(&cacheitem, 0, sizeof(cacheitem));

        cacheitem.font = font;
        cacheitem.wch = wch;
		cacheitem.page = page;

		cacheitem.texCoords[0] = static_cast<double>(drawX + FONT_GLYPH_PADDING) / twide;
		cacheitem.texCoords[1] = static_cast<double>(drawY + FONT_GLYPH_PADDING) / ttall;
		cacheitem.texCoords[2] = static_cast<double>(drawX + FONT_GLYPH_PADDING + fontWide) / twide;
		cacheitem.texCoords[3] = static_cast<double>(drawY + FONT_GLYPH_PADDING + fontTall) / ttall;

		index = m_CharCache.emplace(std::make_pair(font, wch), cacheitem).first;
	}

	const auto& cacheData = index->second;
	const auto& pageData = m_PageList[cacheData.page];

	*textureID = pageData.textureID;

	texCoords[0] = cacheData.texCoords[0];
	texCoords[1] = cacheData.texCoords[1];
	texCoords[2] = cacheData.texCoords[2];
	texCoords[3] = cacheData.texCoords[3];

	return true;
}
