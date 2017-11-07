#include "Managers\Config_Manager.h"
#include <vector>
#include <map>

static Shared_Asset_Config configurations;
static map<int, vector<void(*)()>> function_map;

namespace CFG
{
	void shutdown()
	{
		saveConfiguration();
	}

	void loadConfiguration()
	{
		if (configurations.get() == nullptr)
			Asset_Manager::load_asset(configurations, "config", false);
	}

	void saveConfiguration()
	{
		if (configurations.get() != nullptr)
			configurations->saveConfig();
	}

	float getPreference(const int & cfg_key)
	{
		if (configurations.get() != nullptr)
			return configurations->getValue(cfg_key);
		return UNDEFINED_CVAL;
	}

	void setPreference(const int & cfg_key, const float & cfg_value)
	{
		if (configurations.get() != nullptr)
			configurations->setValue(cfg_key, cfg_value);
		if (function_map.size() > cfg_key)
			for each (auto &function in function_map[cfg_key])
				function();
	}

	void addPreferenceCallback(const int & cfg_key, void(*function)())
	{
		// Inserts a function vector at the key in the map. Map keys are unique so this will only happen once
		function_map.insert(std::pair<int, vector<void(*)()>>(cfg_key, vector<void(*)()>()));
		// Only add the function to the vector once
		bool found = false;
		for each (auto *current_function in function_map[cfg_key])
			if (function == current_function)
				return;
		function_map[cfg_key].push_back(function);
	}
}