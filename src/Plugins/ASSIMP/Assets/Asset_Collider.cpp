/*
	dt_ASSIMP: Asset_Collider

	- ASSIMP specific implementation of Asset_Collider
*/

#include "Assets\Asset_Collider.h"
#include "Managers\Message_Manager.h"
#include "dt_ASSIMP.h"
#include "ASSIMP\Importer.hpp"
#include "ASSIMP\postprocess.h"
#include "ASSIMP\scene.h"

using namespace Asset_Manager;

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
		const std::string &fulldirectory = getCurrentDir() + "\\Models\\" + filename;
		if (!fileOnDisk(fulldirectory)) {
			MSG::Error(FILE_MISSING, fulldirectory);
			user = fetchDefaultAsset();
			return;
		}

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(fulldirectory, aiProcess_Triangulate);
		if (!scene) {
			MSG::Error(FILE_CORRUPT, fulldirectory);
			user = fetchDefaultAsset();
			return;
		}

		{
			unique_lock<shared_mutex> guard(mutex_IO_assets);
			user = Shared_Asset_Collider(new Asset_Collider(filename));
			assets_colliders.push_back(user);
		}

		btConvexHullShape *shape = new btConvexHullShape();
		for (int a = 0, meshCount = scene->mNumMeshes; a < meshCount; ++a) {
			const aiMesh* mesh = scene->mMeshes[a];
			for (int b = 0, faceCount = mesh->mNumFaces; b < faceCount; ++b) {
				const aiFace& face = mesh->mFaces[b];
				for (int c = 0, indCount = face.mNumIndices; c < indCount; ++c) {
					const aiVector3D &aiv = mesh->mVertices[face.mIndices[c]];
					shape->addPoint(btVector3(aiv.x, aiv.y, aiv.z), false);
				}
			}
		}
		shape->recalcLocalAabb();
		user->shape = shape;
	}
}