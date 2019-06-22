#pragma once
#ifndef GRAPHICS_LIGHTING_COMPONENTS_H
#define GRAPHICS_LIGHTING_COMPONENTS_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/GL/GL_ArrayBuffer.h"
#include "glm/glm.hpp"
#include <memory>

#define NUM_CASCADES 4


/** A directional light component, emulating the appearance of sun lighting. */
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
	GL_AB_Index m_lightIndex = nullptr;
};

/** A point light component, emulating a light bulb like appearance. */
struct LightPoint_Component : public ECSComponent<LightPoint_Component> {
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
	GL_AB_Index m_lightIndex = nullptr;

	/** OpenGL buffer for point lights. */
	struct Point_Buffer {
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		float LightIntensity;
		float LightRadius; glm::vec2 padding3;
	};
};

/** A point light shadow component, formatted to support using a cubemap for shadows. */
struct LightPointShadow_Component : public ECSComponent<LightPointShadow_Component> {
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = false;
	GL_AB_Index m_shadowIndex = nullptr;
	
	/** OpenGL buffer for point light shadows. */
	struct Point_Shadow_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV[6];
		glm::mat4 inversePV[6];
		int Shadow_Spot; glm::vec3 padding1;
	};
};

/** A spot light component, emulating a flash light/spot light. */
struct LightSpot_Component : public ECSComponent<LightSpot_Component> {
	glm::vec3 m_color = glm::vec3(1.0f);
	float m_intensity = 1.0f;
	float m_radius = 1.0f;
	float m_cutoff = 45.0f;
	glm::vec3 m_position = glm::vec3(0.0f);
	GL_AB_Index m_lightIndex = nullptr;
};

/** A spot light shadow component, formatted to support a single shadow map. */
struct LightSpotShadow_Component : public ECSComponent<LightSpotShadow_Component> {
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = false;
	GL_AB_Index m_shadowIndex = nullptr;
};

/** Represents an environment map buffer component. */
struct Reflector_Component : public ECSComponent<Reflector_Component> {
	float m_updateTime = 0.0f;
	int m_cubeSpot = 0;
	bool m_outOfDate = false;
	CameraBuffer::BufferStructure m_cameraData[6];
	GL_AB_Index m_reflectorIndex = nullptr;
};

#endif // GRAPHICS_LIGHTING_COMPONENTS_H
