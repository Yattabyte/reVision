#pragma once
#ifndef	COLLIDER_H
#define	COLLIDER_H

#include "Assets/Mesh.h"
#include <btBulletDynamicsCommon.h>


class Engine;
class Collider;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Collider : public std::shared_ptr<Collider> {
public:
	// Public (de)Constructors
	/** Constructs an empty asset. */
	inline Shared_Collider() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Collider(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** A 3D mesh tuned for use in physics simulations instead of rendering. */
class Collider : public Asset {
public:
	// Public (de)Constructors
	/** Destroy the Collider. */
	inline ~Collider() = default;
	/** Construct the Collider. */
	Collider(Engine * engine, const std::string & filename);
	
	
	// Public Attributes
	Shared_Mesh m_mesh;
	btCollisionShape * m_shape = nullptr;


protected:
	// Private Interface Implementation
	virtual void initialize() override;


	// Private Attributes
	friend class Shared_Collider;
};

#endif // COLLIDER_H