#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Systems\Graphics\Graphics.h"
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
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void Reflector_Component::receiveMessage(const ECSmessage & message)
{
	if (Component::compareMSGSender(message)) return;
	switch (message.GetCommandID()) {
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			Reflection_Struct * uboData = &reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
			uboData->mMatrix = glm::translate(mat4(1.0f), payload);
			uboData->BoxCamPos.xyz = payload;
			m_position = payload;
			m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			Reflection_Struct * uboData = &reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
			uboData->mMatrix = payload.m_modelMatrix;
			uboData->BoxCamPos.xyz = payload.m_position;			
			m_position = payload.m_position;
			m_scale = payload.m_scale;
			m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			break;
		}
		case SET_REFLECTOR_RADIUS: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			Reflection_Struct * uboData = &reinterpret_cast<Reflection_Struct*>(m_uboBuffer->pointer)[m_uboIndex];
			uboData->Radius = payload;
			m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			break;
		}
	}
}

bool Reflector_Component::isVisible(const mat4 & PMatrix, const mat4 & VMatrix) const
{
	return Frustum(PMatrix * VMatrix).sphereInFrustum(m_position.xyz, m_scale);
}

const unsigned int Reflector_Component::getBufferIndex() const
{
	return m_uboIndex;
}

