#pragma once
#ifndef LIGHTPOINT_C_H
#define LIGHTPOINT_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/** OpenGL buffer for point lights.
Formatted to adhere to glsl specifications. */
struct LightPoint_Buffer {
	glm::mat4 mMatrix;
	glm::vec3 LightColor; float padding1;
	glm::vec3 LightPosition; float padding2;
	float LightIntensity;
	float LightRadius; glm::vec2 padding3;
};

/** A point light component, emulating a light bulb like appearance. */
struct LightPoint_Component : public ECSComponent<LightPoint_Component> {
	VB_Element<LightPoint_Buffer> * m_data;
};

/** OpenGL buffer for point light shadows.
Formatted to adhere to glsl specifications. */
struct LightPointShadow_Buffer {
	glm::mat4 lightV;
	glm::mat4 lightPV[6];
	glm::mat4 inversePV[6];
	int Shadow_Spot; glm::vec3 padding1;
};

/** A point light shadow component, formatted to support using a cubemap for shadows. */
struct LightPointShadow_Component : public ECSComponent<LightPointShadow_Component> {
	float m_updateTime;
	int m_shadowSpot;
	bool m_outOfDate;
	VB_Element<LightPointShadow_Buffer> * m_data;
};

#endif // LIGHTPOINT_C_H