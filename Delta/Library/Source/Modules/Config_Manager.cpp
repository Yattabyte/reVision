
#include "Modules\Config_Manager.h"

static Shared_Asset_Config configurations;

namespace CFG
{
	void shutdown()
	{
		saveConfiguration();
	}
	void loadConfiguration()
	{
		if (configurations.get() == nullptr)
			Asset_Manager::load_asset(configurations, "config.cfg", false);
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
}