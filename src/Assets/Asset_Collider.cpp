#include "Asset_Collider.h"
#include "Utilities\IO\Text_IO.h"
#include "Utilities\IO\Model_IO.h"
#include "Engine.h"

#define DIRECTORY_COLLIDER Engine::Get_Current_Dir() + "\\Models\\"
#define ABS_DIRECTORY_COLLIDER(filename) DIRECTORY_COLLIDER + filename 


Asset_Collider::~Asset_Collider()
{
	if (m_shape != nullptr)
		delete m_shape;
}

Asset_Collider::Asset_Collider(const std::string & filename) : Asset(filename)
{
	m_shape = nullptr;
}

Shared_Asset_Collider Asset_Collider::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Collider>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Collider>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string fullDirectory = ABS_DIRECTORY_COLLIDER(filename);
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory) || (filename == "") || (filename == " ")) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Collider::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative
	m_shape = new btBoxShape(btVector3(1, 1, 1));	
}

void Asset_Collider::initialize(Engine * engine, const std::string & fullDirectory)
{
	// Attempt to create the asset
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_hull, dataContainer)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Collider");
		initializeDefault(engine);
		return;
	}
	std::vector<btScalar> orderedPoints;
	orderedPoints.reserve(dataContainer.vertices.size() * 3);
	for each (const auto & vertex in dataContainer.vertices) {
		orderedPoints.push_back(vertex.x);
		orderedPoints.push_back(vertex.y);
		orderedPoints.push_back(vertex.z);
	}
	btConvexHullShape *shape = new btConvexHullShape(&orderedPoints[0], orderedPoints.size(), sizeof(btScalar) * 3);
	shape->recalcLocalAabb();
	m_shape = shape;
}

void Asset_Collider::finalize(Engine * engine)
{
	Asset::finalize(engine);
}
