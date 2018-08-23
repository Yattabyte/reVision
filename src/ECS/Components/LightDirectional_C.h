#pragma once
#ifndef LIGHTDIRECTIONAL_C_H
#define LIGHTDIRECTIONAL_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/** OpenGL buffer for directional lights.
Formatted to adhere to glsl specifications. */
struct LightDirectional_Buffer {
	glm::vec3 LightColor; float padding1;
	glm::vec3 LightDirection; float padding2;
	float LightIntensity; glm::vec3 padding3;
};

/** A directional light component, emulating the appearance of sun lighting. */
struct LightDirectional_Component : public ECSComponent<LightDirectional_Component> {
	VB_Element<LightDirectional_Buffer> * m_data;
};

/** OpenGL buffer for directional light shadows.
Formatted to adhere to glsl specifications. */
struct LightDirectionalShadow_Buffer {
	#define NUM_CASCADES 4
	glm::mat4 lightV = glm::mat4(1.0f);
	int Shadow_Spot = 0;
	float CascadeEndClipSpace[NUM_CASCADES]; glm::vec3 padding1; // end of scalars, pad by 2
	glm::mat4 lightVP[NUM_CASCADES];
	glm::mat4 inverseVP[NUM_CASCADES]; 
};

/** A directional light shadow component, formatted for 4 parallel split cascaded shadow maps. */
struct LightDirectionalShadow_Component : public ECSComponent<LightDirectionalShadow_Component> {
	float m_updateTime;
	float m_cascadeEnd[5];
	int m_shadowSpot;
	size_t m_visSize[2];
	float m_shadowSize;
	glm::mat4 m_mMatrix = glm::mat4(1.0f);
	VB_Element<LightDirectionalShadow_Buffer> * m_data;
};

#endif // LIGHTDIRECTIONAL_C_H