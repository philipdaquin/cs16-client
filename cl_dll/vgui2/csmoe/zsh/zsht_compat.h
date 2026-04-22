#pragma once

#include <string>
#include <vector>

namespace cl
{
	struct zsht_buildmenu_s
	{
		int id = 0;
		int submenu = 0;
		std::string command;
		std::string Tooltip;
		std::string Name;
		int wood = 0;
		int iron = 0;
		int power = 0;
	};

	inline std::vector<zsht_buildmenu_s> ZombieShelterTeam_BuildMenuCfg()
	{
		// Original data lives in the missing cfg_zsht.h module. Keep the
		// interface intact for now and fall back to an empty config list.
		return {};
	}
}
