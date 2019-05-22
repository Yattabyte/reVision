#pragma once
#ifndef REFLECTOR_C_H
#define REFLECTOR_C_H

#include "Modules/Graphics/Common/CameraBuffer.h"
#include "Modules/Graphics/Common/FBO_EnvMap.h"
#include "Utilities/ECS/ecsComponent.h"
#include "Utilities/GL/VectorBuffer.h"
#include "Utilities/Transform.h"
#include "glm/glm.hpp"


/** OpenGL buffer for Parallax reflectors.
Formatted to adhere to glsl specifications. */
struct Reflection_Buffer {
	glm::mat4 mMatrix;
	glm::mat4 rotMatrix;
	glm::vec3 BoxCamPos; float padding1;
	glm::vec3 BoxScale; float padding2;
	int CubeSpot; glm::vec3 padding3;
};

/** Represents an environment map buffer component. */
struct Reflector_Component: public ECSComponent<Reflector_Component> {
	VB_Element<Reflection_Buffer> * m_data = nullptr;
	int m_cubeSpot = 0;
	bool m_outOfDate = true;
	float m_updateTime = 0.0f;
	Transform m_transform;
	CameraBuffer::BufferStructure m_Cameradata[6];
};
/** A constructor to aid in creation. */
struct Reflector_Constructor : ECSComponentConstructor<Reflector_Component> {
	// Public (de)Constructors
	Reflector_Constructor(VectorBuffer<Reflection_Buffer> * refElementBuffer, FBO_EnvMap * envmapFBO)
		: m_refElementBuffer(refElementBuffer), m_envmapFBO(envmapFBO) {};


	// Interface Implementation
	inline virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto * component = new Reflector_Component();
		component->m_data = m_refElementBuffer->newElement();
		component->m_data->data->CubeSpot = m_envCount;
		component->m_cubeSpot = m_envCount;
		m_envCount += 6;
		m_envmapFBO->resize(m_envmapFBO->m_size.x, m_envmapFBO->m_size.y, m_envCount);		
		component->m_outOfDate = true;		
		for (int x = 0; x < 6; ++x) {
			component->m_Cameradata[x].Dimensions = m_envmapFBO->m_size;
			component->m_Cameradata[x].FOV = 90.0f;
		}
		return { component, component->ID };
	}


private:
	// Private Attributes
	GLuint m_envCount = 0;
	VectorBuffer<Reflection_Buffer> * m_refElementBuffer = nullptr;
	FBO_EnvMap * m_envmapFBO = nullptr;
};

#endif // REFLECTOR_C_H