#include "Asset_Collider.h"
#include "Utilities\IO\Text_IO.h"
#include "Utilities\IO\Mesh_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_COLLIDER = "\\Models\\";

Asset_Collider::~Asset_Collider()
{
	if (m_shape != nullptr)
		delete m_shape;
}

Asset_Collider::Asset_Collider(const std::string & filename) : Asset(filename) {}

Shared_Asset_Collider Asset_Collider::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Collider>(filename, threaded);
	if (!userAsset) {
		userAsset = std::make_shared<Asset_Collider>(filename);
		assetManager.addShareableAsset(userAsset);

		// Submit the work order
		const std::string relativePath(DIRECTORY_COLLIDER + filename);
		assetManager.submitNewWorkOrder(std::move(std::bind(&initialize, userAsset.get(), engine, relativePath)), threaded);
	}
	return userAsset;
}

void Asset_Collider::initialize(Engine * engine, const std::string & relativePath)
{
	// Forward asset creation
	m_mesh = Asset_Mesh::Create(engine, relativePath, false);
	std::vector<btScalar> orderedPoints;
	orderedPoints.reserve(m_mesh->m_geometry.vertices.size() * 3);
	for each (const auto & vertex in m_mesh->m_geometry.vertices) {
		orderedPoints.push_back(vertex.x);
		orderedPoints.push_back(vertex.y);
		orderedPoints.push_back(vertex.z);
	}
	btConvexHullShape *shape = new btConvexHullShape(&orderedPoints[0], (int)orderedPoints.size(), sizeof(btScalar) * 3);
	shape->recalcLocalAabb();
	m_shape = shape;

	Asset::finalize(engine);
}
