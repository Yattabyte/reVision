#pragma once
#ifndef PLAYERFREELOOK_S_H
#define PLAYERFREELOOK_S_H 

#include "States/GameSystemInterface.h"
#include "States/Game/ECS/Player3D_C.h"
#include "Utilities/ECS/Transform_C.h"
#include "Engine.h"
#include "glm/glm.hpp"


/** A system responsible for updating player components based on keyboard/mouse. */
class PlayerFreeLook_System : public Game_System_Interface {
public: 
	// (de)Constructors
	~PlayerFreeLook_System() = default;
	PlayerFreeLook_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Transform_Component::ID);
		addComponentType(Player3D_Component::ID);
	}


	// Interface Implementation
	virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<BaseECSComponent*> > & components) override {
		auto & graphicsModule = m_engine->getModule_Graphics();
		for each (const auto & componentParam in components) {
			Transform_Component * transformComponent = (Transform_Component*)componentParam[0];
			Player3D_Component * playerComponent = (Player3D_Component*)componentParam[1];

			auto & actionState = m_engine->getActionState();
			auto & rotation = playerComponent->m_rotation;
			auto & transform = transformComponent->m_transform;
			// Determine how much the camera should rotate
			rotation += 25.0f * deltaTime * glm::vec3(actionState.at(ActionState::LOOK_X), actionState.at(ActionState::LOOK_Y), 0);
			rotation.x = fmodf(rotation.x, 360.0f);
			if (rotation.x < 0.0f)
				rotation.x += 360.0f;
			if (rotation.y > 90.0f)
				rotation.y = 90.0f;
			else if (rotation.y < -90.0f)
				rotation.y = -90.0f;
			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(1.0f, 0, 0)) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(0, 1.0f, 0));
			transform.m_orientation = quat_cast(rotationMatrix);

			// Determine how much to move in local space
			const float velocity = 25.0f;
			const float moveAmount = velocity * deltaTime;
			glm::vec3 deltaPosition(0.0f);
			if (actionState.at(ActionState::FORWARD) > 0.5f)
				deltaPosition += glm::vec3(0, 0, -moveAmount);
			if (actionState.at(ActionState::BACKWARD) > 0.5f)
				deltaPosition += glm::vec3(0, 0, moveAmount);
			if (actionState.at(ActionState::LEFT) > 0.5f)
				deltaPosition += glm::vec3(-moveAmount, 0, 0);
			if (actionState.at(ActionState::RIGHT) > 0.5f)
				deltaPosition += glm::vec3(moveAmount, 0, 0);
			// Make the translation amount be relative to the camera's orientation
			glm::vec4 rotatedPosition = glm::inverse(rotationMatrix) * glm::vec4(deltaPosition, 1.0f);
			transform.m_position += glm::vec3(rotatedPosition / rotatedPosition.w);

			// Update the engine pointer
			auto & cameraBuffer = graphicsModule.getActiveCameraBuffer()->data;
			cameraBuffer->EyePosition = transform.m_position;
			glm::mat4 vMatrix = glm::toMat4(transform.m_orientation) * glm::translate(glm::mat4(1.0f), -transform.m_position);
			cameraBuffer->vMatrix = vMatrix;
			cameraBuffer->vMatrix_Inverse = glm::inverse(vMatrix);
		}
	};


private:
	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // PLAYERFREELOOK_S_H