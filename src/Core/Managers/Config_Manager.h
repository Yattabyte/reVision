/*
	Config Manager

	- Loads an engine-wide configuration that other aspects of the engine may need to use
		- For example: Resolution, quality preferences, and key binds
	- Mostly a wrapper for the config asset, but this keeps a static one for all to use
*/

#include "Assets\Asset_Config.h"

#pragma once
#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER
#ifdef	CORE_EXPORT
#define CONFIG_MANAGER_API __declspec(dllexport)
#else
#define	CONFIG_MANAGER_API __declspec(dllimport)
#endif

using namespace std;

namespace CFG
{
	// Shutsdown by saving the current configuration map to disk
	CONFIG_MANAGER_API void shutdown();
	// Loads a configuration file from disk. Defaults to "config"
	CONFIG_MANAGER_API void loadConfiguration();
	// Saves a configuration file to disk. Defaults to "config"
	CONFIG_MANAGER_API void saveConfiguration();
	// Returns a preference value given a preference key
	CONFIG_MANAGER_API float getPreference(const int &cfg_key);
}

#endif // CONFIG_MANAGER