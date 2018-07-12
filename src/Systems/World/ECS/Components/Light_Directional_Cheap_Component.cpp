#include "Systems\World\ECS\Components\Light_Directional_Cheap_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Engine.h"


Light_Directional_Cheap_Component::~Light_Directional_Cheap_Component()
{
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightDirCheapSSBO.removeElement(&m_uboIndex);
}

Light_Directional_Cheap_Component::Light_Directional_Cheap_Component(Engine *engine)
{
	m_engine = engine;
	m_uboBuffer = m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightDirCheapSSBO.addElement(&m_uboIndex);
	m_commandMap["Set_Light_Color"] = [&](const ECS_Command & payload) {
		if (payload.isType<glm::vec3>()) setColor(payload.toType<glm::vec3>());
	};
	m_commandMap["Set_Light_Intensity"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setIntensity(payload.toType<float>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};
}

void Light_Directional_Cheap_Component::setColor(const glm::vec3 & color)
{
	(&reinterpret_cast<Directional_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightColor = color;
}

void Light_Directional_Cheap_Component::setIntensity(const float & intensity)
{
	(&reinterpret_cast<Directional_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightIntensity = intensity;
}

void Light_Directional_Cheap_Component::setTransform(const Transform & transform)
{
	const glm::mat4 &rotation = transform.m_modelMatrix;
	(&reinterpret_cast<Directional_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightDirection = glm::normalize(rotation * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;	
}

bool Light_Directional_Cheap_Component::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	// Directional lights are infinite as they simulate the sun.
	return true;
}

float Light_Directional_Cheap_Component::getImportance(const glm::vec3 & position) const
{
	return 0.0f;
}