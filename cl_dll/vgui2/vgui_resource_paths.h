#ifndef VGUI_RESOURCE_PATHS_H
#define VGUI_RESOURCE_PATHS_H

#ifdef _WIN32
#pragma once
#endif

namespace vgui2::resource_paths
{
inline constexpr const char kMenuTeam[] = "resource/UI/Teammenu.res";
inline constexpr const char kMenuClassCT[] = "Resource/UI/ClassMenu_CT.res";
inline constexpr const char kMenuClassTER[] = "Resource/UI/ClassMenu_TER.res";
inline constexpr const char kMenuBuy[] = "resource/UI/BuyMenu.res";
inline constexpr const char kMenuBuyCT[] = "resource/UI/BuyMenu_CT.res";
inline constexpr const char kMenuBuyTER[] = "resource/UI/BuyMenu_TER.res";
inline constexpr const char kMenuBuyMain[] = "resource/UI/MainBuyMenu.res";
inline constexpr const char kMenuBuyPistols[] = "resource/UI/BuyPistols.res";
inline constexpr const char kMenuBuyEquipment[] = "resource/UI/BuyEquipment.res";
inline constexpr const char kMenuBuyEquipmentCT[] = "resource/UI/BuyEquipment_CT.res";
inline constexpr const char kMenuBuyEquipmentTER[] = "resource/UI/BuyEquipment_TER.res";
inline constexpr const char kMenuBuyShotguns[] = "resource/UI/BuyShotguns.res";
inline constexpr const char kMenuBuyPistolsCT[] = "resource/UI/BuyPistols_CT.res";
inline constexpr const char kMenuBuyPistolsTER[] = "resource/UI/BuyPistols_TER.res";
inline constexpr const char kMenuBuyShotgunsCT[] = "resource/UI/BuyShotguns_CT.res";
inline constexpr const char kMenuBuyShotgunsTER[] = "resource/UI/BuyShotguns_TER.res";
inline constexpr const char kMenuBuyRifles[] = "resource/UI/BuyRifles.res";
inline constexpr const char kMenuBuyRiflesCT[] = "resource/UI/BuyRifles_CT.res";
inline constexpr const char kMenuBuyRiflesTER[] = "resource/UI/BuyRifles_TER.res";
inline constexpr const char kMenuBuySubMachineguns[] = "resource/UI/BuySubMachineguns.res";
inline constexpr const char kMenuBuySubMachinegunsCT[] = "resource/UI/BuySubMachineguns_CT.res";
inline constexpr const char kMenuBuySubMachinegunsTER[] = "resource/UI/BuySubMachineguns_TER.res";
inline constexpr const char kMenuBuyMachineguns[] = "resource/UI/BuyMachineguns.res";
inline constexpr const char kMenuBuyMachinegunsCT[] = "resource/UI/BuyMachineguns_CT.res";
inline constexpr const char kMenuBuyMachinegunsTER[] = "resource/UI/BuyMachineguns_TER.res";
inline constexpr const char kMenuMOTD[] = "resource/UI/MOTD.res";
inline constexpr const char kMenuBackgroundPanel[] = "resource/UI/BackgroundPanel.res";

inline constexpr const char *ClassMenuForTeam(bool isTerrorist)
{
	return isTerrorist ? kMenuClassTER : kMenuClassCT;
}

inline constexpr const char *BuyCategoryForTeam(const char *ct, const char *terrorist, bool isTerrorist)
{
	return isTerrorist ? terrorist : ct;
}
}

#endif
