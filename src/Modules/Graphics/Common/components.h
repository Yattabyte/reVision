#pragma once
#ifndef GRAPHICS_COMPONENTS_H
#define GRAPHICS_COMPONENTS_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/Graphics/Common/Viewport.h"
#include <memory>


/***/
struct Renderable_Component : public ECSComponent<Renderable_Component> {
	bool m_visible = false;
};

/***/
struct CameraFollower_Component : public ECSComponent<CameraFollower_Component> {
	std::shared_ptr<Viewport> m_viewport;
};

/***/
struct BoundingSphere_Component : public ECSComponent<BoundingSphere_Component> {
	glm::vec3 m_positionOffset = glm::vec3(0.0f);
	float m_radius = 1.0f;
};

#endif // GRAPHICS_COMPONENTS_H