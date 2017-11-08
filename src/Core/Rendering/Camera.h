/*
	Camera

	- An element that defines where and how a scene should be viewed
*/

#pragma once
#ifndef CAMERA
#define CAMERA
#ifdef	DT_CORE_EXPORT
#define DELTA_CORE_API __declspec(dllexport)
#else
#define	DELTA_CORE_API __declspec(dllimport)
#endif

#include "GL\glew.h"
#include "glm\common.hpp"
#include "glm\glm.hpp"

using namespace glm;

struct Camera_Buffer
{
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 Dimensions;
	float DrawDistance;
	float FOV;

	Camera_Buffer() {
		pMatrix = mat4(1.0f);
		vMatrix = mat4(1.0f);
		EyePosition = vec3(0.0f);
		Dimensions = vec2(1.0f);
		DrawDistance = 1.0f;
		FOV = 1.0f;
	}
};

class Camera 
{
public:
	// Destructs the camera
	DELTA_CORE_API ~Camera();
	// Constructs the camera
	DELTA_CORE_API Camera(const vec3 &position = vec3(), const vec2 &size = vec2(1.0f), const float &draw_distance = 1.0f, const float &horizontal_FOV = 90.0f);
	// Creates this camera and makes it a copy from another camera @other
	DELTA_CORE_API Camera(Camera const &other);
	// Copies another camera @other
	DELTA_CORE_API void operator=(Camera const&other);
	// Sets the position of the camera
	// Doesn't update GPU state
	DELTA_CORE_API void setPosition(const vec3 &p);
	// Sets the dimensions of the camera
	// Doesn't update GPU state
	DELTA_CORE_API void setDimensions(const vec2 &d);
	// Sets the draw distance of the camera
	// Doesn't update GPU state
	DELTA_CORE_API void setDrawDistance(const float &d);
	// Sets the horizontal FOV of the camera
	// Doesn't update GPU state
	DELTA_CORE_API void setHorizontalFOV(const float &fov);
	// Returns a copy of this camera's data buffer
	DELTA_CORE_API Camera_Buffer getCameraBuffer() const;
	// Updates the camera's state on the GPU
	// All matrix updates performed here
	DELTA_CORE_API void Update();
	// Make the current camera active
	// Exposes this camera's attribute buffer to all shaders at spot 1
	DELTA_CORE_API void Bind();

private:
	GLuint ssboCameraID;
	Camera_Buffer ssboCameraBuffer;
};

#endif // CAMERA