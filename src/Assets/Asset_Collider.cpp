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
		userAsset = assetManager.createNewAsset<Asset_Collider>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string relativePath = DIRECTORY_COLLIDER + filename;
		const std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, relativePath);
		const std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
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
}

void Asset_Collider::finalize(Engine * engine)
{
	Asset::finalize(engine);
}
