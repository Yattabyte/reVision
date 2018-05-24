#include "Systems\World\ECS\Components\Light_Point_Cheap_Component.h"
#include "Systems\World\ECS\ECSmessage.h"
#include "Systems\Graphics\Graphics.h"
#include "Utilities\EnginePackage.h"
#include "Utilities\Transform.h"


Light_Point_Cheap_Component::~Light_Point_Cheap_Component()
{
}

Light_Point_Cheap_Component::Light_Point_Cheap_Component(const ECShandle & id, const ECShandle & pid, EnginePackage *enginePackage) : Lighting_Component(id, pid)
{
	m_radius = 0;
	m_squaredRadius = 0;
	m_lightPos = vec3(0.0f);
	m_uboBuffer = enginePackage->getSubSystem<System_Graphics>("Graphics")->m_lightBuffers.m_lightPointCheapSSBO.addElement(&m_uboIndex);
}

void Light_Point_Cheap_Component::updateViews()
{
	Point_Cheap_Struct * uboData = &reinterpret_cast<Point_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

	// Calculate view matrixs
	const mat4 trans = glm::translate(mat4(1.0f), m_lightPos);
	const mat4 scl = glm::scale(mat4(1.0f), vec3(m_squaredRadius));
	uboData->mMatrix = trans * scl;
}

void Light_Point_Cheap_Component::receiveMessage(const ECSmessage &message)
{
	if (Component::compareMSGSender(message)) return;
	Point_Cheap_Struct * uboData = &reinterpret_cast<Point_Cheap_Struct*>(m_uboBuffer->pointer)[m_uboIndex];

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
			updateViews();
			break;
		}
		case SET_POSITION: {
			if (!message.IsOfType<vec3>()) break;
			const auto &payload = message.GetPayload<vec3>();
			uboData->LightPosition = payload;
			m_lightPos = payload;
			updateViews();
			break;
		}
		case SET_TRANSFORM: {
			if (!message.IsOfType<Transform>()) break;
			const auto &payload = message.GetPayload<Transform>();
			uboData->LightPosition = payload.m_position;
			m_lightPos = payload.m_position;
			updateViews();
			break;
		}
	}
}

bool Light_Point_Cheap_Component::isVisible(const float & radius, const vec3 & eyePosition) const
{
	const float distance = glm::distance(m_lightPos, eyePosition);
	return radius + m_radius > distance;
}

float Light_Point_Cheap_Component::getImportance(const vec3 & position) const
{
	return 0.0f;
}