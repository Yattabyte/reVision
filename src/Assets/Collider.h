#pragma once
#ifndef	COLLIDER_H
#define	COLLIDER_H

#include "Assets/Mesh.h"


// Forward Declarations
class Collider;
class btCollisionShape;

/** Shared version of a Collider asset.
Responsible for the creation, containing, and sharing of assets. */
class Shared_Collider final : public std::shared_ptr<Collider> {
public:
	// Public (De)Constructors
	/** Constructs an empty asset. */
	inline Shared_Collider() noexcept = default;
	/** Begins the creation process for this asset.
	@param	engine			reference to the engine to use. 
	@param	filename		the filename to use.
	@param	threaded		create in a separate thread.
	@return					the desired asset. */
	Shared_Collider(Engine& engine, const std::string& filename, const bool& threaded = true);
};

/** A collision shape asset used in physics.
Represents a 3D mesh asset tuned for use in physics simulations instead of rendering.
@note	uses Bullet library. */
class Collider final : public Asset {
public:
	// Public (De)Constructors
	/** Destroy the Collider. */
	inline ~Collider() noexcept = default;
	/** Construct the Collider.
	@param	engine		reference to the engine to use. 
	@param	filename	the asset file name (relative to engine directory). */
	Collider(Engine& engine, const std::string& filename) noexcept;


	// Public Attributes
	Shared_Mesh m_mesh;
	std::unique_ptr<btCollisionShape> m_shape;


protected:
	// Private but deleted
	/** Disallow asset move constructor. */
	inline Collider(Collider&&) noexcept = delete;
	/** Disallow asset copy constructor. */
	inline Collider(const Collider&) noexcept = delete;
	/** Disallow asset move assignment. */
	inline const Collider& operator =(Collider&&) noexcept = delete;
	/** Disallow asset copy assignment. */
	inline const Collider& operator =(const Collider&) noexcept = delete;


	// Private Interface Implementation
	void initialize() final;


	// Private Attributes
	friend class Shared_Collider;
};

#endif // COLLIDER_H