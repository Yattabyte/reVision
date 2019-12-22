#pragma once
#ifndef	CONFIG_H
#define	CONFIG_H

#include "Assets/Asset.h"
#include <map>


// Forward Declarations
class Config;

/** Shared version of a Config asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Config final : public std::shared_ptr<Config> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Config() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	cfg_strings		the configuration strings to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	Shared_Config(Engine& engine, const std::string& filename, const std::vector<std::string>& cfg_strings, const bool& threaded = true);
};

/** A map for configuration name-value pairs.
Represents a series of values corresponding to strings, like user preferences or binds. */
class Config final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Config. */
	inline ~Config() = default;
	/** Construct the config with a particular set of variable names.
	@param	engine		reference to the engine to use. 
	@param	filename	the asset file name (relative to engine directory).
	@param	strings		the configuration strings to use. */
	Config(Engine& engine, const std::string& filename, const std::vector<std::string>& strings);


	// Public Methods
	/** Assigns the specified value to the specified key.
	@param	cfg_key		the key to apply this new value to.
	@param	cfg_value	the new value to give to this key. */
	void setValue(const unsigned int& cfg_key, const float& cfg_value);
	/** Retrieves the value assigned to the supplied key.
	@param	cfg_key		the key in which to fetch the value from.
	@return				the value assigned to supplied key (NaN if the supplied key doesn't exist). */
	float getValue(const unsigned int& cfg_key) const;
	/** Writes the configuration file back to disk within the \\Config\\ folder. */
	void saveConfig() const;


	// Public Attributes
	std::map<unsigned int, float> m_configuration;
	std::vector<std::string> m_strings;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Config(Config&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Config(const Config&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Config& operator =(Config&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Config& operator =(const Config&) noexcept = delete;


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Config;
};

#endif //CONFIG_H