#pragma once
#ifndef	MESH_H
#define	MESH_H

#include "Assets/Asset.h"
#include "Utilities/IO/Mesh_IO.h"
#include "glm/glm.hpp"
#include "glm/geometric.hpp"
#include <map>
#include <string>
#include <vector>


class Engine;
class Mesh;

/** Shared version of a Mesh asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Mesh final : public std::shared_ptr<Mesh> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Mesh() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used.
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Mesh(Engine* engine, const std::string& filename, const bool& threaded = true);
};


/** A data set representing a 3D mesh.
This asset contains raw unformatted geometric data. */
class Mesh final : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Mesh. */
	~Mesh() = default;
	/** Construct the Mesh.
	@param	engine		the engine to use.
	@param	filename	the asset file name (relative to engine directory). */
	Mesh(Engine* engine, const std::string& filename);


	// Public Attributes
	Mesh_Geometry m_geometry;


private:
	// Private Interface Implementation
	virtual void initialize() override final;


	// Private Attributes
	friend class Shared_Mesh;
};

#endif // MESH_H