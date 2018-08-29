#pragma once
#ifndef	ASSET_CONFIG_H
#define	ASSET_CONFIG_H
#define UNDEFINED_CVAL -12345.67890f // Undefined preference

#include "Assets\Asset.h"
#include <map>
#include <vector>


class Engine;
class Asset_Config;
using Shared_Asset_Config = std::shared_ptr<Asset_Config>;

/** A registry for configuration name-value pairs. */
class Asset_Config : public Asset
{
public: 
	/** Destroy the Config. */
	~Asset_Config() = default;


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	cfg_strings		the configuration strings to use
	@param	threaded		create in a separate thread
	@return					the desired asset */	
	static Shared_Asset_Config Create(Engine * engine, const std::string & filename, const std::vector<std::string> & cfg_strings, const bool & threaded = true);
	/** Assigns the specified value to the specified key.
	@param	cfg_key		the key to apply this new value to
	@param	cfg_value	the new value to give to this key */
	void setValue(const unsigned int & cfg_key, const float & cfg_value);
	/** Retrieves the value assigned to the supplied key.
	@param	cfg_key		the key in which to fetch the value from
	@return				the value assigned to supplied key (UNDEFINED_CVAL if the supplied key doesn't exist) */
	float getValue(const unsigned int & cfg_key);
	/** Writes the configuration file back to disk within the \\Config\\ folder. */
	void saveConfig();


	// Public Attributes
	std::map<unsigned int, float> m_configuration;
	std::vector<std::string> m_strings;


private:
	// Private Constructors
	/** Construct the config with a particular set of variable names. */
	Asset_Config(const std::string & filename, const std::vector<std::string> & strings);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine) override;
	virtual void initialize(Engine * engine, const std::string & fullDirectory) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
}; 

#endif //ASSET_CONFIG_H
