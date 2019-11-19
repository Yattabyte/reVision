#include "Modules/Editor/UI/CameraController.h"
#include "Modules/Editor/Editor_M.h"
#include "imgui.h"
#include "Engine.h"
#include "glm/glm.hpp"
#include "glm/matrix.hpp"


CameraController::CameraController(Engine* engine) noexcept :
	m_engine(engine)
{
	m_open = true;
}

void CameraController::tick(const float& deltaTime) noexcept
{
	// All camera input, including both rotation and translation only when mouse isn't captured by any windows
	if (!ImGui::GetIO().WantCaptureMouse && ImGui::IsMouseDown(1)) {
		auto& actionState = m_engine->getActionState();

		// Save beginning mouse click position
		if (!m_beginPress) {
			m_beginPress = true;
			m_startPos = glm::vec2(actionState[ActionState::Action::MOUSE_X], actionState[ActionState::Action::MOUSE_Y]);
		}
		// Calculate delta rotation
		else if (m_beginPress && ImGui::IsMouseDragging(1)) {
			const auto currentPos = glm::vec2(actionState[ActionState::Action::MOUSE_X], actionState[ActionState::Action::MOUSE_Y]);
			const auto delta = currentPos - m_startPos;
			m_rotation = glm::vec2(delta + m_lastRotation);
			m_rotation.x = fmodf(m_rotation.x, 360.0f);
			if (m_rotation.x < 0.0f)
				m_rotation.x += 360.0f;
			if (m_rotation.y > 90.0f)
				m_rotation.y = 90.0f;
			else if (m_rotation.y < -90.0f)
				m_rotation.y = -90.0f;
		}

		// Determine how much to move in local space
		const float velocity = 25.0f;
		const float moveAmount = velocity * deltaTime;
		glm::vec3 deltaPosition(0.0f);
		if ((int)actionState.isAction(ActionState::Action::FORWARD))
			deltaPosition += glm::vec3(0, 0, -moveAmount);
		if ((int)actionState.isAction(ActionState::Action::BACKWARD))
			deltaPosition += glm::vec3(0, 0, moveAmount);
		if ((int)actionState.isAction(ActionState::Action::LEFT))
			deltaPosition += glm::vec3(-moveAmount, 0, 0);
		if ((int)actionState.isAction(ActionState::Action::RIGHT))
			deltaPosition += glm::vec3(moveAmount, 0, 0);

		// Integrate rotation and translation into a new set of matrices
		auto cam = m_engine->getModule_Graphics().getClientCamera();
		const auto rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.y), glm::vec3(1.0f, 0, 0)) * glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.x), glm::vec3(0, 1.0f, 0));
		// Make the translation amount be relative to the camera's orientation
		const auto rotatedPosition = glm::inverse(rotationMatrix) * glm::vec4(deltaPosition, 1.0f);
		auto& eyePosition = cam->get()->EyePosition;
		eyePosition += glm::vec3(rotatedPosition / rotatedPosition.w);
		const auto vMatrix = rotationMatrix * glm::translate(glm::mat4(1.0f), -eyePosition);
		cam->get()->vMatrix = vMatrix;
		cam->get()->vMatrixInverse = glm::inverse(vMatrix);
		cam->get()->pvMatrix = cam->get()->pMatrix * vMatrix;
	}
	// Save last rotation state
	else {
		m_beginPress = false;
		m_lastRotation = m_rotation;
	}
}