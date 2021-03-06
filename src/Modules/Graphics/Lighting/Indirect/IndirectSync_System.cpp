#include "Modules/Graphics/Lighting/Indirect/IndirectSync_System.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectData.h"
#include "Modules/ECS/component_types.h"


IndirectSync_System::IndirectSync_System(Indirect_Light_Data& frameData) :
	m_frameData(frameData)
{
	addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
}

void IndirectSync_System::updateComponents(const float& /*deltaTime*/, const std::vector<std::vector<ecsBaseComponent*>>& components)
{
	// Resize light buffers to match number of entities this frame
	m_frameData.lightBuffer.resize(components.size());
	m_frameData.lightBuffer.beginWriting();
	int index = 0;
	for (const auto& componentParam : components) {
		auto* trans = static_cast<Transform_Component*>(componentParam[0]);
		const auto* light = static_cast<Light_Component*>(componentParam[1]);
		const auto* shadow = static_cast<Shadow_Component*>(componentParam[2]);

		// Sync Common Buffer Attributes
		const auto radiusSquared = (light->m_radius * light->m_radius);
		trans->m_localTransform.m_scale = glm::vec3(radiusSquared * 1.1F);
		trans->m_localTransform.update();
		m_frameData.lightBuffer[index].LightColor = light->m_color;
		m_frameData.lightBuffer[index].LightPosition = trans->m_worldTransform.m_position;
		m_frameData.lightBuffer[index].LightIntensity = light->m_intensity;
		m_frameData.lightBuffer[index].Shadow_Spot = shadow->m_shadowSpot;
		m_frameData.lightBuffer[index].Light_Type = static_cast<int>(light->m_type);

		// Copy Shadow Attributes
		{
			const auto shadowCameraCount = shadow->m_cameras.size();
			for (int x = 0; x < shadowCameraCount; ++x) {
				if (shadow->m_cameras[x].getEnabled()) {
					m_frameData.lightBuffer[index].lightVP[x] = shadow->m_cameras[x].get()->pvMatrix;
					m_frameData.lightBuffer[index].CascadeEndClipSpace[x] = shadow->m_cameras[x].get()->FarPlane;
				}
			}
		}
		index++;
	}
	m_frameData.lightBuffer.endWriting();
}