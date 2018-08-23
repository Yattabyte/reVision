#pragma once
#ifndef	ASSET_COLLIDER_H
#define	ASSET_COLLIDER_H

#include "Assets\Asset.h"
#include <btBulletDynamicsCommon.h>


class Engine;
class Asset_Collider;
typedef std::shared_ptr<Asset_Collider> Shared_Asset_Collider;

/** A 3D mesh tuned for use in physics simulations instead of rendering. */
class Asset_Collider : public Asset
{
public:
	/** Destroy the Collider. */
	~Asset_Collider();


	/** Begins the creation process for this asset.
	@param	engine			the engine being used
	@param	filename		the filename to use
	@param	threaded		create in a separate thread
	@return					the desired asset */
	static Shared_Asset_Collider Create(Engine * engine, const std::string & filename, const bool & threaded = true);
	
	
	// Public Attributes
	btCollisionShape * m_shape;


private:
	// Private Constructors
	/** Construct the Collider. */
	Asset_Collider(const std::string & filename);


	// Private Methods
	// Interface Implementation
	virtual void initializeDefault(Engine * engine);
	virtual void initialize(Engine * engine, const std::string & fullDirectory);
	virtual void finalize(Engine * engine);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_COLLIDER_H