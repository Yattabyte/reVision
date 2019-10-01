#include "Modules/World/World_M.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"
#include <fstream>
#include <filesystem>
#include <random>
#include <sstream>


void World_Module::initialize(Engine* engine)
{
	Engine_Module::initialize(engine);
	m_engine->getManager_Messages().statement("Loading Module: World...");
}

void World_Module::deinitialize()
{
	// Update indicator
	m_engine->getManager_Messages().statement("Unloading Module: World...");
	*m_aliveIndicator = false;
	unloadWorld();
}

void World_Module::frameTick(const float& deltaTime)
{
	auto& assetManager = m_engine->getManager_Assets();

	// Firstly, check and see if the following systems are ready
	if (!assetManager.readyToUse())
		return;

	// Signal that the map has finished loading ONCE
	if (m_state == startLoading)
		notifyListeners(finishLoading);
	else if (m_state == finishLoading) {
		// Lastly, check and see if we observed any changes
		if (assetManager.hasChanged())
			notifyListeners(updated);
	}
}

void World_Module::loadWorld(const std::string& mapName)
{
	unloadWorld();

	// Signal that a new map is beginning to load
	notifyListeners(startLoading);

	// Read ecsData from disk
	const auto path = Engine::Get_Current_Dir() + "\\Maps\\" + mapName;
	std::vector<char> ecsData(std::filesystem::file_size(path));
	std::ifstream mapFile(path, std::ios::binary | std::ios::beg);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot read the binary map file from disk!");
	else
		mapFile.read(ecsData.data(), (std::streamsize)ecsData.size());
	mapFile.close();

	m_engine->getModule_ECS().setWorld(ecsWorld(ecsData));
}

void World_Module::saveWorld(const std::string& mapName)
{
	auto& ecsWorld = m_engine->getModule_ECS().getWorld();
	const auto data = ecsWorld.serializeEntities(ecsWorld.getEntityHandles());

	// Write ECS data to disk
	std::fstream mapFile(Engine::Get_Current_Dir() + "\\Maps\\" + mapName, std::ios::binary | std::ios::out);
	if (!mapFile.is_open())
		m_engine->getManager_Messages().error("Cannot write the binary map file to disk!");
	else
		mapFile.write(data.data(), (std::streamsize)data.size());
	mapFile.close();
}

void World_Module::unloadWorld()
{
	// Overwrite with an empty world
	m_engine->getModule_ECS().setWorld(ecsWorld());

	// Signal that the last map has unloaded
	notifyListeners(unloaded);
}

void World_Module::addLevelListener(const std::shared_ptr<bool>& alive, const std::function<void(const WorldState&)>& func)
{
	m_notifyees.push_back(std::make_pair(alive, func));
}

void World_Module::notifyListeners(const WorldState& state)
{
	// Get all callback functions, and call them if their owners are still alive
	for (int x = 0; x < (int)m_notifyees.size(); ++x) {
		const auto& [alive, func] = m_notifyees[x];
		if (alive && *(alive.get()) == true)
			func(state);
		else {
			m_notifyees.erase(m_notifyees.begin() + x);
			x--;
		}
	}
	m_state = state;
}