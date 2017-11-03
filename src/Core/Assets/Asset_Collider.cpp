#include "Asset_Collider.h"
#include "Managers\Message_Manager.h"
#include "Utilities\ModelImporter.h"
#include "assimp\postprocess.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 0

using namespace Asset_Manager;

Asset_Collider::~Asset_Collider()
{
	if (shape != nullptr)
		delete shape;
}

Asset_Collider::Asset_Collider()
{
	shape = nullptr;
	finalized = false;
}

Asset_Collider::Asset_Collider(const string & _filename) : Asset_Collider()
{
	filename = _filename;
}

Asset_Collider::Asset_Collider(btCollisionShape *new_shape)
{
	shape = new_shape;
	finalized = true;
}

int Asset_Collider::GetAssetType()
{
	return ASSET_TYPE;
}

Shared_Asset_Collider &fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(getMutexIOAssets());
	std::map<int, Shared_Asset> &fallback_assets = getFallbackAssets();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Collider::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Collider::GetAssetType()];
	if (default_asset.get() == nullptr)
		default_asset = shared_ptr<Asset_Collider>(new Asset_Collider());
	return dynamic_pointer_cast<Asset_Collider>(default_asset);
}

namespace Asset_Manager {
	void load_asset(Shared_Asset_Collider &user, const string &filename, const bool &threaded)
	{
		// Check if a copy already exists
		shared_mutex &mutex_IO_assets = getMutexIOAssets();
		auto &assets_colliders = (fetchAssetList(Asset_Collider::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (const auto &asset in assets_colliders) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Collider derived_asset = dynamic_pointer_cast<Asset_Collider>(asset);
				if (derived_asset) {
					if (derived_asset->filename == filename) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						return;
					}
				}
			}
		}

		// Attempt to create the asset
		vector<btScalar> points;
		const std::string &fulldirectory = getCurrentDir() + "\\Models\\" + filename;
		if (!ModelImporter::Import_Model(fulldirectory, aiProcess_Triangulate, points)) {
			user = fetchDefaultAsset();
			return;
		}

		{			
			// Locks the asset map before adding a new asset!
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Collider(new Asset_Collider(filename));
			assets_colliders.push_back(user);
		}

		btConvexHullShape *shape = new btConvexHullShape(&points[0], points.size(), sizeof(btScalar)*3);
		shape->recalcLocalAabb();
		user->shape = shape;
	}
}