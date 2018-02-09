#include "Asset_Collider.h"
#include "Managers\Message_Manager.h"
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

// Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
// Uses hardcoded values
void fetchDefaultAsset(Shared_Asset_Collider & asset)
{
	// Check if a copy already exists
	if (Asset_Manager::QueryExistingAsset<Asset_Collider>(asset, "defaultCollider"))
		return;
	
	// Create hardcoded alternative
	Asset_Manager::CreateNewAsset<Asset_Collider>(asset, "defaultCollider");
	asset->shape = new btBoxShape(btVector3(1, 1, 1));
	Asset_Manager::AddWorkOrder(new Collider_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Collider & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::QueryExistingAsset<Asset_Collider>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_COLLIDER(filename);
		if (!FileReader::FileExistsOnDisk(fullDirectory) || (filename == "") || (filename == " ")) {
			MSG::Error(FILE_MISSING, fullDirectory);
			fetchDefaultAsset(user);
			return;
		}

		// Create the asset
		Asset_Manager::CreateNewAsset<Asset_Collider, Collider_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

void Collider_WorkOrder::Initialize_Order()
{
	// Attempt to create the asset
	vector<btScalar> points;
	if (!ModelImporter::Import_Model(m_filename, aiProcess_Triangulate, points)) {
		fetchDefaultAsset(m_asset);
		return;
	}

	btConvexHullShape *shape = new btConvexHullShape(&points[0], points.size(), sizeof(btScalar) * 3);
	shape->recalcLocalAabb();
	m_asset->shape = shape;
}

void Collider_WorkOrder::Finalize_Order()
{
	if (!m_asset->ExistsYet()) 
		m_asset->Finalize();	
}
