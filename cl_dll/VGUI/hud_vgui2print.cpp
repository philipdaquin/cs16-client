#include <string.h>
#include <wchar.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>

void VGui2_DrawString(int x, int y, const char *text, int r, int g, int b, int a)
{
	if (!g_pVGuiSurface || !g_pVGuiLocalize || !text)
		return;

	wchar_t unicode[1024];
	g_pVGuiLocalize->ConvertANSIToUnicode(text, unicode, sizeof(unicode));

	g_pVGuiSurface->DrawSetTextColor(r, g, b, a);
	g_pVGuiSurface->DrawSetTextPos(x, y);
	g_pVGuiSurface->DrawPrintText(unicode, wcslen(unicode));
}
