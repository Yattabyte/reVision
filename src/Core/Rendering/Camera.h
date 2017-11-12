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
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"

using namespace glm;

struct Camera_Buffer
{
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition; float padding;
	vec2 Dimensions;
	float NearPlane;
	float FarPlane;
	float FOV;

	Camera_Buffer() {
		pMatrix = mat4(1.0f);
		vMatrix = mat4(1.0f);
		pMatrix_Inverse = mat4(1.0f);
		vMatrix_Inverse = mat4(1.0f);
		EyePosition = vec3(0.0f);
		Dimensions = vec2(1.0f);
		NearPlane = 0.01f;
		FarPlane = 1.0f;
		FOV = 1.0f;
	}
};

class Camera 
{
public:
	/*************
	----Common----
	*************/

	DELTA_CORE_API ~Camera();
	DELTA_CORE_API Camera(const vec3 &position = vec3(), const vec2 &size = vec2(1.0f), const float &near_plane = 0.01f, const float &far_plane = 1.0f, const float &horizontal_FOV = 90.0f);
	DELTA_CORE_API Camera(Camera const &other);
	DELTA_CORE_API void operator=(Camera const&other);

	/***********************
	----Camera Functions----
	***********************/

	// Make the current camera active
	// Exposes this camera's attribute buffer to all shaders at spot 1
	DELTA_CORE_API void Bind();


	/*************************
	----Variable Functions----
	*************************/

	// Sets the position of the camera
	void setPosition(const vec3 &p) { m_cameraBuffer.EyePosition = p; };
	// Sets the orientation of the camera
	void setOrientation(const quat &q) { m_orientation = q; };
	// Sets the dimensions of the camera
	void setDimensions(const vec2 &d) { m_cameraBuffer.Dimensions = d; };
	// Sets the closest point the camera can see
	void setNearPlane(const float & n) { m_cameraBuffer.NearPlane = n; };
	// Sets the furthest point the camera can see (aka the draw distance)
	void setFarPlane(const float &f) { m_cameraBuffer.FarPlane = f; };
	// Sets the horizontal FOV of the camera
	void setHorizontalFOV(const float &fov) { m_cameraBuffer.FOV = fov; };
	// Updates the camera's state on the GPU
	// All matrix updates performed here
	DELTA_CORE_API void Update();
	// Returns a copy of this camera's data buffer
	DELTA_CORE_API Camera_Buffer getCameraBuffer() const;


private:
	/****************
	----Variables----
	****************/

	GLuint ssboCameraID;
	Camera_Buffer m_cameraBuffer;
	quat m_orientation;
};

#endif // CAMERA