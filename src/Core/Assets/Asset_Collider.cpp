#include "Asset_Collider.h"
#include "Systems\Message_Manager.h"
#include "Utilities\ModelImporter.h"
#include "assimp\postprocess.h"

/* -----ASSET TYPE----- */
#define ASSET_TYPE 0

Asset_Collider::~Asset_Collider()
{
	if (shape != nullptr)
		delete shape;
}

Asset_Collider::Asset_Collider(const string & filename) : Asset(filename)
{
	shape = nullptr;
}

int Asset_Collider::GetAssetType()
{
	return ASSET_TYPE;
}

Shared_Asset_Collider fetchDefaultAsset()
{
	shared_lock<shared_mutex> guard(Asset_Manager::GetMutex_Assets());
	std::map<int, Shared_Asset> &fallback_assets = Asset_Manager::GetFallbackAssets_Map();
	fallback_assets.insert(std::pair<int, Shared_Asset>(Asset_Collider::GetAssetType(), Shared_Asset()));
	auto &default_asset = fallback_assets[Asset_Collider::GetAssetType()];
	guard.unlock();
	guard.release();
	if (default_asset.get() == nullptr)
		default_asset = shared_ptr<Asset_Collider>(new Asset_Collider("defaultCollider"));
	return dynamic_pointer_cast<Asset_Collider>(default_asset);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Collider & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		shared_mutex  &mutex_IO_assets = Asset_Manager::GetMutex_Assets();
		auto &assets_colliders = (Asset_Manager::GetAssets_List(Asset_Collider::GetAssetType()));
		{
			shared_lock<shared_mutex> guard(mutex_IO_assets);
			for each (const auto &asset in assets_colliders) {
				shared_lock<shared_mutex> asset_guard(asset->m_mutex);
				const Shared_Asset_Collider derived_asset = dynamic_pointer_cast<Asset_Collider>(asset);
				if (derived_asset) {
					if (derived_asset->GetFileName() == filename) {
						asset_guard.unlock();
						asset_guard.release();
						user = derived_asset;
						// Can't guarantee that the asset isn't already being worked on, so no finalization here if threaded
						return;
					}
				}
			}
		}
		
		// Attempt to create the asset
		const std::string &fulldirectory = ABS_DIRECTORY_COLLIDER(filename);
		if (!FileReader::FileExistsOnDisk(fulldirectory) || (filename == "") || (filename == " ")) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultAsset();
			return;
		}

		{
			// Attempt to create the asset
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Collider(new Asset_Collider(filename));
			assets_colliders.push_back(user);
		}

		if (threaded)
			Asset_Manager::AddWorkOrder(new Collider_WorkOrder(user, fulldirectory));
		else {
			Collider_WorkOrder work_order(user, fulldirectory);
			work_order.Initialize_Order();
			work_order.Finalize_Order();
		}		
	}
}

void Collider_WorkOrder::Initialize_Order()
{
	// Attempt to create the asset
	vector<btScalar> points;
	if (!ModelImporter::Import_Model(m_filename, aiProcess_Triangulate, points)) {
		m_asset = fetchDefaultAsset();
		return;
	}

	btConvexHullShape *shape = new btConvexHullShape(&points[0], points.size(), sizeof(btScalar) * 3);
	shape->recalcLocalAabb();
	m_asset->shape = shape;
}

void Collider_WorkOrder::Finalize_Order()
{
	shared_lock<shared_mutex> read_guard(m_asset->m_mutex);
	if (!m_asset->ExistsYet()) {
		read_guard.unlock();
		read_guard.release();
		unique_lock<shared_mutex> write_guard(m_asset->m_mutex);
		m_asset->Finalize();
	}
}
