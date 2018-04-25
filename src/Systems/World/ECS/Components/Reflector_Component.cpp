#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Systems\Graphics\Graphics.h"
#include "Systems\Graphics\Resources\GFX_DEFINES.h"
#include "Utilities\EnginePackage.h"
#include "Utilities\Transform.h"
#include "glm\gtc\matrix_transform.hpp"


Reflector_Component::~Reflector_Component()
{
	m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_reflectionSSBO.removeElement(&m_uboIndex);
}

Reflector_Component::Reflector_Component(const ECShandle & id, const ECShandle & pid, EnginePackage * enginePackage) : Component(id, pid)
{ 
	m_enginePackage = enginePackage;
	m_position = vec3(0.0f);
	m_scale = vec3(1.0f);
	m_uboBuffer = m_enginePackage->getSubSystem<System_Graphics>("Graphics")->m_reflectionSSBO.addElement(&m_uboIndex);
}

void Reflector_Component::receiveMessage(const ECSmessage & message)
{
	if (Component::compareMSGSender(message)) return;
	Reflection_Struct * uboData = &reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	switch (message.GetCommandID()) {
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			uboData->mMatrix = glm::translate(mat4(1.0f), payload);
			uboData->BoxCamPos.xyz = payload;
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			uboData->mMatrix = payload.m_modelMatrix;
			uboData->BoxCamPos.xyz = payload.m_position;			
			m_position = payload.m_position;
			m_scale = payload.m_scale;
			break;
		}
		case SET_REFLECTOR_RADIUS: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			uboData->Radius = payload;
			break;
		}
	}
}

bool Reflector_Component::isVisible(const float & radius, const vec3 & eyePosition, const mat4 & PMatrix, const mat4 & VMatrix) const
{
	const float distance = glm::distance(m_position, eyePosition);
	return radius + glm::length(m_scale) > distance;
}

const unsigned int Reflector_Component::getBufferIndex() const
{
	return m_uboIndex;
}

