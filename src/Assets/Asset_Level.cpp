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
		userAsset = std::make_shared<Asset_Level>(filename);
		assetManager.addShareableAsset(userAsset);

		// Submit the work order
		const std::string relativePath(DIRECTORY_LEVEL + filename);		
		assetManager.submitNewWorkOrder(std::move(std::bind(&initialize, userAsset.get(), engine, relativePath)), threaded);
	}
	return userAsset;
}

void Asset_Level::initialize(Engine * engine, const std::string & relativePath)
{
	if (!Level_IO::Import_Level(engine, relativePath, m_entities)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Level");
	}

	Asset::finalize(engine);
}