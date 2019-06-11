#pragma once
#ifndef GRAPHICS_LIGHTING_COMPONENTS_H
#define GRAPHICS_LIGHTING_COMPONENTS_H

#include "Assets/Mesh.h"
#include "Assets/Model.h"
#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/FBO_EnvMap.h"
#include "Modules/World/ECS/ecsComponent.h"
#include "Utilities/Transform.h"
#include "Utilities/GL/VectorBuffer.h"
#include "glm/glm.hpp"


/** A directional light component, emulating the appearance of sun lighting. */
struct LightDirectional_Component : public ECSComponent<LightDirectional_Component> {
	glm::vec3 m_color = glm::vec3(1.0f);
	glm::vec3 m_direction = (glm::vec3(0, -1, 0));
	float m_intensity = 1.0f;
	size_t * m_lightIndex = nullptr;
};

/** A directional light shadow component, formatted for 4 parallel split cascaded shadow maps. */
struct LightDirectionalShadow_Component : public ECSComponent<LightDirectionalShadow_Component> {
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	size_t m_visSize[2];
	float m_shadowSize = 512.0f;
	glm::mat4 m_mMatrix = glm::mat4(1.0f);
	glm::quat m_orientation = glm::quat(1, 0, 0, 0);
	size_t * m_shadowIndex = nullptr;
};

/** A point light component, emulating a light bulb like appearance. */
struct LightPoint_Component : public ECSComponent<LightPoint_Component> {
	/** OpenGL buffer for point lights. */
	struct GL_Buffer {
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		float LightIntensity;
		float LightRadius; glm::vec2 padding3;
	};

	glm::vec3 LightColor = glm::vec3(1.0f);
	float LightIntensity = 1.0f;
	float m_radius = 1.0f;
	VB_Element<GL_Buffer> * m_data = nullptr;
};

/** A point light shadow component, formatted to support using a cubemap for shadows. */
struct LightPointShadow_Component : public ECSComponent<LightPointShadow_Component> {
	/** OpenGL buffer for point light shadows. */
	struct GL_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV[6];
		glm::mat4 inversePV[6];
		int Shadow_Spot; glm::vec3 padding1;
	};

	float m_radius = 1.0f;
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = true;
	glm::vec3 m_position = glm::vec3(0.0f);
	VB_Element<GL_Buffer> * m_data = nullptr;
};

/** A spot light component, emulating a flash light/spot light. */
struct LightSpot_Component : public ECSComponent<LightSpot_Component> {
	/** OpenGL buffer for spot lights. */
	struct GL_Buffer {
		glm::mat4 mMatrix;
		glm::vec3 LightColor; float padding1;
		glm::vec3 LightPosition; float padding2;
		glm::vec3 LightDirection; float padding3;
		float LightIntensity;
		float LightRadius;
		float LightCutoff; float padding4;
	};

	glm::vec3 LightColor = glm::vec3(1.0f);
	float LightIntensity = 1.0f;
	float m_radius = 1.0f;
	float m_cutoff = 90.0f;
	VB_Element<GL_Buffer> * m_data = nullptr;
};

/** A spot light shadow component, formatted to support a single shadow map. */
struct LightSpotShadow_Component : public ECSComponent<LightSpotShadow_Component> {
	/** OpenGL buffer for spot light shadows. */
	struct GL_Buffer {
		glm::mat4 lightV;
		glm::mat4 lightPV;
		glm::mat4 inversePV;
		int Shadow_Spot; glm::vec3 padding1;
	};

	glm::vec3 m_position = glm::vec3(0.0f);
	float m_radius = 1.0f;
	float m_cutoff = 90.0f;
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = true;
	VB_Element<GL_Buffer> * m_data = nullptr;
};

/** Represents an environment map buffer component. */
struct Reflector_Component : public ECSComponent<Reflector_Component> {
	/** OpenGL buffer for Parallax reflectors. */
	struct GL_Buffer {
		glm::mat4 mMatrix;
		glm::mat4 rotMatrix;
		glm::vec3 BoxCamPos; float padding1;
		glm::vec3 BoxScale; float padding2;
		int CubeSpot; glm::vec3 padding3;
	};

	VB_Element<GL_Buffer> * m_data = nullptr;
	int m_cubeSpot = 0;
	bool m_outOfDate = true;
	float m_updateTime = 0.0f;
	Transform m_transform;
	CameraBuffer::BufferStructure m_Cameradata[6];
};

#endif // GRAPHICS_LIGHTING_COMPONENTS_H
