#pragma once
#ifndef REFLECTOR_C_H
#define REFLECTOR_C_H

#include "ECS\Components\ecsComponent.h"
#include "ECS\Components\Camera_C.h"
#include "ECS\Resources\FBO_EnvMap.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\Transform.h"
#include "glm\glm.hpp"


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
	VB_Element<Reflection_Buffer> * m_data;
	int m_cubeSpot = 0;
	bool m_outOfDate = true;
	float m_updateTime = 0.0f;
	Transform m_transform;
	VB_Element<Camera_Buffer> * m_Cameradata[6];
};
/** A constructor to aid in creation. */
struct Reflector_Constructor : ECSComponentConstructor<Reflector_Component> {
	Reflector_Constructor(VectorBuffer<Camera_Buffer> * camElementBuffer, VectorBuffer<Reflection_Buffer> * refElementBuffer, FBO_EnvMap * envmapFBO)
		: m_camElementBuffer(camElementBuffer), m_refElementBuffer(refElementBuffer), m_envmapFBO(envmapFBO) {};
	// Interface Implementation
	virtual Component_and_ID construct(const std::vector<std::any> & parameters) override {
		auto position = castAny(parameters[0], glm::vec3(0.0f));
		auto orientation = castAny(parameters[1], glm::quat(1, 0, 0, 0));
		auto scale = castAny(parameters[2], glm::vec3(1.0f));
		auto * component = new Reflector_Component();
		component->m_data = m_refElementBuffer->newElement();
		component->m_data->data->CubeSpot = m_envCount;
		component->m_cubeSpot = m_envCount;
		m_envCount += 6;
		m_envmapFBO->resize(m_envmapFBO->m_size.x, m_envmapFBO->m_size.y, m_envCount);
		const float largest = pow(std::max(std::max(scale.x, scale.y), scale.z), 2.0f);
		component->m_transform = Transform(position, orientation, scale);
		component->m_data->data->mMatrix = component->m_transform.m_modelMatrix;
		component->m_data->data->rotMatrix = glm::inverse(glm::toMat4(component->m_transform.m_orientation));
		component->m_data->data->BoxCamPos = position;
		component->m_data->data->BoxScale = component->m_transform.m_scale;
		component->m_outOfDate = true;
		const glm::mat4 vMatrices[6] = {
			glm::lookAt(position, position + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0)),
			glm::lookAt(position, position + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0)),
			glm::lookAt(position, position + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1)),
			glm::lookAt(position, position + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)),
			glm::lookAt(position, position + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0)),
			glm::lookAt(position, position + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0))
		};
		const glm::mat4 pMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, largest);
		const glm::mat4 pMatrix_Inverse = glm::inverse(pMatrix);
		for (int x = 0; x < 6; ++x) {
			component->m_Cameradata[x] = m_camElementBuffer->newElement();
			component->m_Cameradata[x]->data->NearPlane = 0.01f;
			component->m_Cameradata[x]->data->FarPlane = largest;
			component->m_Cameradata[x]->data->EyePosition = position;
			component->m_Cameradata[x]->data->Dimensions = m_envmapFBO->m_size;
			component->m_Cameradata[x]->data->FOV = 90.0f;
			component->m_Cameradata[x]->data->Gamma = 1.0f;
			component->m_Cameradata[x]->data->pMatrix = pMatrix;
			component->m_Cameradata[x]->data->pMatrix_Inverse = pMatrix_Inverse;
			component->m_Cameradata[x]->data->vMatrix = vMatrices[x];
			component->m_Cameradata[x]->data->vMatrix_Inverse = glm::inverse(vMatrices[x]);
		}
		return { component, component->ID };
	}
	GLuint m_envCount = 0;
	VectorBuffer<Camera_Buffer> * m_camElementBuffer;
	VectorBuffer<Reflection_Buffer> * m_refElementBuffer;
	FBO_EnvMap * m_envmapFBO;
};

#endif // REFLECTOR_C_H