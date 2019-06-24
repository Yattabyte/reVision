#pragma once
#ifndef SPOTSYNC_SYSTEM_H
#define SPOTSYNC_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Logical/components.h"
#include "Modules/Graphics/Lighting/components.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include <memory>


/***/
struct Spot_Buffers {
	/** OpenGL buffer for spot lights. */
	struct Spot_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV;
		glm::mat4 inversePV;
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		glm::vec3 LightDirection; float padding3;
		float LightIntensity;
		float LightRadius;
		float LightCutoff;
		int Shadow_Spot;
	};
	GL_ArrayBuffer<Spot_Buffer> lightBuffer;
	bool shadowOutOfDate = false;
};

/***/
class SpotSync_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~SpotSync_System() = default;
	/***/
	inline SpotSync_System(const std::shared_ptr<Spot_Buffers> & buffers)
		: m_buffers(buffers) {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(LightSpot_Component::ID, FLAG_REQUIRED);
		addComponentType(LightColor_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightRadius_Component::ID, FLAG_OPTIONAL);
		addComponentType(LightCutoff_Component::ID, FLAG_OPTIONAL);
		addComponentType(Transform_Component::ID, FLAG_OPTIONAL);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			LightSpot_Component * lightComponent = (LightSpot_Component*)componentParam[1];
			LightColor_Component * colorComponent = (LightColor_Component*)componentParam[2];
			LightRadius_Component * radiusComponent = (LightRadius_Component*)componentParam[3];
			LightCutoff_Component * cutoffComponent = (LightCutoff_Component*)componentParam[4];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[5];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[6];
			const auto & index = lightComponent->m_lightIndex;

			// Relay when shadows need to be rebuilt
			if (m_buffers->shadowOutOfDate)
				lightComponent->m_staticOutOfDate = true;			

			// Synchronize the component if it is visible
			if (renderableComponent->m_visible) {
				// Sync Color Attributes
				if (colorComponent) {
					m_buffers->lightBuffer[index].LightColor = colorComponent->m_color;
					m_buffers->lightBuffer[index].LightIntensity = colorComponent->m_intensity;
				}

				// Sync Radius Attributes
				float radiusSquared = 1.0f;
				if (radiusComponent) {
					m_buffers->lightBuffer[index].LightRadius = radiusComponent->m_radius;
					radiusSquared = radiusComponent->m_radius * radiusComponent->m_radius;
				}
				if (bsphereComponent)
					bsphereComponent->m_radius = radiusSquared;

				// Sync Cutoff Attributes
				float cutoff = 90.0f;
				if (cutoffComponent) {
					m_buffers->lightBuffer[index].LightCutoff = cosf(glm::radians(cutoffComponent->m_cutoff));
					cutoff = cutoffComponent->m_cutoff;
				}

				// Sync Transform Attributes
				if (transformComponent) {
					const auto & position = transformComponent->m_transform.m_position;
					const auto & orientation = transformComponent->m_transform.m_orientation;
					const auto matRot = glm::mat4_cast(orientation);
					lightComponent->m_position = position;
					m_buffers->lightBuffer[index].LightPosition = position;
					auto dir = matRot * glm::vec4(0, 0, -1, 1);
					dir /= dir.w;
					m_buffers->lightBuffer[index].LightDirection = glm::normalize(glm::vec3(dir));
					const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
					const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radiusSquared)*1.1f);
					m_buffers->lightBuffer[index].mMatrix = trans * matRot * scl;

					if (lightComponent->m_hasShadow) {
						const glm::mat4 pMatrix = glm::perspective(glm::radians(cutoff * 2.0f), 1.0f, 0.01f, radiusSquared);
						const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
						m_buffers->lightBuffer[index].lightV = trans;
						const auto pv = pMatrix * glm::inverse(trans * glm::mat4_cast(orientation));
						m_buffers->lightBuffer[index].lightPV = pv;
						m_buffers->lightBuffer[index].inversePV = glm::inverse(pv);
					}
				}

				// Sync Buffer Attributes
				m_buffers->lightBuffer[index].Shadow_Spot = lightComponent->m_shadowSpot;
			}
		}
	}

private:
	// Private Attributes
	std::shared_ptr<Spot_Buffers> m_buffers;
};

#endif // SPOTSYNC_SYSTEM_H