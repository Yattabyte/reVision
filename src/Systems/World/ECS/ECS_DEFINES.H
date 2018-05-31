#pragma once
#ifndef ECS_DEFINES
#define ECS_DEFINES
#include "glm\glm.hpp"
#include <cstring>
#include <map>
#include <vector>

using namespace glm;

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
	mat4 pMatrix = mat4(1.0f);
	mat4 vMatrix = mat4(1.0f);
	mat4 pMatrix_Inverse = mat4(1.0f);
	mat4 vMatrix_Inverse = mat4(1.0f);
	vec3 EyePosition = vec3(0.0f); float padding;
	vec2 Dimensions = vec2(1.0f);
	float NearPlane = 0.01f;
	float FarPlane = 1.0f;
	float FOV = 1.0f;
	float Gamma = 1.0f;
};

#endif // ECS_DEFINES