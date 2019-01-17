#pragma once
#ifndef	ASSET_COLLIDER_H
#define	ASSET_COLLIDER_H

#include "Assets/Mesh.h"
#include <btBulletDynamicsCommon.h>


class Engine;
class Collider;

/** Responsible for the creation, containing, and sharing of assets. */
class Shared_Collider : public std::shared_ptr<Collider> {
public:
	Shared_Collider() = default;
	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	explicit Shared_Collider(Engine * engine, const std::string & filename, const bool & threaded = true);
};

/** A 3D mesh tuned for use in physics simulations instead of rendering. */
class Collider : public Asset
{
public:
	/** Destroy the Collider. */
	~Collider() = default;
	/** Construct the Collider. */
	Collider(const std::string & filename);
	
	
	// Public Attributes
	Shared_Mesh m_mesh;
	btCollisionShape * m_shape;


protected:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class Shared_Collider;
};

#endif // ASSET_COLLIDER_H