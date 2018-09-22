#pragma once
#ifndef	ASSET_MESH_H
#define	ASSET_MESH_H

#include "Assets\Asset.h"
#include "Utilities\IO\Mesh_IO.h"
#include "assimp\scene.h"
#include "GL\glew.h"
#include "GLM\glm.hpp"
#include "glm\geometric.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;
class Asset_Mesh;
using Shared_Asset_Mesh = std::shared_ptr<Asset_Mesh>;

/** A 3D geometric mesh. */
class Asset_Mesh : public Asset
{
public:
	/** Destroy the Mesh. */
	~Asset_Mesh();


	// Public Methods
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Mesh Create(Engine * engine, const std::string & filename, const bool & threaded = true);


	// Public Attributes
	Mesh_Geometry						m_geometry;


private:
	// Private Constructors
	/** Construct the Mesh. */
	Asset_Mesh(const std::string & filename);


	// Private Methods
	// Interface Implementation
	void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & fullDirectory) override;
	virtual void finalize(Engine * engine) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_MESH_H