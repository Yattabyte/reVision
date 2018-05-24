#include "Systems\World\ECS\Components\Light_Spot_Cheap_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Utilities\EnginePackage.h"
#include "Utilities\Transform.h"


Light_Spot_Cheap_Component::~Light_Spot_Cheap_Component()
{
}

Light_Spot_Cheap_Component::Light_Spot_Cheap_Component(const ECShandle & id, const ECShandle & pid, EnginePackage * enginePackage) : Lighting_Component(id, pid)
{
	m_radius = 0;
	m_squaredRadius = 0;
	m_lightPos = vec3(0.0f);
	m_orientation = quat(1, 0, 0, 0);
	m_uboBuffer = enginePackage->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightSpotCheapSSBO.addElement(&m_uboIndex);
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

void Light_Spot_Cheap_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	Spot_Cheap_Struct * uboData = &reinterpret_cast<Spot_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

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
		case SET_LIGHT_RADIUS: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			m_radius = payload;
			m_squaredRadius = payload * payload;
			uboData->LightRadius = payload;
			break;
		}
		case SET_LIGHT_CUTOFF: {
			if (!message.IsOfType<float>()) break;
			const auto &payload = message.GetPayload<float>();
			uboData->LightCutoff = cosf(glm::radians(payload));
			break;
		}
		case SET_ORIENTATION: {
			if (!message.IsOfType<quat>()) break;
			const auto &payload = message.GetPayload<quat>();
			m_orientation = payload;		
			updateViews();
			break;
		}
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			m_lightPos = payload;
			uboData->LightPosition = payload;
			updateViews();
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			uboData->LightPosition = payload.m_position;
			m_lightPos = payload.m_position;
			m_orientation = payload.m_orientation;
			updateViews();
			break;
		}
	}
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