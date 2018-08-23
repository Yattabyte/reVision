#pragma once
#ifndef LIGHTSPOT_C_H
#define LIGHTSPOT_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/** OpenGL buffer for spot lights.
Formatted to adhere to glsl specifications. */
struct LightSpot_Buffer {
	glm::mat4 mMatrix;
	glm::vec3 LightColor; float padding1;
	glm::vec3 LightPosition; float padding2;
	glm::vec3 LightDirection; float padding3;
	float LightIntensity;
	float LightRadius;
	float LightCutoff; float padding4;	
};

/** A spot light component, emulating a flash light/spot light. */
struct LightSpot_Component : public ECSComponent<LightSpot_Component> {
	VB_Element<LightSpot_Buffer> * m_data;
};

/** OpenGL buffer for spot light shadows.
Formatted to adhere to glsl specifications. */
struct LightSpotShadow_Buffer {
	glm::mat4 lightV;
	glm::mat4 lightPV;
	glm::mat4 inversePV;
	int Shadow_Spot; glm::vec3 padding1;
};

/** A spot light shadow component, formatted to support a single shadow map. */
struct LightSpotShadow_Component : public ECSComponent<LightSpotShadow_Component> {
	float m_updateTime;
	int m_shadowSpot;
	bool m_outOfDate;
	VB_Element<LightSpotShadow_Buffer> * m_data;
};

#endif // LIGHTSPOT_C_H