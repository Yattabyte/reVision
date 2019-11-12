#pragma once
#ifndef CAMERA_H
#define CAMERABUFFER_H

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"


/** Contains the data for a single point of view. */
class Camera {
public:
	// Public Structs
	constexpr static float ConstNearPlane = 0.5f;
	struct GPUData {
		glm::mat4 pMatrix = glm::mat4(1.0f);
		glm::mat4 pMatrixInverse = glm::mat4(1.0f);
		glm::mat4 vMatrix = glm::mat4(1.0f);
		glm::mat4 vMatrixInverse = glm::mat4(1.0f);
		glm::mat4 pvMatrix = glm::mat4(1.0f);
		glm::vec3 EyePosition = glm::vec3(0.0f); float padding1 = 0;
		glm::vec2 Dimensions = glm::vec2(1.0f);
		float NearPlane = ConstNearPlane;
		float FarPlane = 1.0f;
		float FOV = 90.0f; glm::vec3 padding2 = glm::vec3(0, 0, 0);
	};


	// Public (De)Constructors
	/** Destroy the camera buffer. */
	inline ~Camera() = default;
	/** Construct a camera buffer. */
	inline Camera() = default;


	// Public Methods
	/** Set the enabled state for this camera.
	@param	enabled		whether this camera should be enabled or not. */
	inline void setEnabled(const bool& enabled) {
		m_enabled = enabled;
	}
	/** Retrieve if this camera is enabled or not.
	@return				true if enabled, false otherwise. */
	inline bool getEnabled() const {
		return m_enabled;
	}
	/** Recalculate frustum data for this camera. */
	inline void updateFrustum() {
		glm::vec4 posB = m_localData.vMatrixInverse * glm::vec4(0, 0, -m_localData.FarPlane / 2.0f, 1.0f);
		posB /= posB.w;
		m_frustumCenter = glm::vec3(posB) + m_localData.EyePosition;
	}
	/** Retrieve the center of this camera's frustum.
	@return				the center of this frustum. */
	inline glm::vec3 getFrustumCenter() const {
		return m_frustumCenter;
	}
	/** Retrieve a const pointer to the underlying data structure. */
	inline const GPUData* operator-> () const {
		return &m_localData;
	}
	/** Retrieve a pointer to the underlying data structure. */
	inline GPUData* operator-> () {
		return &m_localData;
	}
	/** Retrieve a const pointer to the underlying data structure. */
	inline const GPUData* get() const {
		return &m_localData;
	}
	/** Retrieve a pointer to the underlying data structure. */
	inline GPUData* get() {
		return &m_localData;
	}


	// Public Attributes
	glm::vec3 m_frustumCenter = glm::vec3(0);


private:
	// Private Attributes
	bool m_enabled = false;
	GPUData m_localData;
};

#endif // CAMERA_H