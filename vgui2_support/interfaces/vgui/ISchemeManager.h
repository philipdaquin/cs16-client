#ifndef ISCHEMEMANAGER_H
#define ISCHEMEMANAGER_H

#include "tier1/interface.h"
#include "vgui/VGUI.h"

namespace vgui2 {
	class IScheme;
	class IBorder;
	class IImage;

	class ISchemeManager : public IBaseInterface
	{
	public:
		virtual HScheme LoadSchemeFromFile(const char* fileName, const char* tag) = 0;
		virtual void ReloadSchemes(void) = 0;
		virtual HScheme GetDefaultScheme(void) = 0;
		virtual HScheme GetScheme(const char* tag) = 0;
		virtual IImage* GetImage(const char* imageName, bool hardwareFiltered) = 0;
		virtual HTexture GetImageID(const char* imageName, bool hardwareFiltered) = 0;
		virtual IScheme* GetIScheme(HScheme scheme) = 0;
		virtual void Shutdown(bool full = true) = 0;
		virtual int GetProportionalScaledValue(int normalizedValue) = 0;
		virtual int GetProportionalNormalizedValue(int scaledValue) = 0;

	public:
		int GetProportionalScaledValueEx(HScheme scheme, int normalizedValue);
		int GetProportionalNormalizedValueEx(HScheme scheme, int scaledValue);
	};

	class ISchemeManagerEx : public ISchemeManager
	{
	public:
		[[deprecated]] virtual HScheme LoadSchemeFromFileEx(VPANEL sizingPanel, const char *fileName, const char *tag) = 0;
		[[deprecated]] virtual int GetProportionalScaledValueEx(HScheme scheme, int normalizedValue) = 0;
		[[deprecated]] virtual int GetProportionalNormalizedValueEx(HScheme scheme, int scaledValue) = 0;

		virtual HScheme LoadSchemeFromFilePath(const char *fileName, const char *pathID, const char *tag) = 0;
	};
}

#define VGUI_SCHEME_INTERFACE_VERSION "VGUI_Scheme009"

#endif // ISCHEMEMANAGER_H
