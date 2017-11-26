/*
	Asset_Collider
	
	- A geometric mesh used for collision detection
	- Specific to bullet physics
*/

#pragma once
#ifndef	ASSET_COLLIDER
#define	ASSET_COLLIDER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
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

	DT_ENGINE_API ~Asset_Collider();
	DT_ENGINE_API Asset_Collider();
	DT_ENGINE_API Asset_Collider(const string &_filename);
	DT_ENGINE_API Asset_Collider(btCollisionShape *new_shape);
	DT_ENGINE_API static int GetAssetType();

	/****************
	----Variables----
	****************/

	string filename;
	btCollisionShape *shape;
};
namespace Asset_Manager {
	DT_ENGINE_API void load_asset(Shared_Asset_Collider &user, const string & filename, const bool &threaded = true);
};
#endif // ASSET_COLLIDER