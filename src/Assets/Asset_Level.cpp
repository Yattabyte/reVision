#include "Assets/Asset_Level.h"
#include "Utilities/IO/Level_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_LEVEL = "\\Maps\\";

Shared_Level::Shared_Level(Engine * engine, const std::string & filename, const bool & threaded)
	: std::shared_ptr<Asset_Level>(std::dynamic_pointer_cast<Asset_Level>(engine->getManager_Assets().shareAsset(typeid(Asset_Level).name(), filename)))
{
	// Find out if the asset needs to be created
	if (!get()) {
		// Create new asset on shared_ptr portion of this class 
		(*(std::shared_ptr<Asset_Level>*)(this)) = std::make_shared<Asset_Level>(filename);
		// Submit data to asset manager
		engine->getManager_Assets().submitNewAsset(typeid(Asset_Level).name(), (*(std::shared_ptr<Asset>*)(this)), std::move(std::bind(&Asset_Level::initialize, get(), engine, (DIRECTORY_LEVEL + filename))), threaded);
	}
	// Check if we need to wait for initialization
	else
		if (!threaded)
			// Stay here until asset finalizes
			while (!get()->existsYet())
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

Asset_Level::Asset_Level(const std::string & filename) : Asset(filename) {}

void Asset_Level::initialize(Engine * engine, const std::string & relativePath)
{
	if (!Level_IO::Import_Level(engine, relativePath, m_entities)) {
		engine->getManager_Messages().error("Asset_Level \"" + m_filename + "\" failed to initialize.");
	}

	Asset::finalize(engine);
}