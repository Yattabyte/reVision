#pragma once
#ifndef LIGHTPOINT_C_H
#define LIGHTPOINT_C_H

#include "Modules/Graphics/Common/FBO_Shadow_Point.h"
#include "Utilities/ECS/ecsComponent.h"
#include "Utilities/GL/VectorBuffer.h"
#include "glm/glm.hpp"


///////////////////
///-POINT LIGHT-///
///////////////////
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
	VB_Element<LightPoint_Buffer> * m_data = nullptr;
	float m_radius = 0.0f;
};
/** A constructor to aid in creation. */
struct LightPoint_Constructor : ECSComponentConstructor<LightPoint_Component> {
	// Public (de)Constructors
	LightPoint_Constructor(VectorBuffer<LightPoint_Buffer> * elementBuffer) 
		: m_elementBuffer(elementBuffer) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto color = castAny(parameters[0], glm::vec3(1.0f));
		auto intensity = castAny(parameters[1], 1.0f);
		auto radius = castAny(parameters[2], 1.0f);
		auto * component = new LightPoint_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->LightColor = color;
		component->m_data->data->LightIntensity = intensity;
		component->m_data->data->LightRadius = radius;
		component->m_radius = radius;
		return { component, component->ID };
	}


private:
	// Private Attributes
	VectorBuffer<LightPoint_Buffer> * m_elementBuffer = nullptr;
};

////////////////////
///-POINT SHADOW-///
////////////////////
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
	float m_radius = 0.0f;
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = true;
	glm::vec3 m_position = glm::vec3(0.0f);
	VB_Element<LightPointShadow_Buffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct LightPointShadow_Constructor : ECSComponentConstructor<LightPointShadow_Component> {
	// Public (de)Constructors
	LightPointShadow_Constructor(VectorBuffer<LightPointShadow_Buffer> * elementBuffer, FBO_Shadow_Point * shadowFBO)
		: m_elementBuffer(elementBuffer), m_shadowFBO(shadowFBO) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new LightPointShadow_Component();
		component->m_radius = castAny(parameters[0], 1.0f);
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->Shadow_Spot = m_shadowCount;
		component->m_updateTime = 0.0f;
		component->m_shadowSpot = m_shadowCount;
		component->m_outOfDate = true;
		m_shadowCount += 12;
		m_shadowFBO->resize(m_shadowFBO->m_size, m_shadowCount);	
		return { component, component->ID };
	}


private:
	// Private Attributes
	GLuint m_shadowCount = 0;
	VectorBuffer<LightPointShadow_Buffer> * m_elementBuffer = nullptr;
	FBO_Shadow_Point * m_shadowFBO = nullptr;
};
#endif // LIGHTPOINT_C_H