#include "Systems\World\ECS\Components\Light_Point_Cheap_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Engine.h"


Light_Point_Cheap_Component::~Light_Point_Cheap_Component()
{
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightPointCheapSSBO.removeElement(&m_uboIndex);
}

Light_Point_Cheap_Component::Light_Point_Cheap_Component(Engine *engine)
{
	m_engine = engine;
	m_radius = 0;
	m_squaredRadius = 0;
	m_lightPos = glm::vec3(0.0f);
	m_uboBuffer = engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightPointCheapSSBO.addElement(&m_uboIndex);

	m_commandMap["Set_Light_Color"] = [&](const ECS_Command & payload) {
		if (payload.isType<glm::vec3>()) setColor(payload.toType<glm::vec3>());
	};
	m_commandMap["Set_Light_Intensity"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setIntensity(payload.toType<float>());
	};
	m_commandMap["Set_Light_Radius"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setRadius(payload.toType<float>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};
}

void Light_Point_Cheap_Component::updateViews()
{
	Point_Cheap_Struct * uboData = &reinterpret_cast<Point_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	// Calculate view matrixs
	const glm::mat4 trans = glm::translate(glm::mat4(1.0f), m_lightPos);
	const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(m_squaredRadius));
	uboData->mMatrix = trans * scl;
}

void Light_Point_Cheap_Component::setColor(const glm::vec3 & color)
{
	(&reinterpret_cast<Point_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightColor = color;
}

void Light_Point_Cheap_Component::setIntensity(const float & intensity)
{
	(&reinterpret_cast<Point_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightIntensity = intensity;
}

void Light_Point_Cheap_Component::setRadius(const float & radius)
{
	m_radius = radius;
	m_squaredRadius = radius * radius;
	(&reinterpret_cast<Point_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightRadius = radius;
	updateViews();
}

void Light_Point_Cheap_Component::setTransform(const Transform & transform)
{
	(&reinterpret_cast<Point_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightPosition = transform.m_position;
	m_lightPos = transform.m_position;
	updateViews();
}

bool Light_Point_Cheap_Component::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

float Light_Point_Cheap_Component::getImportance(const glm::vec3 & position) const
{
	return 0.0f;
}