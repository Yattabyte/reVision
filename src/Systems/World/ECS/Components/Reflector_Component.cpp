#include "Systems\World\ECS\Components\Reflector_Component.h"
#include "Systems\Graphics\Graphics.h"
#include "Utilities\EnginePackage.h"
#include "glm\gtc\matrix_transform.hpp"


Reflector_Component::~Reflector_Component()
{
}

Reflector_Component::Reflector_Component(const ECShandle & id, const ECShandle & pid, EnginePackage * enginePackage) : Component(id, pid)
{ 
	m_fence = nullptr;
	m_enginePackage = enginePackage;

	if (m_enginePackage->findSubSystem("Graphics"))
		m_uboBuffer = m_enginePackage->getSubSystem<System_Graphics>("Graphics")->getReflectionBuffer().addReflector(m_uboIndex);
	
	reinterpret_cast<Reflection_Struct*>(m_uboBuffer)[m_uboIndex] = m_uboData;
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void Reflector_Component::receiveMessage(const ECSmessage & message)
{
	if (Component::compareMSGSender(message)) return;
	switch (message.GetCommandID()) {
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			m_uboData.mMatrix = glm::translate(mat4(1.0f), payload); 
			m_uboData.BoxCamPos.xyz = payload;
			reinterpret_cast<Reflection_Struct*>(m_uboBuffer)[m_uboIndex] = m_uboData;
			m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}
	}
}

void Reflector_Component::draw()
{
	if (m_fence != nullptr) {
		const auto state = glClientWaitSync(m_fence, 0, 0);
		if (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED)) && (state != GL_WAIT_FAILED)) {

			// draw stuff //
		}
	}
}

bool Reflector_Component::isVisible(const mat4 & PMatrix, const mat4 & VMatrix)
{
	return Frustum(PMatrix * VMatrix).sphereInFrustum(m_uboData.BoxCamPos.xyz, vec3(1.0f));
}

void Reflector_Component::update()
{
	/*if (m_fence != nullptr) {
		auto state = glClientWaitSync(m_fence, 0, 0);
		while (((state == GL_ALREADY_SIGNALED) || (state == GL_CONDITION_SATISFIED)) && (state != GL_WAIT_FAILED))
			state = glClientWaitSync(m_fence, 0, 0);
	}
	glNamedBufferSubData(m_uboID, 0, sizeof(Reflector_Component::Transform_Buffer), &m_uboData);*/
}

const unsigned int Reflector_Component::getBufferIndex() const
{
	return m_uboIndex;
}

