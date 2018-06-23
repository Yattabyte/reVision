#include "Systems\World\ECS\Components\Light_Spot_Cheap_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Engine.h"


Light_Spot_Cheap_Component::~Light_Spot_Cheap_Component()
{
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightSpotCheapSSBO.removeElement(&m_uboIndex);
}

Light_Spot_Cheap_Component::Light_Spot_Cheap_Component(Engine * engine)
{
	m_engine = engine;
	m_radius = 0;
	m_squaredRadius = 0;
	m_lightPos = vec3(0.0f);
	m_orientation = quat(1, 0, 0, 0);
	m_uboBuffer = m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightSpotCheapSSBO.addElement(&m_uboIndex);


	m_commandMap["Set_Light_Color"] = [&](const ECS_Command & payload) {
		if (payload.isType<vec3>()) setColor(payload.toType<vec3>());
	};
	m_commandMap["Set_Light_Intensity"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setIntensity(payload.toType<float>());
	};
	m_commandMap["Set_Light_Radius"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setRadius(payload.toType<float>());
	};
	m_commandMap["Set_Light_Cutoff"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setCutoff(payload.toType<float>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};
}

void Light_Spot_Cheap_Component::setColor(const vec3 & color)
{
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightColor = color;
}

void Light_Spot_Cheap_Component::setIntensity(const float & intensity)
{
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightIntensity = intensity;
}

void Light_Spot_Cheap_Component::setRadius(const float & radius)
{
	m_radius = radius;
	m_squaredRadius = radius * radius;
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightRadius = radius;
	updateViews();
}

void Light_Spot_Cheap_Component::setCutoff(const float & cutoff)
{
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightCutoff = cosf(glm::radians(cutoff));
	updateViews();
}

void Light_Spot_Cheap_Component::setTransform(const Transform & transform)
{
	Spot_Cheap_Struct * uboData = &reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->LightPosition = transform.m_position;
	m_lightPos = transform.m_position;
	m_orientation = transform.m_orientation;
	updateViews();
}

void Light_Spot_Cheap_Component::updateViews()
{
	Spot_Cheap_Struct * uboData = &reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	
	// Recalculate view matrix
	const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
	const mat4 rot = glm::mat4_cast(m_orientation);
	const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
	uboData->LightDirection = (rot * vec4(1, 0, 0, 0)).xyz;
	uboData->mMatrix = (trans * rot) * scl;
}

bool Light_Spot_Cheap_Component::isVisible(const float & radius, const vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

float Light_Spot_Cheap_Component::getImportance(const vec3 & position) const
{
	return 0.0f;
}