#pragma once
#ifndef	MESH_H
#define	MESH_H

#include "Assets/Asset.h"
#include "Utilities/GL/glad/glad.h"
#include "Utilities/IO/Mesh_IO.h"
#include "assimp/scene.h"
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
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Mesh() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Mesh(Engine * engine, const std::string & filename, const bool & threaded = true);
};


/** A 3D geometric mesh. */
class Mesh : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Mesh. */
	~Mesh() = default;
	/** Construct the Mesh. */
	Mesh(Engine * engine, const std::string & filename);


	// Public Attributes
	Mesh_Geometry m_geometry;


private:
	// Private Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Mesh;
};

#endif // MESH_H