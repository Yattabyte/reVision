#pragma once
#ifndef CAMERA_C_H
#define CAMERA_C_H

#include "Utilities\ECS\ecsComponent.h"
#include "Utilities\GL\VectorBuffer.h"
#include "glm\glm.hpp"


/** OpenGL buffer for point lights.
Formatted to adhere to glsl specifications. */
struct Camera_Buffer {
	#define CAMERA_NEAR_PLANE 0.01f
	glm::mat4 pMatrix;
	glm::mat4 vMatrix;
	glm::mat4 pMatrix_Inverse;
	glm::mat4 vMatrix_Inverse;
	glm::vec3 EyePosition; float padding1;
	glm::vec2 Dimensions;
	float FarPlane; float FOV; // These 2 values are padded out unless shader uses "Dimensions" as vec4
};

/** A camera component. */
struct Camera_Component : public ECSComponent<Camera_Component> {
	VB_Element<Camera_Buffer> * m_data = nullptr;
};

#endif // CAMERA_C_H