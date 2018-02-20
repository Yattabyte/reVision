#pragma once
#ifndef	ASSET_COLLIDER
#define	ASSET_COLLIDER
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif
#define DIRECTORY_COLLIDER File_Reader::GetCurrentDir() + "\\Models\\"
#define ABS_DIRECTORY_COLLIDER(filename) DIRECTORY_COLLIDER + filename 
#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
#include "Utilities\File_Reader.h"
#include <btBulletDynamicsCommon.h>

class Asset_Collider;
typedef shared_ptr<Asset_Collider> Shared_Asset_Collider;


/** 
 * A 3D mesh tuned for use in physics simulations instead of rendering.
 **/
class DT_ENGINE_API Asset_Collider : public Asset
{
public:
	// (de)Constructors
	/** Destroy the Collider. */
	~Asset_Collider();
	/** Construct the Collider. */
	Asset_Collider(const string & filename);


	// Public Attributes
	btCollisionShape * shape;
};

/**
 * Namespace that provides functionality for loading assets.
 **/
namespace Asset_Loader {
	/** Attempts to create an asset from disk or share one if it already exists. */
	DT_ENGINE_API void load_asset(Shared_Asset_Collider & user, const string & filename, const bool & threaded = true);
};

/**
 * Implements a work order for Collider Assets.
 **/
class Collider_WorkOrder : public Work_Order {
public:
	/** Constructs an Asset_Collider work order. */
	Collider_WorkOrder(Shared_Asset_Collider & asset, const std::string & filename) : m_asset(asset), m_filename(filename) {};
	~Collider_WorkOrder() {};
	virtual void initializeOrder();
	virtual void finalizeOrder();


private:
	// Private Attributes
	string m_filename;
	Shared_Asset_Collider m_asset;
};

#endif // ASSET_COLLIDER