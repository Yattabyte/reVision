#pragma once
#ifndef	ASSET_COLLIDER_H
#define	ASSET_COLLIDER_H

#include "Assets\Asset_Mesh.h"
#include <btBulletDynamicsCommon.h>


class Engine;
class Asset_Collider;
using Shared_Asset_Collider = std::shared_ptr<Asset_Collider>;

/** A 3D mesh tuned for use in physics simulations instead of rendering. */
class Asset_Collider : public Asset
{
public:
	/** Destroy the Collider. */
	~Asset_Collider() = default;
	/** Construct the Collider. */
	Asset_Collider(const std::string & filename);


	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Collider Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	
	
	// Public Attributes
	Shared_Asset_Mesh m_mesh;
	std::unique_ptr<btCollisionShape> m_shape;


protected:
	// Private Methods
	// Interface Implementation
	virtual void initialize(Engine * engine, const std::string & relativePath) override;


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_COLLIDER_H