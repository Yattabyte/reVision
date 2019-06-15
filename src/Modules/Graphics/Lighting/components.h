#pragma once
#ifndef GRAPHICS_LIGHTING_COMPONENTS_H
#define GRAPHICS_LIGHTING_COMPONENTS_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "glm/glm.hpp"


/** A directional light component, emulating the appearance of sun lighting. */
struct LightDirectional_Component : public ECSComponent<LightDirectional_Component> {
	glm::vec3 m_color = glm::vec3(1.0f);
	glm::vec3 m_direction = glm::vec3(0, -1, 0);
	float m_intensity = 1.0f;
	size_t * m_lightIndex = nullptr;
};

/** A directional light shadow component, formatted for 4 parallel split cascaded shadow maps. */
struct LightDirectionalShadow_Component : public ECSComponent<LightDirectionalShadow_Component> {
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	glm::mat4 m_mMatrix = glm::mat4(1.0f);
	size_t * m_shadowIndex = nullptr;
};

/** A point light component, emulating a light bulb like appearance. */
struct LightPoint_Component : public ECSComponent<LightPoint_Component> {
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
	size_t * m_lightIndex = nullptr;
};

/** A point light shadow component, formatted to support using a cubemap for shadows. */
struct LightPointShadow_Component : public ECSComponent<LightPointShadow_Component> {
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = false;
	size_t * m_shadowIndex = nullptr;
};

/** A spot light component, emulating a flash light/spot light. */
struct LightSpot_Component : public ECSComponent<LightSpot_Component> {
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	float m_cutoff = 45.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
	size_t * m_lightIndex = nullptr;
};

/** A spot light shadow component, formatted to support a single shadow map. */
struct LightSpotShadow_Component : public ECSComponent<LightSpotShadow_Component> {
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = false;
	size_t * m_shadowIndex = nullptr;
};

/** Represents an environment map buffer component. */
struct Reflector_Component : public ECSComponent<Reflector_Component> {
	float m_updateTime = 0.0f;
	int m_cubeSpot = 0;
	bool m_outOfDate = false;
	CameraBuffer::BufferStructure m_Cameradata[6];
	size_t * m_reflectorIndex = nullptr;
};

#endif // GRAPHICS_LIGHTING_COMPONENTS_H
