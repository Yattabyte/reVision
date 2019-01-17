#pragma once
#ifndef	ASSET_LEVEL_H
#define	ASSET_LEVEL_H

#include "Assets/Asset.h"
#include <any>
#include <optional>
#include <string>


class Engine;
class Asset_Level;
struct LevelStruct_Entity;
struct LevelStruct_Component;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Level : public std::shared_ptr<Asset_Level> {
public:
	Shared_Level() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Level(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** An asset which represents some intermediate form of a world level. */
class Asset_Level : public Asset
{
public:
	/** Destroy the Level. */
	~Asset_Level() = default;
	/** Construct the Level. */
	Asset_Level(const std::string & filename);
	
	
	// Public Attributes
	std::vector<LevelStruct_Entity> m_entities;

private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class Shared_Level;
};

struct LevelStruct_Component {
	std::string type;
	std::vector<std::any> parameters;
};
struct LevelStruct_Entity {
	std::vector<LevelStruct_Component> components;
};

#endif // ASSET_LEVEL_H