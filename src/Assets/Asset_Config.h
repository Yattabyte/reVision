#pragma once
#ifndef	ASSET_CONFIG
#define	ASSET_CONFIG
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define EXT_CONFIG ".cfg"
#define DIRECTORY_CONFIG File_Reader::GetCurrentDir() + "\\Configs\\"
#define ABS_DIRECTORY_CONFIG(filename) DIRECTORY_CONFIG + filename + EXT_CONFIG
#define UNDEFINED_CVAL -12345.67890f // Undefined preference

#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include <map>
#include <vector>

class Asset_Config;
typedef shared_ptr<Asset_Config> Shared_Asset_Config;


/**
 * A registry for configuration name-value pairs.
 **/
class DT_ENGINE_API Asset_Config : public Asset
{
public: 
	// (de)Constructors
	/** Destroy the Config. */
	~Asset_Config();
	/** Construct the config with a particular set of variable names. */
	Asset_Config(const string & filename, const vector<string> & strings);

	
	// Public Methods
	/** Assigns the specified value to the specified key.
	 * @param	cfg_key		the key to apply this new value to
	 * @param	cfg_value	the new value to give to this key */
	void setValue(const unsigned int & cfg_key, const float & cfg_value);
	/** Retrieves the value assigned to the supplied key.
	 * @param	cfg_key		the key in which to fetch the value from
	 * @return				the value assigned to supplied key (UNDEFINED_CVAL if the supplied key doesn't exist) */
	float getValue(const unsigned int & cfg_key);
	/** Writes the configuration file back to disk within the \\Config\\ folder. */
	void saveConfig();


	// Public Attributes
	map<unsigned int, float> configuration;
	vector<string> m_strings;
}; 

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists. */
	DT_ENGINE_API void load_asset(Shared_Asset_Config & user, const string & filename, const vector<string> & cfg_strings, const bool & threaded = true);
};

/**
 * Implements a work order for Configuration Assets.
 **/
class Config_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Config work order. */
	Config_WorkOrder(Shared_Asset_Config & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Config_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Attributes
	string m_filename;
	Shared_Asset_Config m_asset;
};
#endif //ASSET_CONFIG
