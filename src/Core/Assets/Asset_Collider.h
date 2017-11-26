/*
	Asset_Collider
	
	- A geometric mesh used for collision detection
	- Specific to bullet physics
*/

#pragma once
#ifndef	ASSET_COLLIDER
#define	ASSET_COLLIDER
#ifdef	ENGINE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "Assets\Asset.h"
#include "Systems\Asset_Manager.h"
#include <btBulletDynamicsCommon.h>

class Asset_Collider;
typedef shared_ptr<Asset_Collider> Shared_Asset_Collider;
class Asset_Collider : public Asset
{
public:
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~Asset_Collider();
	DELTA_CORE_API Asset_Collider();
	DELTA_CORE_API Asset_Collider(const string &_filename);
	DELTA_CORE_API Asset_Collider(btCollisionShape *new_shape);
	DELTA_CORE_API static int GetAssetType();

	/****************
	----Variables----
	****************/

	string filename;
	btCollisionShape *shape;
};
namespace Asset_Manager {
	DELTA_CORE_API void load_asset(Shared_Asset_Collider &user, const string & filename, const bool &threaded = true);
};
#endif // ASSET_COLLIDER