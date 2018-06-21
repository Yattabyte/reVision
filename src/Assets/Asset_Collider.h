#pragma once
#ifndef	ASSET_COLLIDER_H
#define	ASSET_COLLIDER_H
#define DIRECTORY_COLLIDER File_Reader::GetCurrentDir() + "\\Models\\"
#define ABS_DIRECTORY_COLLIDER(filename) DIRECTORY_COLLIDER + filename 
#include "Assets\Asset.h"
#include "Managers\Asset_Manager.h"
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


	/** Attempts to create an asset from disk or share one if it already exists. */
	static void Create(Shared_Asset_Collider & userAsset, const string & filename, const bool & threaded = true);


	// Public Attributes
	btCollisionShape * m_shape;


private:
	/** Construct the Collider. */
	Asset_Collider(const string & filename);
	friend class Asset_Manager;
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

#endif // ASSET_COLLIDER_H