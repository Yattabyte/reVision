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
	auto & ecs = m_engine->getECS();
	auto & graphics = m_engine->getGraphicsModule();
	auto * propSys = graphics.getSystem<PropRendering_System>();
	auto * lightDirSys = graphics.getSystem<LightingDirectional_System>();
	auto * lightSpotSys = graphics.getSystem<LightingSpot_System>();
	auto * lightPointSys = graphics.getSystem<LightingPoint_System>();
	auto * reflectorSys = graphics.getSystem<Reflector_System>();

	std::shared_lock readGuard(m_level->m_mutex);
	for each (auto & lvlEntity in m_level->m_entities) {
		std::vector<BaseECSComponent*> components;
		std::vector<unsigned int> ids;
		for each (const auto & lvlComponent in lvlEntity.components) {
			BaseECSComponent * newComponent = nullptr;
			unsigned int id;
			if (lvlComponent.type.find("BasicPlayer_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<glm::vec3>(0).value_or(glm::vec3(0.0f));
				auto * component = new BasicPlayer_Component();
				component->m_transform.m_position = p0;
				component->m_transform.update();
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("LightDirectional_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<glm::vec3>(0).value_or(glm::vec3(1.0f));
				auto p1 = lvlComponent.getParameter<float>(1).value_or(1.0f);
				auto p2 = lvlComponent.getParameter<glm::vec3>(2).value_or(glm::vec3(0.0f));
				auto p3 = lvlComponent.getParameter<glm::quat>(3).value_or(glm::quat(1, 0, 0, 0));
				auto p4 = lvlComponent.getParameter<glm::vec3>(4).value_or(glm::vec3(1.0f));
				auto * component = new LightDirectional_Component();
				lightDirSys->registerComponent(*component);
				component->m_data->data->LightColor = p0;
				component->m_data->data->LightIntensity = p1;
				glm::mat4 sunTransform = glm::mat4_cast(p3);
				component->m_data->data->LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
				glm::mat4 sunModelMatrix = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("LightDirectionalShadow_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<glm::vec3>(0).value_or(glm::vec3(0.0f));
				auto p1 = lvlComponent.getParameter<glm::quat>(1).value_or(glm::quat(1, 0, 0, 0));
				auto p2 = lvlComponent.getParameter<glm::vec3>(2).value_or(glm::vec3(1.0f));
				auto * component = new LightDirectionalShadow_Component();
				lightDirSys->registerComponent(*component);
				glm::mat4 sunTransform = glm::mat4_cast(p1);
				glm::mat4 sunModelMatrix = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
				component->m_mMatrix = sunModelMatrix;
				component->m_data->data->lightV = sunModelMatrix;
				newComponent = component;
				id = component->ID;			
			}
			else if (lvlComponent.type.find("LightSpot_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<glm::vec3>(0).value_or(glm::vec3(1.0f));
				auto p1 = lvlComponent.getParameter<float>(1).value_or(1.0f);
				auto p2 = lvlComponent.getParameter<float>(2).value_or(1.0f);
				auto p3 = lvlComponent.getParameter<float>(3).value_or(45.0f);
				auto p4 = lvlComponent.getParameter<glm::vec3>(4).value_or(glm::vec3(0.0f));
				auto * component = new LightSpot_Component();
				lightSpotSys->registerComponent(*component);
				component->m_data->data->LightColor = p0;
				component->m_data->data->LightIntensity = p1;
				component->m_data->data->LightRadius = p2;
				component->m_data->data->LightCutoff = cosf(glm::radians(p3));
				component->m_data->data->LightPosition = p4;
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), p4);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(p2*p2)*1.1f);
				component->m_data->data->LightDirection = glm::vec3(1, 0, 0);
				component->m_data->data->mMatrix = (trans)* scl;
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("LightSpotShadow_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<float>(0).value_or(1.0f);
				auto p1 = lvlComponent.getParameter<float>(1).value_or(45.0f);
				auto p2 = lvlComponent.getParameter<glm::vec3>(2).value_or(glm::vec3(0.0f));
				auto * component = new LightSpotShadow_Component();
				lightSpotSys->registerComponent(*component);
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), p2);
				const glm::mat4 final = glm::inverse(trans * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0)));
				const float verticalRad = 2.0f * atanf(tanf(glm::radians(p1 * 2) / 2.0f));
				const glm::mat4 perspective = glm::perspective(verticalRad, 1.0f, 0.01f, p0*p0);
				component->m_data->data->lightV = final;
				component->m_data->data->lightPV = perspective * final;
				component->m_data->data->inversePV = glm::inverse(perspective * final);
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("LightPoint_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<glm::vec3>(0).value_or(glm::vec3(1.0f));
				auto p1 = lvlComponent.getParameter<float>(1).value_or(1.0f);
				auto p2 = lvlComponent.getParameter<float>(2).value_or(1.0f);
				auto p3 = lvlComponent.getParameter<glm::vec3>(3).value_or(glm::vec3(0.0f));
				auto * component = new LightPoint_Component();
				lightPointSys->registerComponent(*component);
				component->m_data->data->LightColor = p0;
				component->m_data->data->LightIntensity = p1;
				component->m_data->data->LightRadius = p2;
				component->m_data->data->LightPosition = p3;
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), p3);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(p2*p2)*1.1f);
				component->m_data->data->mMatrix = (trans) * scl;
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("LightPointShadow_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<float>(0).value_or(1.0f);
				auto p1 = lvlComponent.getParameter<glm::vec3>(1).value_or(glm::vec3(0.0f));
				auto * component = new LightPointShadow_Component();
				lightPointSys->registerComponent(*component);
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), p1);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(p0*p0)*1.1f);
				component->m_data->data->lightV = glm::translate(glm::mat4(1.0f), -p1);
				glm::mat4 rotMats[6];
				const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, p0*p0);
				rotMats[0] = pMatrix * glm::lookAt(p1, p1 + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
				rotMats[1] = pMatrix * glm::lookAt(p1, p1 + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
				rotMats[2] = pMatrix * glm::lookAt(p1, p1 + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
				rotMats[3] = pMatrix * glm::lookAt(p1, p1 + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
				rotMats[4] = pMatrix * glm::lookAt(p1, p1 + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
				rotMats[5] = pMatrix * glm::lookAt(p1, p1 + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
				for (int x = 0; x < 6; ++x) {
					component->m_data->data->lightPV[x] = rotMats[x];
					component->m_data->data->inversePV[x] = glm::inverse(rotMats[x]);
				}
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("Prop_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<std::string>(0).value_or("");
				auto p1 = lvlComponent.getParameter<unsigned int>(1).value_or(0);
				auto p2 = lvlComponent.getParameter<glm::vec3>(2).value_or(glm::vec3(0.0f));
				auto p3 = lvlComponent.getParameter<glm::quat>(3).value_or(glm::quat(1, 0, 0, 0));
				auto p4 = lvlComponent.getParameter<glm::vec3>(4).value_or(glm::vec3(1.0f));
				auto * component = new Prop_Component();
				propSys->registerComponent(*component);
				component->m_model = Asset_Model::Create(m_engine, p0);
				component->m_transform.m_position = p2;
				component->m_transform.m_orientation = p3;
				component->m_transform.m_scale = p4;
				component->m_transform.update();
				component->m_data->data->materialID = p1;
				component->m_data->data->mMatrix = component->m_transform.m_modelMatrix;
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("BoundingSphere_Component") != std::string::npos) {
				newComponent = new BoundingSphere_Component();
				id = BoundingSphere_Component::ID;
			}
			else if (lvlComponent.type.find("Skeleton_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<std::string>(0).value_or("");
				auto p1 = lvlComponent.getParameter<int>(1).value_or(0);
				auto * component = new Skeleton_Component();
				propSys->registerComponent(*component);
				component->m_model = Asset_Model::Create(m_engine, p0);
				component->m_animation = p1;
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("Reflector_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<glm::vec3>(0).value_or(glm::vec3(0.0f));
				auto p1 = lvlComponent.getParameter<glm::quat>(1).value_or(glm::quat(1, 0, 0, 0));
				auto p2 = lvlComponent.getParameter<glm::vec3>(2).value_or(glm::vec3(1.0f));
				auto * component = new Reflector_Component();
				reflectorSys->registerComponent(*component);
				const float largest = pow(std::max(std::max(p2.x, p2.y), p2.z), 2.0f);
				component->m_transform = Transform(p0, p1, p2);
				component->m_data->data->mMatrix = component->m_transform.m_modelMatrix;
				component->m_data->data->rotMatrix = glm::inverse(glm::toMat4(component->m_transform.m_orientation));
				component->m_data->data->BoxCamPos = p0;
				component->m_data->data->BoxScale = component->m_transform.m_scale;
				component->m_Cameradata[0]->data->vMatrix = glm::lookAt(p0, p0 + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
				component->m_Cameradata[1]->data->vMatrix = glm::lookAt(p0, p0 + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
				component->m_Cameradata[2]->data->vMatrix = glm::lookAt(p0, p0 + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
				component->m_Cameradata[3]->data->vMatrix = glm::lookAt(p0, p0 + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
				component->m_Cameradata[4]->data->vMatrix = glm::lookAt(p0, p0 + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
				component->m_Cameradata[5]->data->vMatrix = glm::lookAt(p0, p0 + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
				for (int x = 0; x < 6; ++x) {
					component->m_Cameradata[x]->data->FarPlane = largest;
					component->m_Cameradata[x]->data->EyePosition = p0;
					component->m_Cameradata[x]->data->pMatrix = glm::perspective(glm::radians(component->m_Cameradata[x]->data->FOV), 1.0f, 0.01f, component->m_Cameradata[x]->data->FarPlane);
					component->m_Cameradata[x]->data->pMatrix_Inverse = glm::inverse(component->m_Cameradata[x]->data->pMatrix);
					component->m_Cameradata[x]->data->vMatrix_Inverse = glm::inverse(component->m_Cameradata[x]->data->vMatrix);
				}
				newComponent = component;
				id = component->ID;
			}
			else if (lvlComponent.type.find("Skybox_Component") != std::string::npos) {
				auto p0 = lvlComponent.getParameter<std::string>(0).value_or("");
				auto * component = new Skybox_Component();
				component->m_texture = Asset_Cubemap::Create(m_engine, p0);
				newComponent = component;
				id = component->ID;
			}
			if (newComponent) {
				components.push_back(newComponent);
				ids.push_back(id);
			}
		}
		ecs.makeEntity(components.data(), ids.data(), components.size());
		for each (auto * component in components)
			delete component;
	}
}
