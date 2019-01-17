#pragma once
#ifndef	ASSET_MESH_H
#define	ASSET_MESH_H

#include "Assets/Asset.h"
#include "Utilities/IO/Mesh_IO.h"
#include "assimp/scene.h"
#include "GL/glad/glad.h"
#include "glm/glm.hpp"
#include "glm/geometric.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;
class Mesh;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Mesh : public std::shared_ptr<Mesh> {
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
class Mesh : public Asset
{
public:
	/** Destroy the Mesh. */
	~Mesh() = default;
	/** Construct the Mesh. */
	Mesh(const std::string & filename);


	// Public Attributes
	Mesh_Geometry m_geometry;


private:
	// Private Methods
	// Interface Implementation
	void initializeDefault();
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class Shared_Mesh;
};

#endif // ASSET_MESH_H