/*
	Asset_Config
	
	- Stores and retrieves configuration values / preferences
	- They are mapped between an integer key and a float value
	- For convienience, the keys are defined at the bottom of this document as ENUMS and strings
*/

#pragma once
#ifndef	ASSET_CONFIG
#define	ASSET_CONFIG
#ifdef	ASSET_CONFIG_EXPORT
#define	ASSET_CONFIG_API __declspec(dllexport)
#else
#define	ASSET_CONFIG_API __declspec(dllimport)
#endif

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

	~Asset_Config();
	Asset_Config();
	ASSET_CONFIG_API static int GetAssetType();
	ASSET_CONFIG_API virtual void Finalize();

	/****************
	----Variables----
	****************/

	string filename;
	map<int, float> configuration;

	/******************************
	----Configuration Functions----
	******************************/

	// Saves the value of @cfg_value to the spot of @cfg_key in our configuration map
	ASSET_CONFIG_API void setValue(const int &cfg_key, const float &cfg_value);
	// Gets the value in our configuration map at the spot of @cfg_key
	// Returns UNDEFINED_CVAL when @cfg_key out of bounds (doesn't exist)
	ASSET_CONFIG_API float getValue(const int & cfg_key);
	// Saves our configuration map to disk within the \\Config\\ folder
	ASSET_CONFIG_API void saveConfig();
}; 

namespace Asset_Manager {
	// Attempts to create an asset from disk or share one if it already exists
	ASSET_CONFIG_API void load_asset(Shared_Asset_Config &user, const string &filename, const bool &threaded = true);
}

#define UNDEFINED_CVAL -12345.67890f // Undefined preference

/****************************
----Preferences as ENUMS ----
****************************/
static const enum CFG_ENUM {
	C_WINDOW_WIDTH,
	C_WINDOW_HEIGHT,
	C_TEXTURE_ANISOTROPY
};

/******************************
----Preferences as STRINGS ----
******************************/
static const vector<string> CFG_STRING = {
	"C_WINDOW_WIDTH",
	"C_WINDOW_HEIGHT",
	"C_TEXTURE_ANISOTROPY"
};

#endif //ASSET_CONFIG
