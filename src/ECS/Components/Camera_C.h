#pragma once
#ifndef CAMERA_C_H
#define CAMERA_C_H

#include "ECS\Components\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/** OpenGL buffer for point lights.
Formatted to adhere to glsl specifications. */
struct Camera_Buffer {
	glm::mat4 pMatrix;
	glm::mat4 vMatrix;
	glm::mat4 pMatrix_Inverse;
	glm::mat4 vMatrix_Inverse;
	glm::vec3 EyePosition; float padding1;
	glm::vec2 Dimensions;
	float NearPlane;
	float FarPlane;
	float FOV;
	float Gamma; glm::vec2 padding2;
};

/** A camera component. */
struct Camera_Component : public ECSComponent<Camera_Component> {
	VB_Element<Camera_Buffer> * m_data;
};

#endif // CAMERA_C_H