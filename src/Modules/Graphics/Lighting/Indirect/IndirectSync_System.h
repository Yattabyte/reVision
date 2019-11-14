#pragma once
#ifndef INDIRECTSYNC_SYSTEM_H
#define INDIRECTSYNC_SYSTEM_H

#include "Modules/ECS/ecsSystem.h"
#include "Modules/ECS/component_types.h"
#include "Modules/Graphics/Lighting/Indirect/IndirectData.h"
#include <algorithm>


/** An ECS system responsible for synchronizing indirect light components and sending data to the GPU. */
class IndirectSync_System final : public ecsBaseSystem {
public:
	// Public (De)Constructors
	/** Destroy this system. */
	inline ~IndirectSync_System() = default;
	/** Construct this system.
	@param	frameData	shared pointer of common data that changes frame-to-frame. */
	inline explicit IndirectSync_System(const std::shared_ptr<Indirect_Light_Data>& frameData)
		: m_frameData(frameData) {
		addComponentType(Transform_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
		addComponentType(Light_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
		addComponentType(Shadow_Component::Runtime_ID, RequirementsFlag::FLAG_REQUIRED);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float& deltaTime, const std::vector<std::vector<ecsBaseComponent*>>& components) noexcept override final {
		// Resize light buffers to match number of entities this frame
		m_frameData->lightBuffer.resize(components.size());
		m_frameData->lightBuffer.beginWriting();
		int index = 0;
		for each (const auto & componentParam in components) {
			auto* trans = static_cast<Transform_Component*>(componentParam[0]);
			auto* light = static_cast<Light_Component*>(componentParam[1]);
			const auto* shadow = static_cast<Shadow_Component*>(componentParam[2]);

			// Sync Common Buffer Attributes
			const auto radiusSquared = (light->m_radius * light->m_radius);
			const auto& position = trans->m_worldTransform.m_position;
			const auto transM = glm::translate(glm::mat4(1.0f), position);
			const auto rotM = glm::mat4_cast(trans->m_worldTransform.m_orientation);
			const auto sclM = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared * 1.1f));
			trans->m_localTransform.m_scale = glm::vec3(radiusSquared * 1.1f);
			trans->m_localTransform.update();
			m_frameData->lightBuffer[index].LightColor = light->m_color;
			m_frameData->lightBuffer[index].LightPosition = trans->m_worldTransform.m_position;
			m_frameData->lightBuffer[index].LightIntensity = light->m_intensity;
			m_frameData->lightBuffer[index].Shadow_Spot = shadow->m_shadowSpot;
			m_frameData->lightBuffer[index].Light_Type = static_cast<int>(light->m_type);

			// Copy Shadow Attributes
			{
				for (int x = 0; x < shadow->m_cameras.size(); ++x) {
					if (shadow->m_cameras[x].getEnabled()) {
						m_frameData->lightBuffer[index].lightVP[x] = shadow->m_cameras[x].get()->pvMatrix;
						m_frameData->lightBuffer[index].CascadeEndClipSpace[x] = shadow->m_cameras[x].get()->FarPlane;
					}
				}
			}
			index++;
		}
		m_frameData->lightBuffer.endWriting();
	}


private:
	// Private Attributes
	std::shared_ptr<Indirect_Light_Data> m_frameData;
};

#endif // INDIRECTSYNC_SYSTEM_H