#include "Assets/Level.h"
#include "Utilities/IO/Level_IO.h"
#include "Engine.h"


constexpr char* DIRECTORY_LEVEL = "\\Maps\\";

Shared_Level::Shared_Level(Engine * engine, const std::string & filename, const bool & threaded)
{
	(*(std::shared_ptr<Level>*)(this)) = std::dynamic_pointer_cast<Level>(
		engine->getManager_Assets().shareAsset(
			typeid(Level).name(),
			filename,
			[engine, filename]() { return std::make_shared<Level>(engine, filename); },
			threaded
		));
}

Level::Level(Engine * engine, const std::string & filename) : Asset(engine, filename) {}

void Level::initialize()
{
	if (!Level_IO::Import_Level(m_engine, DIRECTORY_LEVEL + getFileName(), m_entities)) {
		m_engine->getManager_Messages().error("Level \"" + m_filename + "\" failed to initialize.");
	}

	Asset::finalize();
}