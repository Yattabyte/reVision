#pragma once
#ifndef FRUSTUMCULL_SYSTEM_H
#define FRUSTUMCULL_SYSTEM_H

#include "Modules/World/ECS/ecsSystem.h"
#include "Modules/World/ECS/components.h"
#include "Modules/Graphics/Common/components.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "glm/gtc/type_ptr.hpp"
#include <memory>


/***/
class FrustumCull_System : public BaseECSSystem {
public:
	// Public (de)Constructors
	/***/
	inline ~FrustumCull_System() = default;
	/***/
	inline FrustumCull_System() {
		addComponentType(Renderable_Component::ID, FLAG_REQUIRED);
		addComponentType(Transform_Component::ID, FLAG_REQUIRED);
		addComponentType(BoundingSphere_Component::ID, FLAG_OPTIONAL);
	}


	// Public Interface Implementations
	inline virtual void updateComponents(const float & deltaTime, const std::vector<std::vector<BaseECSComponent*>> & components) override {
		for each (const auto & componentParam in components) {
			Renderable_Component * renderableComponent = (Renderable_Component*)componentParam[0];
			Transform_Component * transformComponent = (Transform_Component*)componentParam[1];
			BoundingSphere_Component * bsphereComponent = (BoundingSphere_Component*)componentParam[2];
			
			// Err on the side of caution and say its visible
			bool isVisible = true;

			// Our visibility tests will try to EXCLUDE, not INCLUDE
			// Frustum x Bounding-Sphere Test
			if (bsphereComponent) {
				const auto position = transformComponent->m_transform.m_position + bsphereComponent->m_positionOffset;
				const auto radius = bsphereComponent->m_radius;
				for (int i = 0; i < 6; ++i)	{
					if (m_planes[i].x * position.x + m_planes[i].y * position.y + m_planes[i].z * position.z + m_planes[i].w <= -radius) {
						isVisible = false;
						break;
					}
				}
			}

			// Set visibility
			renderableComponent->m_visible = isVisible;
		}
	}


	// Public Methods
	/***/
	void setViewport(const std::shared_ptr<Viewport> & viewport) {
		constexpr static auto normalizePlane = [](glm::vec4 &plane) {
			float magnitude = (float)sqrtf(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			plane[0] /= magnitude;
			plane[1] /= magnitude;
			plane[2] /= magnitude;
			plane[3] /= magnitude;
		};
		const auto pMatrix = glm::value_ptr(viewport->getPerspectiveMatrix());
		const auto vMatrix = glm::value_ptr(viewport->getViewMatrix());
		float   clip[16]; //clipping planes

		clip[0] = vMatrix[0] * pMatrix[0] + vMatrix[1] * pMatrix[4] + vMatrix[2] * pMatrix[8] + vMatrix[3] * pMatrix[12];
		clip[1] = vMatrix[0] * pMatrix[1] + vMatrix[1] * pMatrix[5] + vMatrix[2] * pMatrix[9] + vMatrix[3] * pMatrix[13];
		clip[2] = vMatrix[0] * pMatrix[2] + vMatrix[1] * pMatrix[6] + vMatrix[2] * pMatrix[10] + vMatrix[3] * pMatrix[14];
		clip[3] = vMatrix[0] * pMatrix[3] + vMatrix[1] * pMatrix[7] + vMatrix[2] * pMatrix[11] + vMatrix[3] * pMatrix[15];

		clip[4] = vMatrix[4] * pMatrix[0] + vMatrix[5] * pMatrix[4] + vMatrix[6] * pMatrix[8] + vMatrix[7] * pMatrix[12];
		clip[5] = vMatrix[4] * pMatrix[1] + vMatrix[5] * pMatrix[5] + vMatrix[6] * pMatrix[9] + vMatrix[7] * pMatrix[13];
		clip[6] = vMatrix[4] * pMatrix[2] + vMatrix[5] * pMatrix[6] + vMatrix[6] * pMatrix[10] + vMatrix[7] * pMatrix[14];
		clip[7] = vMatrix[4] * pMatrix[3] + vMatrix[5] * pMatrix[7] + vMatrix[6] * pMatrix[11] + vMatrix[7] * pMatrix[15];

		clip[8] = vMatrix[8] * pMatrix[0] + vMatrix[9] * pMatrix[4] + vMatrix[10] * pMatrix[8] + vMatrix[11] * pMatrix[12];
		clip[9] = vMatrix[8] * pMatrix[1] + vMatrix[9] * pMatrix[5] + vMatrix[10] * pMatrix[9] + vMatrix[11] * pMatrix[13];
		clip[10] = vMatrix[8] * pMatrix[2] + vMatrix[9] * pMatrix[6] + vMatrix[10] * pMatrix[10] + vMatrix[11] * pMatrix[14];
		clip[11] = vMatrix[8] * pMatrix[3] + vMatrix[9] * pMatrix[7] + vMatrix[10] * pMatrix[11] + vMatrix[11] * pMatrix[15];

		clip[12] = vMatrix[12] * pMatrix[0] + vMatrix[13] * pMatrix[4] + vMatrix[14] * pMatrix[8] + vMatrix[15] * pMatrix[12];
		clip[13] = vMatrix[12] * pMatrix[1] + vMatrix[13] * pMatrix[5] + vMatrix[14] * pMatrix[9] + vMatrix[15] * pMatrix[13];
		clip[14] = vMatrix[12] * pMatrix[2] + vMatrix[13] * pMatrix[6] + vMatrix[14] * pMatrix[10] + vMatrix[15] * pMatrix[14];
		clip[15] = vMatrix[12] * pMatrix[3] + vMatrix[13] * pMatrix[7] + vMatrix[14] * pMatrix[11] + vMatrix[15] * pMatrix[15];

		m_planes[0][0] = clip[3] - clip[0];
		m_planes[0][1] = clip[7] - clip[4];
		m_planes[0][2] = clip[11] - clip[8];
		m_planes[0][3] = clip[15] - clip[12];
		normalizePlane(m_planes[0]);

		m_planes[1][0] = clip[3] + clip[0];
		m_planes[1][1] = clip[7] + clip[4];
		m_planes[1][2] = clip[11] + clip[8];
		m_planes[1][3] = clip[15] + clip[12];
		normalizePlane(m_planes[1]);

		m_planes[2][0] = clip[3] + clip[1];
		m_planes[2][1] = clip[7] + clip[5];
		m_planes[2][2] = clip[11] + clip[9];
		m_planes[2][3] = clip[15] + clip[13];
		normalizePlane(m_planes[2]);

		m_planes[3][0] = clip[3] - clip[1];
		m_planes[3][1] = clip[7] - clip[5];
		m_planes[3][2] = clip[11] - clip[9];
		m_planes[3][3] = clip[15] - clip[13];
		normalizePlane(m_planes[3]);

		m_planes[4][0] = clip[3] - clip[2];
		m_planes[4][1] = clip[7] - clip[6];
		m_planes[4][2] = clip[11] - clip[10];
		m_planes[4][3] = clip[15] - clip[14];
		normalizePlane(m_planes[4]);

		m_planes[5][0] = clip[3] + clip[2];
		m_planes[5][1] = clip[7] + clip[6];
		m_planes[5][2] = clip[11] + clip[10];
		m_planes[5][3] = clip[15] + clip[14];
		normalizePlane(m_planes[5]);
	}


private:
	// Private Attributes
	glm::vec4 m_planes[6];
};

#endif // FRUSTUMCULL_SYSTEM_H