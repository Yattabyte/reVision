#pragma once
#ifndef CAMERA_H
#define CAMERABUFFER_H

#include <glad/glad.h>
#include "glm/glm.hpp"


/** Contains the data for a single point of view. */
class Camera {
public:
	// Public Structs
	constexpr static float ConstNearPlane = 0.5f;
	/** OpenGL-formatted data structure for camera data. */
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


	// Public Methods
	/** Set the enabled state for this camera.
	@param	enabled		whether this camera should be enabled or not. */
	void setEnabled(const bool& enabled) noexcept;
	/** Retrieve if this camera is enabled or not.
	@return				true if enabled, false otherwise. */
	bool getEnabled() const noexcept;
	/** Recalculate frustum data for this camera. */
	void updateFrustum();
	/** Retrieve the center of this camera's frustum.
	@return				the center of this frustum. */
	glm::vec3 getFrustumCenter() const noexcept;
	/** Retrieve a const pointer to the underlying data structure.
	@return				const pointer to camera data. */
	const GPUData* operator-> () const noexcept;
	/** Retrieve a pointer to the underlying data structure.
	@return				pointer to camera data. */
	GPUData* operator-> () noexcept;
	/** Retrieve a const pointer to the underlying data structure.
	@return				const pointer to camera data. */
	const GPUData* get() const noexcept;
	/** Retrieve a pointer to the underlying data structure.
	@return				pointer to camera data. */
	GPUData* get() noexcept;


	// Public Attributes
	glm::vec3 m_frustumCenter = glm::vec3(0);


private:
	// Private Attributes
	bool m_enabled = false;
	GPUData m_localData;
};

#endif // CAMERA_H