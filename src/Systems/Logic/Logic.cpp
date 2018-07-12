#include "Systems\Logic\Logic.h"
#include "Engine.h"
#include "Systems\World\Camera.h"
#include "Systems\Input\ActionState.h"


System_Logic::~System_Logic()
{
}

System_Logic::System_Logic()	
{
	m_rotation = glm::vec3(0.0f);
}

void System_Logic::initialize(Engine * engine)
{
	if (!m_Initialized) {
		m_engine = engine; 
		m_Initialized = true;
	}
}

// Noclip type camera
void System_Logic::update(const float & deltaTime)
{
	auto & actionState = m_engine->getActionState();
	// Determine how much the camera should rotate
	m_rotation += 25.0f * deltaTime * glm::vec3(actionState.at(ActionState::LOOK_X), actionState.at(ActionState::LOOK_Y), 0);
	m_rotation.x = fmodf(m_rotation.x, 360.0f);
	if (m_rotation.x < 0.0f)
		m_rotation.x += 360.0f;
	if (m_rotation.y > 90.0f)
		m_rotation.y = 90.0f;
	else if (m_rotation.y < -90.0f)
		m_rotation.y = -90.0f;
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.y), glm::vec3(1.0f, 0, 0)) * glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.x), glm::vec3(0, 1.0f, 0));
	m_transform.m_orientation = quat_cast(rotationMatrix);

	// Determine how much to move in local space
	const float velocity = 50.0f;
	const float moveAmount = velocity * deltaTime;
	glm::vec3 deltaPosition(0.0f);
	if (actionState.at(ActionState::FORWARD) > 0.5f)
		deltaPosition += glm::vec3(0, 0, -moveAmount);
	if (actionState.at(ActionState::BACK) > 0.5f)
		deltaPosition += glm::vec3(0, 0, moveAmount);
	if (actionState.at(ActionState::LEFT) > 0.5f)
		deltaPosition += glm::vec3(-moveAmount, 0, 0);
	if (actionState.at(ActionState::RIGHT) > 0.5f)
		deltaPosition += glm::vec3(moveAmount, 0, 0);
	// Make the translation amount be relative to the camera's orientation
	glm::vec4 rotatedPosition = glm::inverse(rotationMatrix) * glm::vec4(deltaPosition, 1.0f);
	m_transform.m_position += glm::vec3(rotatedPosition / rotatedPosition.w);

	// Update the engine pointer
	m_engine->getCamera()->setPosition(m_transform.m_position);
	m_engine->getCamera()->setOrientation(m_transform.m_orientation);
	m_engine->getCamera()->update();
}
