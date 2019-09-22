#pragma once
#ifndef PLAYERFREELOOK_SYSTEM_H
#define PLAYERFREELOOK_SYSTEM_H 

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Engine.h"
#include "glm/glm.hpp"


/** A system responsible for updating player components based on keyboard/mouse. */
class PlayerFreeLook_System : public ecsBaseSystem {
public: 
	// Public (de)Constructors
	/** Destroy this free-look system. */
	inline ~PlayerFreeLook_System() = default;
	/** Construct a free-look system. */
	inline PlayerFreeLook_System(Engine * engine) : m_engine(engine) {
		// Declare component types used
		addComponentType(Transform_Component::m_ID);
		addComponentType(Player3D_Component::m_ID);

		// Error Reporting
		if (!isValid())
			engine->getManager_Messages().error("Invalid ECS System: PlayerFreeLook_System");
	}


	// Public Interface Implementation
	inline virtual void updateComponents(const float & deltaTime, const std::vector< std::vector<ecsBaseComponent*> > & components) override {
		auto & graphicsModule = m_engine->getModule_Graphics();
		for each (const auto & componentParam in components) {
			Transform_Component * transformComponent = (Transform_Component*)componentParam[0];
			Player3D_Component * playerComponent = (Player3D_Component*)componentParam[1];

			auto & actionState = m_engine->getActionState();
			auto & rotation = playerComponent->m_rotation;
			auto & transform = transformComponent->m_worldTransform;
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
			if (actionState.isAction(ActionState::FORWARD))
				deltaPosition += glm::vec3(0, 0, -moveAmount);
			if (actionState.isAction(ActionState::BACKWARD))
				deltaPosition += glm::vec3(0, 0, moveAmount);
			if (actionState.isAction(ActionState::LEFT))
				deltaPosition += glm::vec3(-moveAmount, 0, 0);
			if (actionState.isAction(ActionState::RIGHT))
				deltaPosition += glm::vec3(moveAmount, 0, 0);
			// Make the translation amount be relative to the camera's orientation
			glm::vec4 rotatedPosition = glm::inverse(rotationMatrix) * glm::vec4(deltaPosition, 1.0f);
			transform.m_position += glm::vec3(rotatedPosition / rotatedPosition.w);
			transform.update();

			// Update the client's camera
			auto cam = graphicsModule.getClientCamera();
			cam->get()->EyePosition = transform.m_position;
			cam->get()->vMatrix = glm::toMat4(transform.m_orientation) * glm::translate(glm::mat4(1.0f), -transform.m_position);
			cam->get()->vMatrixInverse = glm::inverse(cam->get()->vMatrix);
			cam->get()->pvMatrix = cam->get()->pMatrix * cam->get()->vMatrix;
		}
	};


private:
	// Private Attributes
	Engine * m_engine = nullptr;
};

#endif // PLAYERFREELOOK_SYSTEM_H