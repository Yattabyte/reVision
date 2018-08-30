#include "Modules\World\World_M.h"
#include "Modules\Graphics\Graphics_M.h"
#include "Engine.h"
#include "glm\gtc\matrix_transform.hpp"

/* Component Types Used */
#include "ECS/Components/BasicPlayer_C.h"
#include "ECS/Components/Prop_C.h"
#include "ECS/Components/Skeleton_C.h"
#include "ECS/Components/Skybox_C.h"
#include "ECS/Components/LightDirectional_C.h"
#include "ECS/Components/LightSpot_C.h"
#include "ECS/Components/LightPoint_C.h"
#include "ECS/Components/Reflector_C.h"

/* System Types Used */
#include "ECS\Systems\PlayerMovement_S.h"
#include "ECS\Systems\PropRendering_S.h"
#include "ECS\Systems\PropBSphere_S.h"
#include "ECS\Systems\SkeletonAnimation_S.h"
#include "ECS\Systems\Skybox_S.h"
#include "ECS\Systems\LightingDirectional_S.h"
#include "ECS\Systems\LightingSpot_S.h"
#include "ECS\Systems\LightingPoint_S.h"
#include "ECS\Systems\Reflector_S.h"


World_Module::World_Module(Engine * engine) : Engine_Module(engine) {}

void World_Module::initialize()
{
	auto & graphics = m_engine->getGraphicsModule();
	auto & lightDirSys = *graphics.getSystem<LightingDirectional_System>();
	auto & lightPointSys = *graphics.getSystem<LightingPoint_System>();
	auto & lightSpotSys = *graphics.getSystem<LightingSpot_System>();
	auto & propSys = *graphics.getSystem<PropRendering_System>();
	auto & refSys = *graphics.getSystem<Reflector_System>();
	m_constructorMap["BasicPlayer_Component"] = new BasicPlayer_Constructor();
	m_constructorMap["BoundingSphere_Component"] = new BoundingSphere_Constructor();
	m_constructorMap["LightDirectional_Component"] = new LightDirectional_Constructor(&lightDirSys.m_lightBuffer);
	m_constructorMap["LightDirectionalShadow_Component"] = new LightDirectionalShadow_Constructor(m_engine->getPreference<float>(PreferenceState::C_DRAW_DISTANCE), &lightDirSys.m_shadowBuffer, &lightDirSys.m_shadowFBO);
	m_constructorMap["LightPoint_Component"] = new LightPoint_Constructor(&lightPointSys.m_lightBuffer);
	m_constructorMap["LightPointShadow_Component"] = new LightPointShadow_Constructor(&lightPointSys.m_shadowBuffer, &lightPointSys.m_shadowFBO);
	m_constructorMap["LightSpot_Component"] = new LightSpot_Constructor(&lightSpotSys.m_lightBuffer);
	m_constructorMap["LightSpotShadow_Component"] = new LightSpotShadow_Constructor(&lightSpotSys.m_shadowBuffer, &lightSpotSys.m_shadowFBO);
	m_constructorMap["Prop_Component"] = new Prop_Constructor(m_engine, &propSys.m_propBuffer);
	m_constructorMap["Reflector_Component"] = new Reflector_Constructor(&graphics.m_cameraBuffer, &refSys.m_reflectorBuffer, &refSys.m_envmapFBO);
	m_constructorMap["Skeleton_Component"] = new Skeleton_Constructor(m_engine, &propSys.m_skeletonBuffer);
	m_constructorMap["Skybox_Component"] = new Skybox_Constructor(m_engine);
}

void World_Module::loadWorld()
{
	if (m_level) 
		m_level->removeCallback(this);	
	m_level = Asset_Level::Create(m_engine, "newTest.map");
	m_level->addCallback(this, [&]{processLevel();});	
}

void World_Module::addLevelListener(bool * notifier)
{
	m_notifyees.push_back(notifier);
	if (m_finishedLoading)
		*notifier = true;
}

void World_Module::checkIfLoaded()
{
	bool oldStatus = m_finishedLoading;
	m_finishedLoading = m_engine->getAssetManager().finishedWork() & m_engine->getModelManager().finishedWork() & m_engine->getMaterialManager().finishedWork();	
	if (!oldStatus && m_finishedLoading) {
		for each (bool * flag in m_notifyees)
			*flag = true;
	}
}

void World_Module::processLevel()
{
	ECS & ecs = m_engine->getECS();
	std::shared_lock readGuard(m_level->m_mutex); // safe to read level
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
