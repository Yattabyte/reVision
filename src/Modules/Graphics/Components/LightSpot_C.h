#pragma once
#ifndef LIGHTSPOT_C_H
#define LIGHTSPOT_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Modules\Graphics\Common\FBO_Shadow_Spot.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


//////////////////
///-SPOT LIGHT-///
//////////////////
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
	VB_Element<LightSpot_Buffer> * m_data = nullptr;
	float m_radius = 0.0f;
};
/** A constructor to aid in creation. */
struct LightSpot_Constructor : ECSComponentConstructor<LightSpot_Component> {
	// Public (de)Constructors
	LightSpot_Constructor(VectorBuffer<LightSpot_Buffer> * elementBuffer)
		: m_elementBuffer(elementBuffer) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto color = castAny(parameters[0], glm::vec3(1.0f));
		auto intensity = castAny(parameters[1], 1.0f);
		auto radius = castAny(parameters[2], 1.0f);
		auto cutoff = castAny(parameters[3], 45.0f);
		auto * component = new LightSpot_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->LightColor = color;
		component->m_data->data->LightIntensity = intensity;
		component->m_data->data->LightRadius = radius;
		component->m_data->data->LightCutoff = cosf(glm::radians(cutoff));
		component->m_radius = radius;	
		return { component, component->ID };
	}


private:
	// Private Attributes
	VectorBuffer<LightSpot_Buffer> * m_elementBuffer = nullptr;
};

///////////////////
///-SPOT SHADOW-///
///////////////////
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
	glm::vec3 m_position = glm::vec3(0.0f);
	float m_radius = 0.0f;
	float m_cutoff = 45.0f;
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = true;
	VB_Element<LightSpotShadow_Buffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct LightSpotShadow_Constructor : ECSComponentConstructor<LightSpotShadow_Component> {
	// Public (de)Constructors
	LightSpotShadow_Constructor(VectorBuffer<LightSpotShadow_Buffer> * elementBuffer, FBO_Shadow_Spot * shadowFBO)
		: m_elementBuffer(elementBuffer), m_shadowFBO(shadowFBO) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new LightSpotShadow_Component();
		component->m_radius = castAny(parameters[0], 1.0f);
		component->m_cutoff = castAny(parameters[1], 45.0f);
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->Shadow_Spot = m_shadowCount;
		component->m_updateTime = 0.0f;
		component->m_shadowSpot = m_shadowCount;
		component->m_outOfDate = true;
		m_shadowCount += 2;
		m_shadowFBO->resize(m_shadowFBO->m_size, m_shadowCount);		
		return { component, component->ID };
	}


private:
	// Private Attributes
	GLuint m_shadowCount = 0;
	VectorBuffer<LightSpotShadow_Buffer> * m_elementBuffer = nullptr;
	FBO_Shadow_Spot * m_shadowFBO = nullptr;
};

#endif // LIGHTSPOT_C_H