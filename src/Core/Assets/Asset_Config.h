/*
	Asset_Config
	
	- Stores and retrieves configuration values / preferences
	- They are mapped between an integer key and a float value
	- For convienience, the keys are defined at the bottom of this document as ENUMS and strings
*/

#pragma once
#ifndef	ASSET_CONFIG
#define	ASSET_CONFIG
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_CONFIG ".cfg"
#define DIRECTORY_CONFIG getCurrentDir() + "\\Configs\\"
#define ABS_DIRECTORY_CONFIG(filename) DIRECTORY_CONFIG + filename + EXT_CONFIG
#define UNDEFINED_CVAL -12345.67890f // Undefined preference

#include "Assets\Asset.h"
#include "Systems\Asset_Manager.h"
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

	DT_ENGINE_API ~Asset_Config();
	DT_ENGINE_API Asset_Config();
	DT_ENGINE_API static int GetAssetType();
	DT_ENGINE_API virtual void Finalize();

	/****************
	----Variables----
	****************/

	string filename;
	map<unsigned int, float> configuration;
	vector<string> m_strings;

	/******************************
	----Configuration Functions----
	******************************/

	// Saves the value of @cfg_value to the spot of @cfg_key in our configuration map
	DT_ENGINE_API void setValue(const unsigned int &cfg_key, const float &cfg_value);
	// Gets the value in our configuration map at the spot of @cfg_key
	// Returns UNDEFINED_CVAL when @cfg_key out of bounds (doesn't exist)
	DT_ENGINE_API float getValue(const unsigned int & cfg_key);
	// Saves our configuration map to disk within the \\Config\\ folder
	DT_ENGINE_API void saveConfig();
}; 
namespace Asset_Manager {
	// Attempts to create an asset from disk or share one if it already exists
	DT_ENGINE_API void load_asset(Shared_Asset_Config &user, const string &filename, const vector<string> &cfg_strings, const bool &threaded = true);
};

#endif //ASSET_CONFIG
