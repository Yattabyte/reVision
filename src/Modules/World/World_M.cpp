#include "Modules\World\World_M.h"
#include "Modules\Graphics\Graphics_M.h"
#include "Engine.h"
#include "glm\gtc\matrix_transform.hpp"

/* Component Types Used */
#include "ECS/Components/BasicPlayer_C.h"
#include "ECS/Components/Transform_C.h"
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


World_Module::~World_Module()
{
}

World_Module::World_Module(Engine * engine) : Engine_Module(engine)
{
	m_finishedLoading = false;
}

void World_Module::loadWorld()
{
	auto & ecs = m_engine->getECS();
	auto & graphics = m_engine->getGraphicsModule();
	auto * propSys = graphics.getSystem<PropRendering_System>();
	auto * lightDirSys = graphics.getSystem<LightingDirectional_System>();
	auto * lightSpotSys = graphics.getSystem<LightingSpot_System>();
	auto * lightPointSys = graphics.getSystem<LightingPoint_System>();
	auto * reflectorSys = graphics.getSystem<Reflector_System>();

	{
		auto eHandle = ecs.makeEntity(BasicPlayer_Component());
		BasicPlayer_Component * player = ecs.getComponent<BasicPlayer_Component>(eHandle);
		player->m_transform.m_position = glm::vec3(0, 10, 30);
		//player->m_rotation.y = 45.0f;
		player->m_transform.update();
	}
	{
		auto eHandle = ecs.makeEntity(LightDirectional_Component(), LightDirectionalShadow_Component());
		LightDirectional_Component * sun = ecs.getComponent<LightDirectional_Component>(eHandle);
		LightDirectionalShadow_Component * sunShadow = ecs.getComponent<LightDirectionalShadow_Component>(eHandle);
		lightDirSys->registerComponent(*sun);
		lightDirSys->registerComponent(*sunShadow);
		sun->m_data->data->LightColor = glm::vec3(0.75, 0.75, 0.9);
		sun->m_data->data->LightIntensity = 8;
		glm::mat4 sunTransform = glm::mat4_cast(glm::quat(0.0828278884, -0.373612523, 0.901980519, 0.199964225));
		sun->m_data->data->LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
		glm::mat4 sunModelMatrix = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
		sunShadow->m_mMatrix = sunModelMatrix;
		sunShadow->m_data->data->lightV = sunModelMatrix;
	}
	/*{
		auto eHandle = ecs.makeEntity(LightDirectional_Component(), LightDirectionalShadow_Component());
		LightDirectional_Component * sun = ecs.getComponent<LightDirectional_Component>(eHandle);
		LightDirectionalShadow_Component * sunShadow = ecs.getComponent<LightDirectionalShadow_Component>(eHandle);
		lightDirSys->registerComponent(*sun);
		lightDirSys->registerComponent(*sunShadow);
		sun->m_data->data->LightColor = glm::vec3(0.9, 0.75, 0.75);
		sun->m_data->data->LightIntensity = 8;
		glm::mat4 sunTransform = glm::mat4_cast(glm::rotate(glm::quat(0.0828278884, -0.373612523, 0.901980519, 0.199964225), glm::radians(90.0f), glm::vec3(0, 1, 0)));
		sun->m_data->data->LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
		glm::mat4 sunModelMatrix = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
		sunShadow->m_mMatrix = sunModelMatrix;
		sunShadow->m_data->data->lightV = sunModelMatrix;
	}*/
	{
		auto eHandle = ecs.makeEntity(LightSpot_Component(), LightSpotShadow_Component());
		LightSpot_Component * light = ecs.getComponent<LightSpot_Component>(eHandle);
		LightSpotShadow_Component * shadow = ecs.getComponent<LightSpotShadow_Component>(eHandle);
		lightSpotSys->registerComponent(*light);
		lightSpotSys->registerComponent(*shadow);
		light->m_data->data->LightColor = glm::vec3(0, 0, 10);
		light->m_data->data->LightIntensity = 5.0f;
		light->m_data->data->LightRadius = 5.0;
		light->m_data->data->LightPosition = glm::vec3(5, 5, 10);
		const glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(5, 5, 10));
		const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(5 * 5)*1.1f);
		const glm::mat4 final = glm::inverse(trans * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0)));
		const float verticalRad = 2.0f * atanf(tanf(glm::radians(35.0f * 2) / 2.0f));
		const glm::mat4 perspective = glm::perspective(verticalRad, 1.0f, 0.01f, 5.0f * 5.0f);
		light->m_data->data->LightDirection = glm::vec3(1, 0, 0);
		light->m_data->data->mMatrix = (trans)* scl;
		light->m_data->data->LightCutoff = cosf(glm::radians(35.0f));
		shadow->m_data->data->lightV = final;
		shadow->m_data->data->lightPV = perspective * final;
		shadow->m_data->data->inversePV = glm::inverse(perspective * final);
	}
	{
		auto eHandle = ecs.makeEntity(LightSpot_Component(), LightSpotShadow_Component());
		LightSpot_Component * light = ecs.getComponent<LightSpot_Component>(eHandle);
		LightSpotShadow_Component * shadow = ecs.getComponent<LightSpotShadow_Component>(eHandle);
		lightSpotSys->registerComponent(*light);
		lightSpotSys->registerComponent(*shadow);
		light->m_data->data->LightColor = glm::vec3(0, 10, 0);
		light->m_data->data->LightIntensity = 1.0f;
		light->m_data->data->LightRadius = 5.0;
		light->m_data->data->LightPosition = glm::vec3(5, 5, -10);
		const glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(5, 5, -10));
		const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(5 * 5)*1.1f);
		const glm::mat4 final = glm::inverse(trans * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0)));
		const float verticalRad = 2.0f * atanf(tanf(glm::radians(35.0f * 2) / 2.0f));
		const glm::mat4 perspective = glm::perspective(verticalRad, 1.0f, 0.01f, 5.0f * 5.0f);
		light->m_data->data->LightDirection = glm::vec3(1, 0, 0);
		light->m_data->data->mMatrix = (trans)* scl;
		light->m_data->data->LightCutoff = cosf(glm::radians(35.0f));
		shadow->m_data->data->lightV = final;
		shadow->m_data->data->lightPV = perspective * final;
		shadow->m_data->data->inversePV = glm::inverse(perspective * final);
	}
	{
		auto eHandle = ecs.makeEntity(LightPoint_Component(), LightPointShadow_Component());
		LightPoint_Component * light = ecs.getComponent<LightPoint_Component>(eHandle);
		LightPointShadow_Component * shadow = ecs.getComponent<LightPointShadow_Component>(eHandle);
		lightPointSys->registerComponent(*light);
		lightPointSys->registerComponent(*shadow);
		light->m_data->data->LightColor = glm::vec3(0, 0, 10);
		light->m_data->data->LightIntensity = 5.0f;
		light->m_data->data->LightRadius = 5.0;
		const glm::vec3 position = glm::vec3(-5, 5, 10);
		light->m_data->data->LightPosition = position;
		const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(5 * 5)*1.1f);
		light->m_data->data->mMatrix = (trans)* scl;
		shadow->m_data->data->lightV = glm::translate(glm::mat4(1.0f), -position);
		glm::mat4 rotMats[6];
		const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 5.0F * 5.0f);
		rotMats[0] = pMatrix * glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		rotMats[1] = pMatrix * glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		rotMats[2] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		rotMats[3] = pMatrix * glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		rotMats[4] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		rotMats[5] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		for (int x = 0; x < 6; ++x) {
			shadow->m_data->data->lightPV[x] = rotMats[x];
			shadow->m_data->data->inversePV[x] = glm::inverse(rotMats[x]);
		}
	}
	{
		auto eHandle = ecs.makeEntity(LightPoint_Component(), LightPointShadow_Component());
		LightPoint_Component * light = ecs.getComponent<LightPoint_Component>(eHandle);
		LightPointShadow_Component * shadow = ecs.getComponent<LightPointShadow_Component>(eHandle);
		lightPointSys->registerComponent(*light);
		lightPointSys->registerComponent(*shadow);
		light->m_data->data->LightColor = glm::vec3(0, 10, 0);
		light->m_data->data->LightIntensity = 1.0f;
		light->m_data->data->LightRadius = 5.0;
		const glm::vec3 position = glm::vec3(-5, 5, -10);
		light->m_data->data->LightPosition = position;
		const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(5 * 5)*1.1f);
		light->m_data->data->mMatrix = (trans)* scl;
		shadow->m_data->data->lightV = glm::translate(glm::mat4(1.0f), -position);
		glm::mat4 rotMats[6];
		const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 5.0F * 5.0f);
		rotMats[0] = pMatrix * glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		rotMats[1] = pMatrix * glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		rotMats[2] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		rotMats[3] = pMatrix * glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		rotMats[4] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		rotMats[5] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		for (int x = 0; x < 6; ++x) {
			shadow->m_data->data->lightPV[x] = rotMats[x];
			shadow->m_data->data->inversePV[x] = glm::inverse(rotMats[x]);
		}
	}
	{
		auto eHandle = ecs.makeEntity(Reflector_Component(), Transform_Component());
		Reflector_Component & reflector = *ecs.getComponent<Reflector_Component>(eHandle);
		Transform_Component & transform = *ecs.getComponent<Transform_Component>(eHandle);
		reflectorSys->registerComponent(reflector);
		const glm::vec3 position = glm::vec3(0, 15, 0);
		const glm::vec3 scale = glm::vec3(21.0F);
		const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
		transform.m_transform = Transform(position, glm::quat(1, 0, 0, 0), glm::vec3(21));
		reflector.m_data->data->mMatrix = transform.m_transform.m_modelMatrix;
		reflector.m_data->data->rotMatrix = glm::inverse(glm::toMat4(transform.m_transform.m_orientation));
		reflector.m_data->data->BoxCamPos = position;
		reflector.m_data->data->BoxScale = transform.m_transform.m_scale;
		reflector.m_Cameradata[0]->data->vMatrix = glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		reflector.m_Cameradata[1]->data->vMatrix = glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		reflector.m_Cameradata[2]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		reflector.m_Cameradata[3]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		reflector.m_Cameradata[4]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		reflector.m_Cameradata[5]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		for (int x = 0; x < 6; ++x) {
			reflector.m_Cameradata[x]->data->FarPlane = largest;
			reflector.m_Cameradata[x]->data->EyePosition = position;
			reflector.m_Cameradata[x]->data->pMatrix = glm::perspective(glm::radians(reflector.m_Cameradata[x]->data->FOV), 1.0f, 0.01f, reflector.m_Cameradata[x]->data->FarPlane);
			reflector.m_Cameradata[x]->data->pMatrix_Inverse = glm::inverse(reflector.m_Cameradata[x]->data->pMatrix);
			reflector.m_Cameradata[x]->data->vMatrix_Inverse = glm::inverse(reflector.m_Cameradata[x]->data->vMatrix);
		}		
	}
	{
		auto eHandle = ecs.makeEntity(Reflector_Component(), Transform_Component());
		Reflector_Component & reflector = *ecs.getComponent<Reflector_Component>(eHandle);
		Transform_Component & transform = *ecs.getComponent<Transform_Component>(eHandle);
		reflectorSys->registerComponent(reflector);
		const glm::vec3 position = glm::vec3(44, 15, 0);
		const glm::vec3 scale = glm::vec3(21.0F);
		const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
		transform.m_transform = Transform(position, glm::quat(1, 0, 0, 0), glm::vec3(21));
		reflector.m_data->data->mMatrix = transform.m_transform.m_modelMatrix;
		reflector.m_data->data->rotMatrix = glm::inverse(glm::toMat4(transform.m_transform.m_orientation));
		reflector.m_data->data->BoxCamPos = position;
		reflector.m_data->data->BoxScale = transform.m_transform.m_scale;
		reflector.m_Cameradata[0]->data->vMatrix = glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		reflector.m_Cameradata[1]->data->vMatrix = glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		reflector.m_Cameradata[2]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		reflector.m_Cameradata[3]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		reflector.m_Cameradata[4]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		reflector.m_Cameradata[5]->data->vMatrix = glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
		for (int x = 0; x < 6; ++x) {
			reflector.m_Cameradata[x]->data->FarPlane = largest;
			reflector.m_Cameradata[x]->data->EyePosition = position;
			reflector.m_Cameradata[x]->data->pMatrix = glm::perspective(glm::radians(reflector.m_Cameradata[x]->data->FOV), 1.0f, 0.01f, reflector.m_Cameradata[x]->data->FarPlane);
			reflector.m_Cameradata[x]->data->pMatrix_Inverse = glm::inverse(reflector.m_Cameradata[x]->data->pMatrix);
			reflector.m_Cameradata[x]->data->vMatrix_Inverse = glm::inverse(reflector.m_Cameradata[x]->data->vMatrix);
		}
	}
	{
		auto eHandle = ecs.makeEntity(Skybox_Component());
		Skybox_Component * skybox = ecs.getComponent<Skybox_Component>(eHandle);
		skybox->m_texture = Asset_Cubemap::Create(m_engine, "sky\\");
	}
	{
		auto eHandle2 = ecs.makeEntity(Prop_Component(), Transform_Component(), BoundingSphere_Component());
		Prop_Component * model2 = ecs.getComponent<Prop_Component>(eHandle2);
		Transform_Component * transform2 = ecs.getComponent<Transform_Component>(eHandle2);
		BoundingSphere_Component * bsphere2 = ecs.getComponent<BoundingSphere_Component>(eHandle2);
		propSys->registerComponent(*model2);
		model2->m_model = Asset_Model::Create(m_engine, "Test\\hills.obj");
		transform2->m_transform.m_position = glm::vec3(0, -7.5, 10);
		transform2->m_transform.m_scale = glm::vec3(30.0f);
		transform2->m_transform.update();
		model2->m_data->data->materialID = 0;
		model2->m_data->data->mMatrix = transform2->m_transform.m_modelMatrix;
		model2->m_model->addCallback(model2, [&ecs, eHandle2]() {
			auto * component = ecs.getComponent<Prop_Component>(eHandle2);
			component->m_data->data->materialID = component->m_model->getSkinID(0);
		});
	}
	{
		auto eHandle = ecs.makeEntity(Prop_Component(), Transform_Component(), BoundingSphere_Component());
		Prop_Component * model = ecs.getComponent<Prop_Component>(eHandle);
		Transform_Component * transform = ecs.getComponent<Transform_Component>(eHandle);
		BoundingSphere_Component * bsphere = ecs.getComponent<BoundingSphere_Component>(eHandle);
		propSys->registerComponent(*model);
		model->m_model = Asset_Model::Create(m_engine, "Test\\wall.obj");
		transform->m_transform.m_position = glm::vec3(-22, -10, 0);
		transform->m_transform.m_scale = glm::vec3(2.0f);
		transform->m_transform.update();
		model->m_data->data->materialID = 0;
		model->m_data->data->mMatrix = transform->m_transform.m_modelMatrix;
		model->m_model->addCallback(model, [&ecs, eHandle]() {
			auto * component = ecs.getComponent<Prop_Component>(eHandle);
			component->m_data->data->materialID = component->m_model->getSkinID(0);
		});
	}
	{
		auto eHandle = ecs.makeEntity(Prop_Component(), Transform_Component(), BoundingSphere_Component());
		Prop_Component * model = ecs.getComponent<Prop_Component>(eHandle);
		Transform_Component * transform = ecs.getComponent<Transform_Component>(eHandle);
		BoundingSphere_Component * bsphere = ecs.getComponent<BoundingSphere_Component>(eHandle);
		propSys->registerComponent(*model);
		model->m_model = Asset_Model::Create(m_engine, "Test\\wall.obj");
		transform->m_transform.m_position = glm::vec3(22, -10, 0);
		transform->m_transform.m_scale = glm::vec3(2.0f);
		transform->m_transform.update();
		model->m_data->data->materialID = 0;
		model->m_data->data->mMatrix = transform->m_transform.m_modelMatrix;
		model->m_model->addCallback(model, [&ecs, eHandle]() {
			auto * component = ecs.getComponent<Prop_Component>(eHandle);
			component->m_data->data->materialID = component->m_model->getSkinID(2);
		});
	}
	{
		auto eHandle = ecs.makeEntity(Prop_Component(), Transform_Component(), BoundingSphere_Component(), Skeleton_Component());
		Prop_Component * model = ecs.getComponent<Prop_Component>(eHandle);
		Transform_Component * transform = ecs.getComponent<Transform_Component>(eHandle);
		BoundingSphere_Component * bsphere = ecs.getComponent<BoundingSphere_Component>(eHandle);
		Skeleton_Component * anim = ecs.getComponent<Skeleton_Component>(eHandle);
		propSys->registerComponent(*model);
		propSys->registerComponent(*anim);
		model->m_model = Asset_Model::Create(m_engine, "Test\\AnimationTest.fbx");
		transform->m_transform.m_position = glm::vec3(18, 0, 10);
		transform->m_transform.m_scale = glm::vec3(1.0f);
		transform->m_transform.update();
		model->m_data->data->materialID = 0;
		model->m_data->data->mMatrix = transform->m_transform.m_modelMatrix;
		anim->m_animation = 0;
		model->m_model->addCallback(model, [&ecs, eHandle]() {
			auto * prop = ecs.getComponent<Prop_Component>(eHandle);
			prop->m_data->data->materialID = prop->m_model->getSkinID(1);
			auto * skeleton = ecs.getComponent<Skeleton_Component>(eHandle);
			skeleton->m_transforms = prop->m_model->m_boneTransforms;
		});
	}
	{
		auto eHandle = ecs.makeEntity(Prop_Component(), Transform_Component(), BoundingSphere_Component(), Skeleton_Component());
		Prop_Component * model = ecs.getComponent<Prop_Component>(eHandle);
		Transform_Component * transform = ecs.getComponent<Transform_Component>(eHandle);
		BoundingSphere_Component * bsphere = ecs.getComponent<BoundingSphere_Component>(eHandle);
		Skeleton_Component * anim = ecs.getComponent<Skeleton_Component>(eHandle);
		propSys->registerComponent(*model);
		propSys->registerComponent(*anim);
		model->m_model = Asset_Model::Create(m_engine, "Test\\AnimationTest.fbx");
		transform->m_transform.m_position = glm::vec3(18, 0, -10);
		transform->m_transform.m_scale = glm::vec3(1.0f);
		transform->m_transform.update();
		model->m_data->data->materialID = 0;
		model->m_data->data->mMatrix = transform->m_transform.m_modelMatrix;
		anim->m_animation = 1;
		model->m_model->addCallback(model, [&ecs, eHandle]() {
			auto * prop = ecs.getComponent<Prop_Component>(eHandle);
			prop->m_data->data->materialID = prop->m_model->getSkinID(1);
			auto * skeleton = ecs.getComponent<Skeleton_Component>(eHandle);
			skeleton->m_transforms = prop->m_model->m_boneTransforms;
		});
	}
	{
		auto eHandle = ecs.makeEntity(Prop_Component(), Transform_Component(), BoundingSphere_Component(), Skeleton_Component());
		Prop_Component * model = ecs.getComponent<Prop_Component>(eHandle);
		Transform_Component * transform = ecs.getComponent<Transform_Component>(eHandle);
		BoundingSphere_Component * bsphere = ecs.getComponent<BoundingSphere_Component>(eHandle);
		Skeleton_Component * anim = ecs.getComponent<Skeleton_Component>(eHandle);
		propSys->registerComponent(*model);
		propSys->registerComponent(*anim);
		model->m_model = Asset_Model::Create(m_engine, "Test\\AnimationTest.fbx");
		transform->m_transform.m_position = glm::vec3(-18, 0, 10);
		transform->m_transform.m_scale = glm::vec3(1.0f);
		transform->m_transform.update();
		model->m_data->data->materialID = 0;
		model->m_data->data->mMatrix = transform->m_transform.m_modelMatrix;
		anim->m_animation = 2;
		model->m_model->addCallback(model, [&ecs, eHandle]() {
			auto * prop = ecs.getComponent<Prop_Component>(eHandle);
			prop->m_data->data->materialID = prop->m_model->getSkinID(1);
			auto * skeleton = ecs.getComponent<Skeleton_Component>(eHandle);
			skeleton->m_transforms = prop->m_model->m_boneTransforms;
		});
	}
	{
		auto eHandle = ecs.makeEntity(Prop_Component(), Transform_Component(), BoundingSphere_Component(), Skeleton_Component());
		Prop_Component * model = ecs.getComponent<Prop_Component>(eHandle);
		Transform_Component * transform = ecs.getComponent<Transform_Component>(eHandle);
		BoundingSphere_Component * bsphere = ecs.getComponent<BoundingSphere_Component>(eHandle);
		Skeleton_Component * anim = ecs.getComponent<Skeleton_Component>(eHandle);
		propSys->registerComponent(*model);
		propSys->registerComponent(*anim);
		model->m_model = Asset_Model::Create(m_engine, "Test\\AnimationTest.fbx");
		transform->m_transform.m_position = glm::vec3(-18, 0, -10);
		transform->m_transform.m_scale = glm::vec3(1.0f);
		transform->m_transform.update();
		model->m_data->data->materialID = 0;
		model->m_data->data->mMatrix = transform->m_transform.m_modelMatrix;
		anim->m_animation = 3;
		model->m_model->addCallback(model, [&ecs, eHandle]() {
			auto * prop = ecs.getComponent<Prop_Component>(eHandle);
			prop->m_data->data->materialID = prop->m_model->getSkinID(1);
			auto * skeleton = ecs.getComponent<Skeleton_Component>(eHandle);
			skeleton->m_transforms = prop->m_model->m_boneTransforms;
		});
	}
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
