#include "Modules/Game/ECS/PlayerFreeLook_System.h"
#include "Modules/ECS/component_types.h"
#include "glm/glm.hpp"
#include "Engine.h"


PlayerFreeLook_System::PlayerFreeLook_System(Engine& engine)
	: m_engine(engine) 
{
	// Declare component types used
	addComponentType(Transform_Component::Runtime_ID);
	addComponentType(Player3D_Component::Runtime_ID);

	// Error Reporting
	if (!isValid())
		engine.getManager_Messages().error("Invalid ECS System: PlayerFreeLook_System");
}

void PlayerFreeLook_System::updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components)
{
	for (const auto& componentParam : components) {
		auto* transformComponent = static_cast<Transform_Component*>(componentParam[0]);
		auto* playerComponent = static_cast<Player3D_Component*>(componentParam[1]);

		auto& actionState = m_engine.getActionState();
		auto& rotation = playerComponent->m_rotation;
		auto& transform = transformComponent->m_localTransform;
		// Determine how much the camera should rotate
		rotation += 25.0f * deltaTime * glm::vec3(actionState[ActionState::Action::LOOK_X], actionState[ActionState::Action::LOOK_Y], 0);
		rotation.x = fmodf(rotation.x, 360.0f);
		if (rotation.x < 0.0f)
			rotation.x += 360.0f;
		if (rotation.y > 90.0f)
			rotation.y = 90.0f;
		else if (rotation.y < -90.0f)
			rotation.y = -90.0f;

		// Determine how much to move in local space
		constexpr float velocity = 25.0f;
		const float moveAmount = velocity * deltaTime;
		glm::vec3 deltaPosition(0.0f);
		if (static_cast<int>(actionState.isAction(ActionState::Action::FORWARD)))
			deltaPosition += glm::vec3(0, 0, -moveAmount);
		if (static_cast<int>(actionState.isAction(ActionState::Action::BACKWARD)))
			deltaPosition += glm::vec3(0, 0, moveAmount);
		if (static_cast<int>(actionState.isAction(ActionState::Action::LEFT)))
			deltaPosition += glm::vec3(-moveAmount, 0, 0);
		if (static_cast<int>(actionState.isAction(ActionState::Action::RIGHT)))
			deltaPosition += glm::vec3(moveAmount, 0, 0);

		// Integrate rotation and translation into a new set of matrices
		auto& cam = m_engine.getModule_Graphics().getClientCamera();
		const auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(1.0f, 0, 0)) * glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(0, 1.0f, 0));
		// Make the translation amount be relative to the camera's orientation
		const auto rotatedPosition = glm::inverse(rotationMatrix) * glm::vec4(deltaPosition, 1.0f);
		transform.m_position += glm::vec3(rotatedPosition / rotatedPosition.w);
		const auto vMatrix = rotationMatrix * glm::translate(glm::mat4(1.0f), -transform.m_position);
		cam->EyePosition = transform.m_position;
		cam->vMatrix = vMatrix;
		cam->vMatrixInverse = glm::inverse(vMatrix);
		cam->pvMatrix = cam->pMatrix * vMatrix;
	}
}