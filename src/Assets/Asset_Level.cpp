#include "Assets\Asset_Level.h"
#include "Utilities\IO\Level_IO.h"
#include "Engine.h"

#define DIRECTORY_LEVEL Engine::Get_Current_Dir() + "\\Maps\\"
#define ABS_DIRECTORY_LEVEL(filename) DIRECTORY_LEVEL + filename 


Asset_Level::Asset_Level(const std::string & filename) : Asset(filename) {}

Shared_Asset_Level Asset_Level::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	AssetManager & assetManager = engine->getAssetManager();

	// Create the asset or find one that already exists
	auto userAsset = assetManager.queryExistingAsset<Asset_Level>(filename);
	if (!userAsset) {
		userAsset = assetManager.createNewAsset<Asset_Level>(filename);
		auto & assetRef = *userAsset.get();

		// Check if the file/directory exists on disk
		const std::string &fullDirectory = ABS_DIRECTORY_LEVEL(filename);
		std::function<void()> initFunc = std::bind(&initialize, &assetRef, engine, fullDirectory);
		std::function<void()> finiFunc = std::bind(&finalize, &assetRef, engine);
		if (!Engine::File_Exists(fullDirectory)) {
			engine->reportError(MessageManager::FILE_MISSING, fullDirectory);
			initFunc = std::bind(&initializeDefault, &assetRef, engine);
		}

		// Submit the work order
		assetManager.submitNewWorkOrder(userAsset, threaded, initFunc, finiFunc);
	}
	return userAsset;
}

void Asset_Level::initializeDefault(Engine * engine)
{
	// Create hard-coded alternative	
}

void Asset_Level::initialize(Engine * engine, const std::string & fullDirectory)
{
	if (!Level_IO::Import_Level(engine, fullDirectory, m_entities)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Level");
		initializeDefault(engine);
		return;
	}
}

void Asset_Level::finalize(Engine * engine)
{
	Asset::finalize(engine);
}
