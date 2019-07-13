#pragma once
#ifndef GRAPHICS_LIGHTING_COMPONENTS_H
#define GRAPHICS_LIGHTING_COMPONENTS_H

#include "Modules/Graphics/Common/Camera.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include <memory>

#define NUM_CASCADES 4

/***/
struct Shadow_Component : public ECSComponent<Shadow_Component> {
	// Shadow Properties
	int m_shadowSpot = -1;
};

/***/
struct LightColor_Component : public ECSComponent<LightColor_Component> {
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
};

/***/
struct LightRadius_Component : public ECSComponent<LightRadius_Component> {
	float m_radius = 1.0f;
};

/***/
struct LightCutoff_Component : public ECSComponent<LightCutoff_Component> {
	float m_cutoff = 45.0f;
};

/** A directional light component, like a sun. */
struct LightDirectional_Component : public ECSComponent<LightDirectional_Component> {
	glm::mat4 m_pvMatrices[NUM_CASCADES] = { glm::mat4(1.0f),glm::mat4(1.0f), glm::mat4(1.0f), glm::mat4(1.0f) };
	float m_cascadeEnds[NUM_CASCADES] = { 1.0f, 1.0f, 1.0f, 1.0f };
};

/** A point light component, like a light bulb. */
struct LightPoint_Component : public ECSComponent<LightPoint_Component> {
};

/** A spot light component, like a flash light. */
struct LightSpot_Component : public ECSComponent<LightSpot_Component> {
};

/** A parallax reflector component, 360 view of a scene. */
struct Reflector_Component : public ECSComponent<Reflector_Component> {
	// Reflector Properties
	float m_updateTime = 0.0f;
	int m_cubeSpot = -1;
};

#endif // GRAPHICS_LIGHTING_COMPONENTS_H
