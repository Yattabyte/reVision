#pragma once
#ifndef GRAPHICS_COMPONENTS_H
#define GRAPHICS_COMPONENTS_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/Graphics/Common/Viewport.h"
#include <memory>


/***/
struct Renderable_Component : public ECSComponent<Renderable_Component> {
	std::vector<int> m_visible = {};
	bool m_visibleAtAll = false;
};

/***/
struct CameraFollower_Component : public ECSComponent<CameraFollower_Component> {
	std::shared_ptr<Viewport> m_viewport;
};

/***/
struct Viewport_Component : public ECSComponent<Viewport_Component> {
	std::shared_ptr<Viewport> m_camera;
};

/***/
struct BoundingSphere_Component : public ECSComponent<BoundingSphere_Component> {
	glm::vec3 m_positionOffset = glm::vec3(0.0f);
	float m_radius = 1.0f;
	enum CameraCollision {
		OUTSIDE, INSIDE
	} m_cameraCollision;
};

#endif // GRAPHICS_COMPONENTS_H