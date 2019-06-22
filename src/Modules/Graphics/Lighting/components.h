#pragma once
#ifndef GRAPHICS_LIGHTING_COMPONENTS_H
#define GRAPHICS_LIGHTING_COMPONENTS_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include <memory>

#define NUM_CASCADES 4


/** A directional light component, like a sun. */
struct LightDirectional_Component : public ECSComponent<LightDirectional_Component> {
	// Light Properties
	glm::vec3 m_color = glm::vec3(1.0f);
	glm::vec3 m_direction = glm::vec3(0, -1, 0);
	float m_intensity = 1.0f;
	glm::mat4 m_mMatrix = glm::mat4(1.0f);

	// Shadow Properties
	bool m_hasShadow = false;
	float m_updateTime = 0.0f;
	int m_shadowSpot = -1;

	// System Identifier
	GL_AB_Index m_lightIndex = nullptr;
};

/** A point light component, like a light bulb. */
struct LightPoint_Component : public ECSComponent<LightPoint_Component> {
	// Light Properties
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);

	// Shadow Properties
	bool m_hasShadow = false;
	bool m_outOfDate = false;
	float m_updateTime = 0.0f;
	int m_shadowSpot = -1;

	// System Identifier
	GL_AB_Index m_lightIndex = nullptr;

};

/** A spot light component, like a flash light. */
struct LightSpot_Component : public ECSComponent<LightSpot_Component> {
	// Light Properties
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	float m_cutoff = 45.0f;
	glm::vec3 m_position = glm::vec3(0.0f);

	// Shadow Properties
	bool m_hasShadow = false;
	bool m_outOfDate = false;
	float m_updateTime = 0.0f;
	int m_shadowSpot = -1;

	// System Identifier
	GL_AB_Index m_lightIndex = nullptr;
};

/** A parallax reflector component, 360 view of a scene. */
struct Reflector_Component : public ECSComponent<Reflector_Component> {
	// Reflector Properties
	CameraBuffer::BufferStructure m_cameraData[6];
	bool m_outOfDate = false;
	float m_updateTime = 0.0f;
	int m_cubeSpot = -1;

	// System Identifier
	GL_AB_Index m_reflectorIndex = nullptr;
};

#endif // GRAPHICS_LIGHTING_COMPONENTS_H
