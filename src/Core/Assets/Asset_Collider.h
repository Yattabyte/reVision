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
#define DIRECTORY_COLLIDER FileReader::GetCurrentDir() + "\\Models\\"
#define ABS_DIRECTORY_COLLIDER(filename) DIRECTORY_COLLIDER + filename 
#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\FileReader.h"
#include <btBulletDynamicsCommon.h>

class Asset_Collider;
typedef shared_ptr<Asset_Collider> Shared_Asset_Collider;
class DT_ENGINE_API Asset_Collider : public Asset
{
public:
	/*************
	----Common----
	*************/

	~Asset_Collider();
	Asset_Collider();
	Asset_Collider(const string &_filename);
	Asset_Collider(btCollisionShape *new_shape);
	static int GetAssetType();


	/****************
	----Variables----
	****************/

	string filename;
	btCollisionShape *shape;
};

namespace Asset_Loader {
	DT_ENGINE_API void load_asset(Shared_Asset_Collider &user, const string & filename, const bool &threaded = true);
};

class Collider_WorkOrder : public Work_Order {
public:
	Collider_WorkOrder(Shared_Asset_Collider &asset, const std::string &filename) : m_asset(asset), m_filename(filename) {};
	~Collider_WorkOrder() {};
	virtual void Initialize_Order();
	virtual void Finalize_Order();

private:
	std::string m_filename;
	Shared_Asset_Collider m_asset;
};

#endif // ASSET_COLLIDER