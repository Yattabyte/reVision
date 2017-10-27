/*
	Asset_Collider
	
	- A geometric mesh used for collision detection
	- Specific to bullet physics
*/

#pragma once
#ifndef	ASSET_COLLIDER
#define	ASSET_COLLIDER
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif
#include "Assets\Asset.h"
#include <btBulletDynamicsCommon.h>

class Asset_Collider;
typedef shared_ptr<Asset_Collider> Shared_Asset_Collider;
class Asset_Collider : public Asset
{
public:
	DELTA_CORE_API ~Asset_Collider();
	DELTA_CORE_API Asset_Collider();
	DELTA_CORE_API Asset_Collider(const string &_filename);
	DELTA_CORE_API Asset_Collider(btCollisionShape *new_shape);
	DELTA_CORE_API static int GetAssetType();

	// Collider attributes
	string filename;
	btCollisionShape *shape;
};
#endif // ASSET_COLLIDER