#include "Modules\World\World_M.h"
#include "Utilities\IO\Level_IO.h"
#include "Engine.h"


World_Module::~World_Module()
{
	// Update indicator
	m_aliveIndicator = false;
}

void World_Module::initialize(Engine * engine)
{
	Engine_Module::initialize(engine);
	m_engine->getMessageManager().statement("Loading Module: World...");

	// Load world
	loadWorld();
}

void World_Module::loadWorld()
{
	m_level = Asset_Level::Create(m_engine, "devTest.map");
	m_level->addCallback(m_aliveIndicator, std::bind(&World_Module::processLevel, this));	
}

void World_Module::addLevelListener(bool * notifier)
{
	m_notifyees.push_back(notifier);
	if (m_finishedLoading)
		*notifier = true;
}

const bool World_Module::checkIfLoaded()
{
	auto & assetManager = m_engine->getAssetManager();
	auto & modelManager = m_engine->getModelManager();
	auto & materialManager = m_engine->getMaterialManager();
	// Firstly, check and see if the following systems are ready
	if (assetManager.readyToUse() &&
		modelManager.readyToUse() &&
		materialManager.readyToUse()) {

		// Lastly, check and see if we observed any changes
		if (assetManager.hasChanged() ||
			modelManager.hasChanged() ||
			materialManager.hasChanged())
			for each (bool * flag in m_notifyees)
				*flag = true;
		return true;
	}
	return false;
}

void World_Module::processLevel()
{
	if (m_level->existsYet()) {
		ECS & ecs = m_engine->getECS();
		std::vector<BaseECSComponent*> components; // holds each entity's components
		std::vector<unsigned int> ids; // holds each entity's component id's
		Component_and_ID outputComponent; // holds a single component
		const char * type;
		for each (auto & lvlEntity in m_level->m_entities) {
			for each (const auto & lvlComponent in lvlEntity.components) {
				// Get component type
				type = lvlComponent.type.c_str();
				// Call the appropriate creator if available
				outputComponent = ecs.constructComponent(type, lvlComponent.parameters);
				// Push back our result if successfull
				if (outputComponent.success()) {
					components.push_back(outputComponent.component);
					ids.push_back(outputComponent.id);
					outputComponent.clear();
				}
			}
			// Make an entity out of the available components
			if (components.size())
				ecs.makeEntity(components.data(), ids.data(), components.size());
			// Delete temporary components and reset for next entity
			for each (auto * component in components)
				delete component;
			components.clear();
			ids.clear();
		}
	}
}
