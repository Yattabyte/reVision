#include "Assets\Asset_Level.h"
#include "Utilities\IO\Level_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_LEVEL = "\\Maps\\";

Asset_Level::Asset_Level(const std::string & filename) : Asset(filename) {}

Shared_Asset_Level Asset_Level::Create(Engine * engine, const std::string & filename, const bool & threaded)
{
	return engine->getAssetManager().createAsset<Asset_Level>(
		filename,
		DIRECTORY_LEVEL,
		"",
		&initialize,
		engine,
		threaded
	);
}

void Asset_Level::initialize(Engine * engine, const std::string & relativePath)
{
	if (!Level_IO::Import_Level(engine, relativePath, m_entities)) {
		engine->reportError(MessageManager::ASSET_FAILED, "Asset_Level");
	}

	Asset::finalize(engine);
}