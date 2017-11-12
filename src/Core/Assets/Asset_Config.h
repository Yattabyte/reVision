/*
	Asset_Config
	
	- Stores and retrieves configuration values / preferences
	- They are mapped between an integer key and a float value
	- For convienience, the keys are defined at the bottom of this document as ENUMS and strings
*/

#pragma once
#ifndef	ASSET_CONFIG
#define	ASSET_CONFIG
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#define EXT_CONFIG ".cfg"
#define DIRECTORY_CONFIG getCurrentDir() + "\\Configs\\"
#define ABS_DIRECTORY_CONFIG(filename) DIRECTORY_CONFIG + filename + EXT_CONFIG

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include <map>
#include <vector>

class Asset_Config;
typedef shared_ptr<Asset_Config> Shared_Asset_Config;
class Asset_Config : public Asset
{
public: 
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~Asset_Config();
	DELTA_CORE_API Asset_Config();
	DELTA_CORE_API static int GetAssetType();
	DELTA_CORE_API virtual void Finalize();

	/****************
	----Variables----
	****************/

	string filename;
	map<int, float> configuration;

	/******************************
	----Configuration Functions----
	******************************/

	// Saves the value of @cfg_value to the spot of @cfg_key in our configuration map
	DELTA_CORE_API void setValue(const int &cfg_key, const float &cfg_value);
	// Gets the value in our configuration map at the spot of @cfg_key
	// Returns UNDEFINED_CVAL when @cfg_key out of bounds (doesn't exist)
	DELTA_CORE_API float getValue(const int & cfg_key);
	// Saves our configuration map to disk within the \\Config\\ folder
	DELTA_CORE_API void saveConfig();
}; 
namespace Asset_Manager {
	// Attempts to create an asset from disk or share one if it already exists
	DELTA_CORE_API void load_asset(Shared_Asset_Config &user, const string &filename, const bool &threaded = true);
};

#define UNDEFINED_CVAL -12345.67890f // Undefined preference

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

#endif //ASSET_CONFIG