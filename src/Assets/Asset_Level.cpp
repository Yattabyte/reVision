#include "Assets\Asset_Level.h"
#include "Utilities\IO\Level_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_LEVEL = "\\Maps\\";

Shared_Level::Shared_Level(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Asset_Level>(engine->getAssetManager().createAsset<Asset_Level>(
		filename,
		DIRECTORY_LEVEL,
		"",
		engine,
		threaded
		)) {}

Asset_Level::Asset_Level(const std::string & filename) : Asset(filename) {}

void Asset_Level::initialize(Engine * engine, const std::string & relativePath)
{
	if (!Level_IO::Import_Level(engine, relativePath, m_entities)) {
		engine->getMessageManager().error("Asset_Level \"" + m_filename + "\" failed to initialize.");
	}

	Asset::finalize(engine);
}