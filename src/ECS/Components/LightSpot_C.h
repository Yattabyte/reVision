#pragma once
#ifndef LIGHTSPOT_C_H
#define LIGHTSPOT_C_H

#include "ECS\Components\ecsComponent.h"
#include "Modules\Graphics\Common\FBO_Shadow_Spot.h"
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
	VB_Element<LightSpot_Buffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct LightSpot_Constructor : ECSComponentConstructor<LightSpot_Component> {
	LightSpot_Constructor(VectorBuffer<LightSpot_Buffer> * elementBuffer)
		: m_elementBuffer(elementBuffer) {};
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto color = castAny(parameters[0], glm::vec3(1.0f));
		auto intensity = castAny(parameters[1], 1.0f);
		auto radius = castAny(parameters[2], 1.0f);
		auto cutoff = castAny(parameters[3], 45.0f);
		auto position = castAny(parameters[4], glm::vec3(0.0f));
		auto * component = new LightSpot_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->LightColor = color;
		component->m_data->data->LightIntensity = intensity;
		component->m_data->data->LightRadius = radius;
		component->m_data->data->LightCutoff = cosf(glm::radians(cutoff));
		component->m_data->data->LightPosition = position;
		component->m_data->data->LightDirection = glm::vec3(1, 0, 0);
		const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 scl = glm::scale(glm::mat4(1.0f), glm::vec3(radius*radius)*1.1f);
		component->m_data->data->mMatrix = (trans)* scl;
		return { component, component->ID };
	}
	VectorBuffer<LightSpot_Buffer> * m_elementBuffer = nullptr;
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
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	bool m_outOfDate = true;
	VB_Element<LightSpotShadow_Buffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct LightSpotShadow_Constructor : ECSComponentConstructor<LightSpotShadow_Component> {
	LightSpotShadow_Constructor(VectorBuffer<LightSpotShadow_Buffer> * elementBuffer, FBO_Shadow_Spot * shadowFBO)
		: m_elementBuffer(elementBuffer), m_shadowFBO(shadowFBO) {};
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto radius = castAny(parameters[0], 1.0f);
		auto cutoff = castAny(parameters[1], 45.0f);
		auto position = castAny(parameters[2], glm::vec3(0.0f));
		auto * component = new LightSpotShadow_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->Shadow_Spot = m_shadowCount;
		component->m_updateTime = 0.0f;
		component->m_shadowSpot = m_shadowCount;
		component->m_outOfDate = true;
		m_shadowCount += 2;
		m_shadowFBO->resize(m_shadowFBO->m_size, m_shadowCount);
		const glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
		const glm::mat4 final = glm::inverse(trans * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0)));
		const float verticalRad = 2.0f * atanf(tanf(glm::radians(cutoff * 2) / 2.0f));
		const glm::mat4 perspective = glm::perspective(verticalRad, 1.0f, 0.01f, radius*radius);
		component->m_data->data->lightV = final;
		component->m_data->data->lightPV = perspective * final;
		component->m_data->data->inversePV = glm::inverse(perspective * final);
		return { component, component->ID };
	}
	GLuint m_shadowCount = 0;
	VectorBuffer<LightSpotShadow_Buffer> * m_elementBuffer = nullptr;
	FBO_Shadow_Spot * m_shadowFBO = nullptr;
};

#endif // LIGHTSPOT_C_H