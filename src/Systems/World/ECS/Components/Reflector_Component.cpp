#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\EnginePackage.h"
#include "glm\gtc\matrix_transform.hpp"


Reflector_Component::~Reflector_Component()
{
	m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_reflectionSSBO.removeElement(&m_uboIndex);
}

Reflector_Component::Reflector_Component(EnginePackage * enginePackage)
{ 
	m_enginePackage = enginePackage;
	m_position = vec3(0.0f);
	m_scale = vec3(1.0f);
	m_uboBuffer = m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_reflectionSSBO.addElement(&m_uboIndex);

	m_commandMap["Set_Reflector_Radius"] = [&](const ECS_Command & payload) {
		if (payload.isType<float>()) setRadius(payload.toType<float>());
	};
	m_commandMap["Set_Transform"] = [&](const ECS_Command & payload) {
		if (payload.isType<Transform>()) setTransform(payload.toType<Transform>());
	};
}

void Reflector_Component::setRadius(const float & radius)
{
	(&reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex])->Radius = radius;
}

void Reflector_Component::setTransform(const Transform & transform)
{
	Reflection_Struct * uboData = &reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
	uboData->mMatrix = transform.m_modelMatrix;
	uboData->BoxCamPos.xyz = transform.m_position;
	m_position = transform.m_position;
	m_scale = transform.m_scale;
}

bool Reflector_Component::isVisible(const float & radius, const vec3 & eyePosition) const
{
	const float distance = glm::distance(m_position, eyePosition);
	return radius + glm::length(m_scale) > distance;
}

const unsigned int Reflector_Component::getBufferIndex() const
{
	return m_uboIndex;
}

