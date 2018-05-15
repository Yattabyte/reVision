#include "Systems\World\ECS\Components\Light_Directional_Cheap_Component.h"
#include "Utilities\EnginePackage.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Utilities\Transform.h"
#include "Systems\Graphics\Graphics.h"
#include "GLFW\glfw3.h"


Light_Directional_Cheap_Component::~Light_Directional_Cheap_Component()
{
}

Light_Directional_Cheap_Component::Light_Directional_Cheap_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_uboBuffer = enginePackage->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightDirCheapSSBO.addElement(&m_uboIndex);
}

void Light_Directional_Cheap_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	Directional_Cheap_Struct * uboData = &reinterpret_cast<Directional_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	switch (message.GetCommandID()) {
		case SET_LIGHT_COLOR: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			uboData->LightColor = payload;
			break;
		}
		case SET_LIGHT_INTENSITY: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			uboData->LightIntensity = payload;
			break;
		}
		case SET_ORIENTATION: {
			if (!message.IsOfType<quat>()) break;
			const auto &payload = message.GetPayload<quat>();
			const mat4 rotation = glm::mat4_cast(payload);
			uboData->LightDirection = glm::normalize(rotation * vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
			update();
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			const mat4 &rotation = payload.m_modelMatrix;
			uboData->LightDirection = glm::normalize(rotation * vec4(1.0f, 0.0f, 0.0f, 0.0f)).xyz;
			update();
			break;
		}
	}
}

bool Light_Directional_Cheap_Component::isVisible(const float & radius, const vec3 & eyePosition) const
{
	// Directional lights are infinite as they simulate the sun.
	return true;
}

void Light_Directional_Cheap_Component::occlusionPass()
{
}

void Light_Directional_Cheap_Component::shadowPass()
{	
}

float Light_Directional_Cheap_Component::getImportance(const vec3 & position) const
{
	return 0.0f;
}

void Light_Directional_Cheap_Component::update()
{
}