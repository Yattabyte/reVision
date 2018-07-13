#pragma once
#ifndef ECS_DEFINES_H
#define ECS_DEFINES_H
#include "glm\glm.hpp"
#include <cstring>
#include <map>
#include <vector>


static enum ECS_MESSAGE_COMMAND_TYPES {
	// SPATIAL
	SET_POSITION,
	SET_ORIENTATION,
	SET_SCALE,
	SET_TRANSFORM,

	// LIGHTING
	SET_LIGHT_COLOR,
	SET_LIGHT_INTENSITY,
	SET_LIGHT_RADIUS,
	SET_LIGHT_CUTOFF,

	// Reflectors
	SET_REFLECTOR_RADIUS,
};

/** A means for sharing camera data with both the program and the GPU in one data structure. */
struct Camera_Buffer {
	glm::mat4 pMatrix = glm::mat4(1.0f);
	glm::mat4 vMatrix = glm::mat4(1.0f);
	glm::mat4 pMatrix_Inverse = glm::mat4(1.0f);
	glm::mat4 vMatrix_Inverse = glm::mat4(1.0f);
	glm::vec3 EyePosition = glm::vec3(0.0f); float padding;
	glm::vec2 Dimensions = glm::vec2(1.0f);
	float NearPlane = 0.01f;
	float FarPlane = 1.0f;
	float FOV = 1.0f;
	float Gamma = 1.0f;
};

#endif // ECS_DEFINES_H