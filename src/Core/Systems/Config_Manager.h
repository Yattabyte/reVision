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
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

using namespace std;

/****************************
----Preferences as ENUMS ----
****************************/
static const enum CFG_ENUM {
	C_WINDOW_WIDTH,
	C_WINDOW_HEIGHT,
	C_TEXTURE_ANISOTROPY,
	C_SHADOW_SIZE_REGULAR,
	C_SHADOW_SIZE_LARGE,
	C_SHADOW_QUALITY
};

/******************************
----Preferences as STRINGS ----
******************************/
static const vector<string> CFG_STRING = {
	"C_WINDOW_WIDTH",
	"C_WINDOW_HEIGHT",
	"C_TEXTURE_ANISOTROPY",
	"C_SHADOW_SIZE_REGULAR",
	"C_SHADOW_SIZE_LARGE",
	"C_SHADOW_QUALITY"
};

namespace CFG
{
	// Shutsdown by saving the current configuration map to disk
	DT_ENGINE_API void shutdown();
	// Loads a configuration file from disk. Defaults to "config"
	DT_ENGINE_API void loadConfiguration();
	// Saves a configuration file to disk. Defaults to "config"
	DT_ENGINE_API void saveConfiguration();
	// Returns a preference value given a preference key
	DT_ENGINE_API float getPreference(const int &cfg_key);
	// Set a preference at @cfg_key with the value at @value
	DT_ENGINE_API void setPreference(const int &cfg_key, const float &cfg_value);
	// Add a callback function @function to be called whenever the preference @cfg_key changes
	DT_ENGINE_API void addPreferenceCallback(const int &cfg_key, void(*function)(const float&));
	// Remove a callback function @function from being called whenever the preference @cfg_key changes
	DT_ENGINE_API void removePreferenceCallback(const int &cfg_key, void(*function)(const float&));
}

#endif // CONFIG_MANAGER