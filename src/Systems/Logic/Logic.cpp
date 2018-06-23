#include "Systems\Logic\Logic.h"
#include "Engine.h"
#include "Systems\World\Camera.h"
#include "Systems\Input\ActionState.h"


System_Logic::~System_Logic()
{
}

System_Logic::System_Logic()	
{
	m_rotation = vec3(0.0f);
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
	// Determine how much the camera should rotate
	m_rotation += 25.0f * deltaTime * vec3(m_engine->m_ActionState.at(ActionState::LOOK_X), m_engine->m_ActionState.at(ActionState::LOOK_Y), 0);
	m_rotation.x = fmodf(m_rotation.x, 360.0f);
	if (m_rotation.x < 0.0f)
		m_rotation.x += 360.0f;
	if (m_rotation.y > 90.0f)
		m_rotation.y = 90.0f;
	else if (m_rotation.y < -90.0f)
		m_rotation.y = -90.0f;
	mat4 rotationMatrix = glm::rotate(mat4(1.0f), glm::radians(m_rotation.y), vec3(1.0f, 0, 0)) * glm::rotate(mat4(1.0f), glm::radians(m_rotation.x), vec3(0, 1.0f, 0));
	m_transform.m_orientation = quat_cast(rotationMatrix);

	// Determine how much to move in local space
	const float velocity = 50.0f;
	const float moveAmount = velocity * deltaTime;
	vec3 deltaPosition(0.0f);
	if (m_engine->m_ActionState.at(ActionState::FORWARD) > 0.5f)
		deltaPosition += vec3(0, 0, -moveAmount);
	if (m_engine->m_ActionState.at(ActionState::BACK) > 0.5f)
		deltaPosition += vec3(0, 0, moveAmount);
	if (m_engine->m_ActionState.at(ActionState::LEFT) > 0.5f)
		deltaPosition += vec3(-moveAmount, 0, 0);
	if (m_engine->m_ActionState.at(ActionState::RIGHT) > 0.5f)
		deltaPosition += vec3(moveAmount, 0, 0);
	// Make the translation amount be relative to the camera's orientation
	vec4 rotatedPosition = glm::inverse(rotationMatrix) * vec4(deltaPosition, 1.0f);
	m_transform.m_position += vec3(rotatedPosition / rotatedPosition.w);

	// Update the engine pointer
	m_engine->m_Camera->setPosition(m_transform.m_position);
	m_engine->m_Camera->setOrientation(m_transform.m_orientation);
	m_engine->m_Camera->update();
}
