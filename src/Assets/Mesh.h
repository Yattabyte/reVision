#pragma once
#ifndef	MESH_H
#define	MESH_H

#include "Assets/Asset.h"
#include "Utilities/IO/Mesh_IO.h"


// Forward Declarations
class Mesh;

/** Shared version of a Mesh asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Mesh final : public std::shared_ptr<Mesh> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Mesh() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	explicit Shared_Mesh(Engine& engine, const std::string& filename, const bool& threaded = true);
};


/** A data set representing a 3D mesh.
This asset contains raw unformatted geometric data. */
class Mesh final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Mesh. */
	~Mesh() noexcept = default;
	/** Construct the Mesh.
	@param	engine		reference to the engine to use. 
	@param	filename	the asset file name (relative to engine directory). */
	Mesh(Engine& engine, const std::string& filename) noexcept;


	// Public Attributes
	Mesh_Geometry m_geometry;


private:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Mesh(Mesh&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Mesh(const Mesh&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Mesh& operator =(Mesh&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Mesh& operator =(const Mesh&) noexcept = delete;


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Mesh;
};

#endif // MESH_H