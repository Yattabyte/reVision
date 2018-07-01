#pragma once
#ifndef	ASSET_COLLIDER_H
#define	ASSET_COLLIDER_H
#define DIRECTORY_COLLIDER File_Reader::GetCurrentDir() + "\\Models\\"
#define ABS_DIRECTORY_COLLIDER(filename) DIRECTORY_COLLIDER + filename 
#include "Assets\Asset.h"
#include "Managers\AssetManager.h"
#include "Utilities\File_Reader.h"
#include <btBulletDynamicsCommon.h>

class Asset_Collider;
class Asset_Manager;
typedef shared_ptr<Asset_Collider> Shared_Asset_Collider;


/** 
 * A 3D mesh tuned for use in physics simulations instead of rendering.
 **/
class Asset_Collider : public Asset
{
public:
	/** Destroy the Collider. */
	~Asset_Collider();


	/** Creates a default asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container */
	static void CreateDefault(AssetManager & assetManager, Shared_Asset_Collider & userAsset);
	/** Begins the creation process for this asset.
	 * @param	assetManager	the asset manager to use
	 * @param	userAsset		the desired asset container
	 * @param	filename		the filename to use
	 * @param	threaded		create in a separate thread */
	static void Create(AssetManager & assetManager, Shared_Asset_Collider & userAsset, const string & filename, const bool & threaded = true);
	


	// Public Attributes
	btCollisionShape * m_shape;


private:
	// Private Constructors
	/** Construct the Collider. */
	Asset_Collider(const string & filename);


	// Private Methods
	/** Initializes the asset. */
	static void Initialize(AssetManager & assetManager, Shared_Asset_Collider & userAsset, const string & fullDirectory);
	/** Finalizes the asset. */
	static void Finalize(AssetManager & assetManager, Shared_Asset_Collider & userAsset);


	// Private Attributes
	friend class AssetManager;
};

#endif // ASSET_COLLIDER_H