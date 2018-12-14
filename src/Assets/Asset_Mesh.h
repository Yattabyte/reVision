#pragma once
#ifndef	ASSET_MESH_H
#define	ASSET_MESH_H

#include "Assets\Asset.h"
#include "Utilities\IO\Mesh_IO.h"
#include "assimp\scene.h"
#include "GL\glad\glad.h"
#include "GLM\glm.hpp"
#include "glm\geometric.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;
class Asset_Mesh;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Mesh : public std::shared_ptr<Asset_Mesh> {
public:
	Shared_Mesh() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Mesh(Engine * engine, const std::string & filename, const bool & threaded = true);
};


/** A 3D geometric mesh. */
class Asset_Mesh : public Asset
{
public:
	/** Destroy the Mesh. */
	~Asset_Mesh() = default;
	/** Construct the Mesh. */
	Asset_Mesh(const std::string & filename);


	// Public Attributes
	Mesh_Geometry m_geometry;


private:
	// Private Methods
	// Interface Implementation
	void initializeDefault();
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MESH_H