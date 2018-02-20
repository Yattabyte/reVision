#include "Asset_Collider.h"
#include "Managers\Message_Manager.h"
#include "Utilities\Model_Importer.h"
#include "assimp\postprocess.h"


Asset_Collider::~Asset_Collider()
{
	if (m_shape != nullptr)
		delete m_shape;
}

Asset_Collider::Asset_Collider(const string & filename) : Asset(filename)
{
	m_shape = nullptr;
}

/** Returns a default asset that can be used whenever an asset doesn't exist, is corrupted, or whenever else desired.
 * @brief Uses hard-coded values
 * @param	asset	a shared pointer to fill with the default asset */
void fetch_default_asset(Shared_Asset_Collider & asset)
{
	// Check if a copy already exists
	if (Asset_Manager::Query_Existing_Asset<Asset_Collider>(asset, "defaultCollider"))
		return;
	
	// Create hard-coded alternative
	Asset_Manager::Create_New_Asset<Asset_Collider>(asset, "defaultCollider");
	asset->m_shape = new btBoxShape(btVector3(1, 1, 1));
	Asset_Manager::Add_Work_Order(new Collider_WorkOrder(asset, ""), true);
}

namespace Asset_Loader {
	void load_asset(Shared_Asset_Collider & user, const string & filename, const bool & threaded)
	{
		// Check if a copy already exists
		if (Asset_Manager::Query_Existing_Asset<Asset_Collider>(user, filename))
			return;

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_COLLIDER(filename);
		if (!File_Reader::FileExistsOnDisk(fullDirectory) || (filename == "") || (filename == " ")) {
			MSG_Manager::Error(MSG_Manager::FILE_MISSING, fullDirectory);
			fetch_default_asset(user);
			return;
		}

		// Create the asset
		Asset_Manager::Submit_New_Asset<Asset_Collider, Collider_WorkOrder>(user, threaded, fullDirectory, filename);
	}
}

void Collider_WorkOrder::initializeOrder()
{
	// Attempt to create the asset
	vector<btScalar> points;
	if (!Model_Importer::import_Model(m_filename, aiProcess_Triangulate, points)) {
		fetch_default_asset(m_asset);
		return;
	}

	btConvexHullShape *shape = new btConvexHullShape(&points[0], points.size(), sizeof(btScalar) * 3);
	shape->recalcLocalAabb();
	m_asset->m_shape = shape;
}

void Collider_WorkOrder::finalizeOrder()
{
	if (!m_asset->existsYet()) 
		m_asset->finalize();	
}
