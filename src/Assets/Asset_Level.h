#pragma once
#ifndef	ASSET_LEVEL_H
#define	ASSET_LEVEL_H

#include "Assets\Asset.h"
#include <any>
#include <optional>
#include <string>


class Engine;
class Asset_Level;
using Shared_Asset_Level = std::shared_ptr<Asset_Level>;
struct LevelStruct_Entity;
struct LevelStruct_Component;

/** An asset which represents some intermediate form of a world level. */
class Asset_Level : public Asset
{
public:
	/** Destroy the Level. */
	~Asset_Level() = default;
	/** Construct the Level. */
	Asset_Level(const std::string & filename);


	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Level Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	
	
	// Public Attributes
	std::vector<LevelStruct_Entity> m_entities;

private:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};

struct LevelStruct_Component {
	std::string type;
	std::vector<std::any> parameters;
};
struct LevelStruct_Entity {
	std::vector<LevelStruct_Component> components;
};

#endif // ASSET_LEVEL_H