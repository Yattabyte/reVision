#include "ECS\Components\Light_Spot_Cheap.h"
#include "ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Engine.h"


Light_Spot_Cheap_C::~Light_Spot_Cheap_C()
{
	m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightSpotCheapSSBO.removeElement(&m_uboIndex);
}

Light_Spot_Cheap_C::Light_Spot_Cheap_C(Engine * engine, const ArgumentList & argumentList)
	: Light_Spot_Cheap_C(
		engine,
		*(glm::vec3*)argumentList.dataPointers[0],
		*(float*)argumentList.dataPointers[1],
		*(float*)argumentList.dataPointers[2],
		*(float*)argumentList.dataPointers[3],
		*(Transform*)argumentList.dataPointers[4]
	) {}

Light_Spot_Cheap_C::Light_Spot_Cheap_C(Engine * engine, const glm::vec3 & color, const float & intensity, const float & radius, const float & cutoff, const Transform & transform)
	: Lighting_C(engine)
{
	// Default Parameters
	m_radius = 0;
	m_squaredRadius = 0;
	m_lightPos = glm::vec3(0.0f);
	m_orientation = glm::quat(1, 0, 0, 0);

	// Acquire and update buffers
	m_uboBuffer = m_engine->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightSpotCheapSSBO.addElement(&m_uboIndex);
	
	// Register Commands
	m_commandMap["Set_Light_Color"] = [&](const ECS_Command & payload) {
		if (payload.isType<glm::vec3>()) setColor(payload.toType<glm::vec3>());
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

	// Update with passed parameters
	setColor(color);
	setIntensity(intensity);
	setRadius(radius);
	setCutoff(cutoff);
	setTransform(transform);
}

void Light_Spot_Cheap_C::setColor(const glm::vec3 & color)
{
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightColor = color;
}

void Light_Spot_Cheap_C::setIntensity(const float & intensity)
{
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightIntensity = intensity;
}

void Light_Spot_Cheap_C::setRadius(const float & radius)
{
	m_radius = radius;
	m_squaredRadius = radius * radius;
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightRadius = radius;
	updateViews();
}

void Light_Spot_Cheap_C::setCutoff(const float & cutoff)
{
	(&reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->LightCutoff = cosf(glm::radians(cutoff));
	updateViews();
}

void Light_Spot_Cheap_C::setTransform(const Transform & transform)
{
	Spot_Cheap_Struct * uboData = &reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->LightPosition = transform.m_position;
	m_lightPos = transform.m_position;
	m_orientation = transform.m_orientation;
	updateViews();
}

void Light_Spot_Cheap_C::updateViews()
{
	Spot_Cheap_Struct * uboData = &reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	
	// Recalculate view matrix
	const glm::mat4 trans = glm::translate(glm::mat4(1.0f), m_lightPos);
	const glm::mat4 rot = glm::mat4_cast(m_orientation);
	const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(m_squaredRadius));
	uboData->LightDirection = (rot * glm::vec4(1, 0, 0, 0)).xyz;
	uboData->mMatrix = (trans * rot) * scl;
}

bool Light_Spot_Cheap_C::isVisible(const float & radius, const glm::vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

float Light_Spot_Cheap_C::getImportance(const glm::vec3 & position) const
{
	return 0.0f;
}