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

void Asset_Collider::CreateDefault(Engine * engine, Shared_Asset_Collider & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, "defaultCollider"))
		return;

	// Create hard-coded alternative
	assetManager.createNewAsset(userAsset, "defaultCollider");
	userAsset->m_shape = new btBoxShape(btVector3(1, 1, 1));
	assetManager.submitNewWorkOrder(userAsset, true,
		/* Initialization. */
		[]() {},
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); }
	);
}

void Asset_Collider::Create(Engine * engine, Shared_Asset_Collider & userAsset, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = ABS_DIRECTORY_COLLIDER(filename);
	if (!Engine::File_Exists(fullDirectory) || (filename == "") || (filename == " ")) {
		engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
		CreateDefault(engine, userAsset);
		return;
	}

	// Create the asset
	assetManager.submitNewAsset(userAsset, threaded,
		/* Initialization. */
		[engine, &userAsset, fullDirectory]() mutable { Initialize(engine, userAsset, fullDirectory); },
		/* Finalization. */
		[engine, &userAsset]() mutable { Finalize(engine, userAsset); },
		filename
	);
}

void Asset_Collider::Initialize(Engine * engine, Shared_Asset_Collider & userAsset, const std::string & fullDirectory)
{
	// Attempt to create the asset
	Model_Geometry dataContainer;
	if (!Model_IO::Import_Model(engine, fullDirectory, import_hull, dataContainer)) {
		CreateDefault(engine, userAsset);
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
	userAsset->m_shape = shape;
}

void Asset_Collider::Finalize(Engine * engine, Shared_Asset_Collider & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();
	userAsset->finalize();
	
	// Notify Completion
	std::shared_lock<std::shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.first, qwe.second);
}
