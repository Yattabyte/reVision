#include "Modules\World\World_M.h"
#include "Modules\Graphics\Graphics_M.h"
#include "Modules\Physics\Physics_M.h"
#include "Utilities\IO\Level_IO.h"
#include "Engine.h"

/* Component Types Used */
#include "ECS/Components/BasicPlayer_C.h"
#include "ECS/Components/Prop_C.h"
#include "ECS/Components/Skeleton_C.h"
#include "ECS/Components/LightDirectional_C.h"
#include "ECS/Components/LightPoint_C.h"
#include "ECS/Components/LightSpot_C.h"
#include "ECS/Components/Collider_C.h"
#include "ECS/Components/Reflector_C.h"
#include "ECS/Components/Skeleton_C.h"
#include "ECS/Components/Transform_C.h"

/* Effect Types Used */
#include "Modules\Graphics\Effects\LightDirectional_FX.h"
#include "Modules\Graphics\Effects\LightPoint_FX.h"
#include "Modules\Graphics\Effects\LightSpot_FX.h"
#include "Modules\Graphics\Effects\PropRendering_FX.h"
#include "Modules\Graphics\Effects\Reflector_FX.h"


World_Module::~World_Module()
{
	// Update indicator
	m_aliveIndicator = false;
}

World_Module::World_Module(Engine * engine) : Engine_Module(engine) {}

void World_Module::initialize()
{
	m_engine->getMessageManager().statement("Loading Module: World...");
	auto & physics = m_engine->getPhysicsModule();
	auto & graphics = m_engine->getGraphicsModule();
	auto & lightDir = *graphics.getEffect<LightDirectional_Effect>();
	auto & lightPoint = *graphics.getEffect<LightPoint_Effect>();
	auto & lightSpot = *graphics.getEffect<LightSpot_Effect>();
	auto & prop = *graphics.getEffect<PropRendering_Effect>();
	auto & ref = *graphics.getEffect<Reflector_Effect>();
	m_constructorMap["BasicPlayer_Component"] = new BasicPlayer_Constructor();
	m_constructorMap["LightDirectional_Component"] = new LightDirectional_Constructor(&lightDir.m_lightBuffer);
	m_constructorMap["LightDirectionalShadow_Component"] = new LightDirectionalShadow_Constructor(&lightDir.m_shadowBuffer, &lightDir.m_shadowFBO);
	m_constructorMap["LightPoint_Component"] = new LightPoint_Constructor(&lightPoint.m_lightBuffer);
	m_constructorMap["LightPointShadow_Component"] = new LightPointShadow_Constructor(&lightPoint.m_shadowBuffer, &lightPoint.m_shadowFBO);
	m_constructorMap["LightSpot_Component"] = new LightSpot_Constructor(&lightSpot.m_lightBuffer);
	m_constructorMap["LightSpotShadow_Component"] = new LightSpotShadow_Constructor(&lightSpot.m_shadowBuffer, &lightSpot.m_shadowFBO);
	m_constructorMap["Collider_Component"] = new Collider_Constructor(m_engine);
	m_constructorMap["Prop_Component"] = new Prop_Constructor(m_engine, &prop.m_propBuffer);
	m_constructorMap["Reflector_Component"] = new Reflector_Constructor(&graphics.m_cameraBuffer, &ref.m_reflectorBuffer, &ref.m_envmapFBO);
	m_constructorMap["Skeleton_Component"] = new Skeleton_Constructor(m_engine, &prop.m_skeletonBuffer);
	m_constructorMap["Transform_Component"] = new Transform_Constructor();

	loadWorld();
}

void World_Module::loadWorld()
{
	m_level = Asset_Level::Create(m_engine, "physTest.map");
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
				if (m_constructorMap.find(type))
					outputComponent = m_constructorMap[type]->construct(lvlComponent.parameters);
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
