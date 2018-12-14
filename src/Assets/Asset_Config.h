#pragma once
#ifndef	ASSET_CONFIG_H
#define	ASSET_CONFIG_H

#include "Assets\Asset.h"
#include <map>
#include <vector>

constexpr float UNDEFINED_CVAL = -12345.67890f; // Undefined preference

class Engine;
class Asset_Config;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Config : public std::shared_ptr<Asset_Config> {
public:
	Shared_Config() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	cfg_strings		the configuration strings to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Config(Engine * engine, const std::string & filename, const std::vector<std::string> & cfg_strings, const bool & threaded = true);
};

/** A registry for configuration name-value pairs. */
class Asset_Config : public Asset
{
public: 
	/** Destroy the Config. */
	~Asset_Config() = default;
	/** Construct the config with a particular set of variable names. */
	Asset_Config(const std::string & filename, const std::vector<std::string> & strings);


	// Public Methods
	/** Assigns the specified value to the specified key.
	@param	cfg_key		the key to apply this new value to
	@param	cfg_value	the new value to give to this key */
	void setValue(const unsigned int & cfg_key, const float & cfg_value);
	/** Retrieves the value assigned to the supplied key.
	@param	cfg_key		the key in which to fetch the value from
	@return				the value assigned to supplied key (UNDEFINED_CVAL if the supplied key doesn't exist) */
	const float getValue(const unsigned int & cfg_key) const;
	/** Writes the configuration file back to disk within the \\Config\\ folder. */
	void saveConfig();


	// Public Attributes
	std::map<unsigned int, float> m_configuration;
	std::vector<std::string> m_strings;


private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
}; 

#endif //ASSET_CONFIG_H
