#include "Asset_Collider.h"
#include "Utilities\Model_Importer.h"
#include "Engine.h"
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

void Asset_Collider::Create(Engine * engine, Shared_Asset_Collider & userAsset, const string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Check if a copy already exists
	if (assetManager.queryExistingAsset(userAsset, filename))
		return;

	// Check if the file/directory exists on disk
	const std::string &fullDirectory = ABS_DIRECTORY_COLLIDER(filename);
	if (!File_Reader::FileExistsOnDisk(fullDirectory) || (filename == "") || (filename == " ")) {
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

void Asset_Collider::Initialize(Engine * engine, Shared_Asset_Collider & userAsset, const string & fullDirectory)
{
	// Attempt to create the asset
	vector<btScalar> points;
	if (!Model_Importer::import_Model(engine->getMessageManager(), fullDirectory, aiProcess_Triangulate, points)) {
		CreateDefault(engine, userAsset);
		return;
	}

	btConvexHullShape *shape = new btConvexHullShape(&points[0], points.size(), sizeof(btScalar) * 3);
	shape->recalcLocalAabb();
	userAsset->m_shape = shape;
}

void Asset_Collider::Finalize(Engine * engine, Shared_Asset_Collider & userAsset)
{
	AssetManager & assetManager = engine->getAssetManager();

	unique_lock<shared_mutex> write_guard(userAsset->m_mutex);
	userAsset->m_finalized = true;
	write_guard.unlock();
	write_guard.release();
	shared_lock<shared_mutex> read_guard(userAsset->m_mutex);
	for each (auto qwe in userAsset->m_callbacks)
		assetManager.submitNotifyee(qwe.second);
	/* To Do: Finalize call here*/
}
