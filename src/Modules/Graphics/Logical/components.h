#pragma once
#ifndef GRAPHICS_COMPONENTS_H
#define GRAPHICS_COMPONENTS_H

#include "Modules/World/ECS/ecsComponent.h"
#include "Modules/Graphics/Common/Viewport.h"
#include "Modules/Graphics/Common/Camera.h"
#include <memory>


struct Renderable_Component : public ECSComponent<Renderable_Component> {
	std::vector<int> m_visible = {};
	bool m_visibleAtAll = false;
};

struct Camera_Component : public ECSComponent<Camera_Component> {
	Camera m_camera;
	float m_updateTime = 0.0f;
};

struct CameraArray_Component : public ECSComponent<CameraArray_Component> {
	std::vector<Camera> m_cameras;
	std::vector<float> m_updateTimes;
};

struct BoundingSphere_Component : public ECSComponent<BoundingSphere_Component> {
	glm::vec3 m_positionOffset = glm::vec3(0.0f);
	float m_radius = 1.0f;
	enum CameraCollision {
		OUTSIDE, INSIDE
	} m_cameraCollision;
};

#endif // GRAPHICS_COMPONENTS_H