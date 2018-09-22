#include "Assets\Asset_Level.h"
#include "Utilities\IO\Level_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_LEVEL = "\\Maps\\";

Asset_Level::Asset_Level(const std::string & filename) : Asset(filename) {}

Shared_Asset_Level Asset_Level::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Level>(filename, threaded);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Level>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &relativePath = DIRECTORY_LEVEL + filename;
		const std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, relativePath);
		const std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		
		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Level::initialize(Engine * engine, const std::string & relativePath)
{
	if (!Level_IO::Import_Level(engine, relativePath, m_entities)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Level");
		return;
	}
}

void Asset_Level::finalize(Engine * engine)
{
	Asset::finalize(engine);
}
