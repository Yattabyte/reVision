#pragma once
#ifndef LIGHTDIRECTIONAL_C_H
#define LIGHTDIRECTIONAL_C_H

#include "ECS\Components\ecsComponent.h"
#include "Modules\Graphics\Common\FBO_Shadow_Directional.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/////////////////////////
///-DIRECTIONAL LIGHT-///
/////////////////////////
/** OpenGL buffer for directional lights.
Formatted to adhere to glsl specifications. */
struct LightDirectional_Buffer {
	glm::vec3 LightColor; float padding1;
	glm::vec3 LightDirection; float padding2;
	float LightIntensity; glm::vec3 padding3;
};
/** A directional light component, emulating the appearance of sun lighting. */
struct LightDirectional_Component : public ECSComponent<LightDirectional_Component> {
	VB_Element<LightDirectional_Buffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct LightDirectional_Constructor : ECSComponentConstructor<LightDirectional_Component> {
	// Public (de)Constructors
	LightDirectional_Constructor(VectorBuffer<LightDirectional_Buffer> * const elementBuffer) : m_elementBuffer(elementBuffer) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto color = castAny(parameters[0], glm::vec3(1.0f));
		auto intensity = castAny(parameters[1], 1.0f);
		auto * component = new LightDirectional_Component();
		component->m_data = m_elementBuffer->newElement();
		component->m_data->data->LightColor = color;
		component->m_data->data->LightIntensity = intensity;			
		return { component, component->ID };
	}


private:
	// Private Attributes
	VectorBuffer<LightDirectional_Buffer> * m_elementBuffer = nullptr;
};

//////////////////////////
///-DIRECTIONAL SHADOW-///
//////////////////////////
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
	float m_updateTime = 0.0f;
	int m_shadowSpot = 0;
	size_t m_visSize[2];
	float m_shadowSize = 0.0f;
	glm::mat4 m_mMatrix = glm::mat4(1.0f);
	glm::quat m_orientation = glm::quat(1, 0, 0, 0);
	VB_Element<LightDirectionalShadow_Buffer> * m_data = nullptr;
};
/** A constructor to aid in creation. */
struct LightDirectionalShadow_Constructor : ECSComponentConstructor<LightDirectionalShadow_Component> {
	// Public (de)Constructors
	LightDirectionalShadow_Constructor(VectorBuffer<LightDirectionalShadow_Buffer> * const elementBuffer, FBO_Shadow_Directional * const shadowFBO) 
		: m_elementBuffer(elementBuffer), m_shadowFBO(shadowFBO) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new LightDirectionalShadow_Component();
		component->m_data = m_elementBuffer->newElement();		
		component->m_data->data->Shadow_Spot = m_shadowCount;
		component->m_updateTime = 0.0f;
		component->m_shadowSpot = m_shadowCount;
		m_shadowCount += 4;
		m_shadowFBO->resize(m_shadowFBO->m_size, m_shadowCount);
		// Default Values
		component->m_data->data->lightV = glm::mat4(1.0f);
		for (int x = 0; x < NUM_CASCADES; ++x) {
			component->m_data->data->lightVP[x] = glm::mat4(1.0f);
			component->m_data->data->inverseVP[x] = glm::inverse(glm::mat4(1.0f));
		}
		return { component, component->ID };
	}


private:
	// Private Attributes
	GLuint m_shadowCount = 0;
	VectorBuffer<LightDirectionalShadow_Buffer> * m_elementBuffer = nullptr;
	FBO_Shadow_Directional * m_shadowFBO = nullptr;
};

#endif // LIGHTDIRECTIONAL_C_H