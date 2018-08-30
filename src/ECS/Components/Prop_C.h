#pragma once
#ifndef MODEL_C_H
#define MODEL_C_H

#include "ECS\Components\ecsComponent.h"
#include "Assets\Asset_Model.h"
#include "Utilities\GL\VectorBuffer.h"
#include "Utilities\Transform.h"


/** OpenGL buffer for basic props.
Formatted to adhere to glsl specifications. */
struct Prop_Buffer {
	GLuint materialID; glm::vec3 padding1;
	glm::mat4 mMatrix;
	glm::mat4 bBoxMatrix;
};

/** A prop component. 
@note		On its own it provides no support for animation. */
struct Prop_Component : public ECSComponent<Prop_Component> {
	Shared_Asset_Model m_model;
	Transform m_transform;
	VB_Element<Prop_Buffer> * m_data;
};

#endif // MODEL_C_H