#pragma once
#ifndef TRANSFORMSYNC_S_H
#define TRANSFORMSYNC_S_H
#define GLM_ENABLE_EXPERIMENTAL

#include "ECS\Systems\ecsSystem.h"
#include "Engine.h"
#include "glm\glm.hpp"
#include "glm\gtx\component_wise.hpp"

/* Component Types Used */
#include "ECS\Components\Collider_C.h"
#include "ECS\Components\Transform_C.h"
#include "ECS\Components\Prop_C.h"
#include "ECS\Components\LightDirectional_C.h"
#include "ECS\Components\LightPoint_C.h"
#include "ECS\Components\LightSpot_C.h"
#include "ECS\Components\Reflector_C.h"


/** A system responsible for updating components that share a common transformation. */
class TransformSync_System : public BaseECSSystem {
public:
	// (de)Constructors
	~TransformSync_System() = default;
	TransformSync_System(Engine * engine, btDiscreteDynamicsWorld * world) 
		: BaseECSSystem(), m_engine(engine), m_world(world) 
	{
		// Declare component types used
		addComponentType(Transform_Component::ID);
		addComponentType(Collider_Component::ID, FLAG_OPTIONAL);
		addComponentType(Prop_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightDirectional_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightDirectionalShadow_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightPoint_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightPointShadow_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightSpot_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightSpotShadow_Component::ID, FLAG_OPTIONAL);
		addComponentType(Reflector_Component::ID, FLAG_OPTIONAL);
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		for each (const auto & componentParam in components) {
			auto * transformComponent = (Transform_Component*)componentParam[0];
			auto * colliderComponent = (Collider_Component*)componentParam[1];
			auto * propComponent = (Prop_Component*)componentParam[2];
			auto * directionalLightComponent = (LightDirectional_Component*)componentParam[3];
			auto * directionalShadowComponent = (LightDirectionalShadow_Component*)componentParam[4];
			auto * pointLightComponent = (LightPoint_Component*)componentParam[5];
			auto * pointShadowComponent = (LightPointShadow_Component*)componentParam[6];
			auto * spotLightComponent = (LightSpot_Component*)componentParam[7];
			auto * spotShadowComponent = (LightSpotShadow_Component*)componentParam[8];
			auto * reflectorComponent = (Reflector_Component*)componentParam[9];

			const auto & position = transformComponent->m_transform.m_position;
			const auto & orientation = transformComponent->m_transform.m_orientation;
			const auto & scale = transformComponent->m_transform.m_scale;
			const auto & modelMatrix = transformComponent->m_transform.m_modelMatrix;

			if (colliderComponent) {
				if (colliderComponent->m_collider->existsYet()) {
					// If the collider's transformation is out of date
					if (colliderComponent->m_transform != transformComponent->m_transform) {
						// Update the transform
						colliderComponent->m_transform = transformComponent->m_transform;

						// Remove from the physics simulation
						if (colliderComponent->m_rigidBody) {
							m_world->removeRigidBody(colliderComponent->m_rigidBody);
							delete colliderComponent->m_rigidBody;
						}

						// Build the collider from the transform info
						if (!colliderComponent->m_motionState)
							colliderComponent->m_motionState = new btDefaultMotionState();
						colliderComponent->m_motionState->setWorldTransform(btTransform(btTransform(btQuaternion(orientation.x, orientation.y, orientation.z, orientation.w), btVector3(position.x, position.y, position.z))));

						// This line won't work if the shape is reused!!
						colliderComponent->m_collider->m_shape->setLocalScaling(btVector3(scale.x, scale.y, scale.z));

						btVector3 Inertia(0, 0, 0);
						colliderComponent->m_collider->m_shape->calculateLocalInertia(colliderComponent->m_mass, Inertia);
						auto bodyCI = btRigidBody::btRigidBodyConstructionInfo(colliderComponent->m_mass, colliderComponent->m_motionState, colliderComponent->m_collider->m_shape, Inertia);
						bodyCI.m_restitution = colliderComponent->m_restitution;
						bodyCI.m_friction = colliderComponent->m_friction;
						colliderComponent->m_rigidBody = new btRigidBody(bodyCI);
						m_world->addRigidBody(colliderComponent->m_rigidBody);
					}
					
					// Otherwise update the transform with the collider info
					else {
						btTransform trans;
						colliderComponent->m_motionState->getWorldTransform(trans);
						const btQuaternion quat = trans.getRotation();
						const btVector3 pos = trans.getOrigin();
						transformComponent->m_transform.m_position = glm::vec3(pos.x(), pos.y(), pos.z());
						transformComponent->m_transform.m_orientation = glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
						transformComponent->m_transform.update();
						colliderComponent->m_transform = transformComponent->m_transform;
					}
				}
			}
			if (propComponent) {
				if (propComponent->m_model->existsYet()) {
					propComponent->m_data->data->mMatrix = modelMatrix;

					// Update bounding sphere
					const glm::vec3 bboxMax_World = (propComponent->m_model->m_bboxMax * scale) + position;
					const glm::vec3 bboxMin_World = (propComponent->m_model->m_bboxMin * scale) + position;
					const glm::vec3 bboxCenter = (bboxMax_World + bboxMin_World) / 2.0f;
					const glm::vec3 bboxScale = (bboxMax_World - bboxMin_World) / 2.0f;
					glm::mat4 matTrans = glm::translate(glm::mat4(1.0f), bboxCenter);
					glm::mat4 matRot = glm::mat4_cast(orientation);
					glm::mat4 matScale = glm::scale(glm::mat4(1.0f), bboxScale);
					glm::mat4 matFinal = (matTrans * matRot * matScale);
					propComponent->m_data->data->bBoxMatrix = matFinal;
					propComponent->m_radius = glm::compMax(propComponent->m_model->m_radius * scale);
					propComponent->m_position = propComponent->m_model->m_bboxCenter + position;
				}
			}
			if (directionalLightComponent) {
				const glm::mat4 sunTransform = glm::mat4_cast(orientation);
				directionalLightComponent->m_data->data->LightDirection = glm::vec3(glm::normalize(sunTransform * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
			}
			if (directionalShadowComponent) {
				if (directionalShadowComponent->m_orientation != orientation) {
					const glm::mat4 sunTransform = glm::mat4_cast(orientation);
					const glm::mat4 sunModelMatrix = glm::inverse(sunTransform * glm::mat4_cast(glm::rotate(glm::quat(1, 0, 0, 0), glm::radians(90.0f), glm::vec3(0, 1.0f, 0))));
					directionalShadowComponent->m_mMatrix = sunModelMatrix;
					directionalShadowComponent->m_data->data->lightV = sunModelMatrix;
					directionalShadowComponent->m_orientation = orientation;
				}
			}
			if (pointLightComponent) {
				pointLightComponent->m_data->data->LightPosition = position;
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(pointLightComponent->m_radius*pointLightComponent->m_radius)*1.1f);
				pointLightComponent->m_data->data->mMatrix = (trans)* scl;
			}
			if (pointShadowComponent) {
				if (pointShadowComponent->m_position != position) {
					pointShadowComponent->m_position = position;
					pointShadowComponent->m_data->data->lightV = glm::translate(glm::mat4(1.0f), -position);
					glm::mat4 rotMats[6];
					const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, pointShadowComponent->m_radius * pointShadowComponent->m_radius);
					rotMats[0] = pMatrix * glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
					rotMats[1] = pMatrix * glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
					rotMats[2] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
					rotMats[3] = pMatrix * glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
					rotMats[4] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
					rotMats[5] = pMatrix * glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
					for (int x = 0; x < 6; ++x) {
						pointShadowComponent->m_data->data->lightPV[x] = rotMats[x];
						pointShadowComponent->m_data->data->inversePV[x] = glm::inverse(rotMats[x]);
					}
				}
			}
			if (spotLightComponent) {
				spotLightComponent->m_data->data->LightPosition = position;
				spotLightComponent->m_data->data->LightDirection = glm::vec3(1, 0, 0);
				const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
				const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(spotLightComponent->m_radius*spotLightComponent->m_radius)*1.1f);
				spotLightComponent->m_data->data->mMatrix = (trans)* scl;
			}
			if (reflectorComponent) {
				const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
				reflectorComponent->m_transform = Transform(position, orientation, scale);
				reflectorComponent->m_data->data->mMatrix = modelMatrix;
				reflectorComponent->m_data->data->rotMatrix = glm::inverse(glm::toMat4(orientation));
				reflectorComponent->m_data->data->BoxCamPos = position;
				reflectorComponent->m_data->data->BoxScale = scale;
				const glm::mat4 vMatrices[6] = {
					glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
					glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
					glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
					glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
				};
				const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, largest);
				const glm::mat4 pMatrix_Inverse = glm::inverse(pMatrix);
				for (int x = 0; x < 6; ++x) {
					reflectorComponent->m_Cameradata[x]->data->FarPlane = largest;
					reflectorComponent->m_Cameradata[x]->data->EyePosition = position;
					reflectorComponent->m_Cameradata[x]->data->pMatrix = pMatrix;
					reflectorComponent->m_Cameradata[x]->data->pMatrix_Inverse = pMatrix_Inverse;
					reflectorComponent->m_Cameradata[x]->data->vMatrix = vMatrices[x];
					reflectorComponent->m_Cameradata[x]->data->vMatrix_Inverse = glm::inverse(vMatrices[x]);
				}
			}
		}
	};


private:
	// Private Attributes
	Engine * m_engine = nullptr;
	btDiscreteDynamicsWorld * m_world = nullptr;
};

#endif // TRANSFORMSYNC_S_H