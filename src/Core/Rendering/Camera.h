/*
	Camera

	- An element that defines where and how a scene should be viewed
*/

#pragma once
#ifndef CAMERA
#define CAMERA
#ifdef	ENGINE_EXPORT
#define DT_ENGINE_API __declspec(dllexport)
#else
#define	DT_ENGINE_API __declspec(dllimport)
#endif

#include "Rendering\Visibility_Token.h"
#include "Utilities\Frustum.h"
#include "GL\glew.h"
#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"
#include <shared_mutex>

using namespace std;
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
	float Gamma;

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
		Gamma = 1.0f;
	}
};

class DT_ENGINE_API Camera
{
public:
	/*************
	----Common----
	*************/

	~Camera();
	Camera(const vec3 &position = vec3(), const vec2 &size = vec2(1.0f), const float &near_plane = 0.01f, const float &far_plane = 10.0f, const float &horizontal_FOV = 90.0f);
	Camera(Camera const &other);
	void operator=(Camera const&other);


	/***********************
	----Camera Functions----
	***********************/

	// Make the current camera active
	// Exposes this camera's attribute buffer to all shaders at spot 1
	void Bind();


	/*************************
	----Variable Functions----
	*************************/

	// Sets the position of the camera
	void setPosition(const vec3 &p) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.EyePosition = p; };
	// Sets the orientation of the camera
	void setOrientation(const quat &q) { lock_guard<shared_mutex> wguard(data_mutex); m_orientation = q; };
	// Sets the dimensions of the camera
	void setDimensions(const vec2 &d) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.Dimensions = d; };
	// Sets the closest point the camera can see
	void setNearPlane(const float & n) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.NearPlane = n; };
	// Sets the furthest point the camera can see (aka the draw distance)
	void setFarPlane(const float &f) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.FarPlane = f; };
	// Sets the horizontal FOV of the camera
	void setHorizontalFOV(const float &fov) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.FOV = fov; };
	// Sets the gamma of the camera
	void setGamma(const float &gamma) { lock_guard<shared_mutex> wguard(data_mutex); m_cameraBuffer.Gamma = gamma; };
	// Return a reference to the visibility token
	Visibility_Token &GetVisibilityToken() { return m_vistoken; };
	// Returns a copy of this camera's data buffer
	Camera_Buffer getCameraBuffer() const { return m_cameraBuffer; };
	// Sets the flag for whether or not this camera should be rendered from
	void enableRendering(const bool &b) { render_enabled = b; };
	// Retrieve the flag for whether or not this camera is active for rendering
	bool shouldRender() const { return render_enabled; }
	// Returns a copy of the viewing frustum
	Frustum getFrustum() const { return m_frustum; };
	// Returns the mutex
	shared_mutex &getDataMutex() const { return data_mutex; };
	// Updates the camera's state on the GPU
	// All matrix updates performed here
	void Update();


private:
	/****************
	----Variables----
	****************/

	mutable shared_mutex data_mutex;
	GLuint ssboCameraID;
	Camera_Buffer m_cameraBuffer;
	quat m_orientation;
	Visibility_Token m_vistoken;
	Frustum m_frustum;
	bool render_enabled;
};

#endif // CAMERA