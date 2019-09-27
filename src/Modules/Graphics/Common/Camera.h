#pragma once
#ifndef CAMERA_H
#define CAMERABUFFER_H

#include "Utilities/GL/glad/glad.h"
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


	// Public (de)Constructors
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
		constexpr static auto normalizePlane = [](glm::vec4& plane) {
			float magnitude = (float)sqrtf(plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2]);
			plane[0] /= magnitude;
			plane[1] /= magnitude;
			plane[2] /= magnitude;
			plane[3] /= magnitude;
		};
		const auto pMatrix = glm::value_ptr(m_localData.pMatrix);
		const auto vMatrix = glm::value_ptr(m_localData.vMatrix);
		float clip[16]; //clipping planes

		clip[0] = vMatrix[0] * pMatrix[0] + vMatrix[1] * pMatrix[4] + vMatrix[2] * pMatrix[8] + vMatrix[3] * pMatrix[12];
		clip[1] = vMatrix[0] * pMatrix[1] + vMatrix[1] * pMatrix[5] + vMatrix[2] * pMatrix[9] + vMatrix[3] * pMatrix[13];
		clip[2] = vMatrix[0] * pMatrix[2] + vMatrix[1] * pMatrix[6] + vMatrix[2] * pMatrix[10] + vMatrix[3] * pMatrix[14];
		clip[3] = vMatrix[0] * pMatrix[3] + vMatrix[1] * pMatrix[7] + vMatrix[2] * pMatrix[11] + vMatrix[3] * pMatrix[15];

		clip[4] = vMatrix[4] * pMatrix[0] + vMatrix[5] * pMatrix[4] + vMatrix[6] * pMatrix[8] + vMatrix[7] * pMatrix[12];
		clip[5] = vMatrix[4] * pMatrix[1] + vMatrix[5] * pMatrix[5] + vMatrix[6] * pMatrix[9] + vMatrix[7] * pMatrix[13];
		clip[6] = vMatrix[4] * pMatrix[2] + vMatrix[5] * pMatrix[6] + vMatrix[6] * pMatrix[10] + vMatrix[7] * pMatrix[14];
		clip[7] = vMatrix[4] * pMatrix[3] + vMatrix[5] * pMatrix[7] + vMatrix[6] * pMatrix[11] + vMatrix[7] * pMatrix[15];

		clip[8] = vMatrix[8] * pMatrix[0] + vMatrix[9] * pMatrix[4] + vMatrix[10] * pMatrix[8] + vMatrix[11] * pMatrix[12];
		clip[9] = vMatrix[8] * pMatrix[1] + vMatrix[9] * pMatrix[5] + vMatrix[10] * pMatrix[9] + vMatrix[11] * pMatrix[13];
		clip[10] = vMatrix[8] * pMatrix[2] + vMatrix[9] * pMatrix[6] + vMatrix[10] * pMatrix[10] + vMatrix[11] * pMatrix[14];
		clip[11] = vMatrix[8] * pMatrix[3] + vMatrix[9] * pMatrix[7] + vMatrix[10] * pMatrix[11] + vMatrix[11] * pMatrix[15];

		clip[12] = vMatrix[12] * pMatrix[0] + vMatrix[13] * pMatrix[4] + vMatrix[14] * pMatrix[8] + vMatrix[15] * pMatrix[12];
		clip[13] = vMatrix[12] * pMatrix[1] + vMatrix[13] * pMatrix[5] + vMatrix[14] * pMatrix[9] + vMatrix[15] * pMatrix[13];
		clip[14] = vMatrix[12] * pMatrix[2] + vMatrix[13] * pMatrix[6] + vMatrix[14] * pMatrix[10] + vMatrix[15] * pMatrix[14];
		clip[15] = vMatrix[12] * pMatrix[3] + vMatrix[13] * pMatrix[7] + vMatrix[14] * pMatrix[11] + vMatrix[15] * pMatrix[15];

		m_planes[0][0] = clip[3] - clip[0];
		m_planes[0][1] = clip[7] - clip[4];
		m_planes[0][2] = clip[11] - clip[8];
		m_planes[0][3] = clip[15] - clip[12];
		normalizePlane(m_planes[0]);

		m_planes[1][0] = clip[3] + clip[0];
		m_planes[1][1] = clip[7] + clip[4];
		m_planes[1][2] = clip[11] + clip[8];
		m_planes[1][3] = clip[15] + clip[12];
		normalizePlane(m_planes[1]);

		m_planes[2][0] = clip[3] + clip[1];
		m_planes[2][1] = clip[7] + clip[5];
		m_planes[2][2] = clip[11] + clip[9];
		m_planes[2][3] = clip[15] + clip[13];
		normalizePlane(m_planes[2]);

		m_planes[3][0] = clip[3] - clip[1];
		m_planes[3][1] = clip[7] - clip[5];
		m_planes[3][2] = clip[11] - clip[9];
		m_planes[3][3] = clip[15] - clip[13];
		normalizePlane(m_planes[3]);

		m_planes[4][0] = clip[3] - clip[2];
		m_planes[4][1] = clip[7] - clip[6];
		m_planes[4][2] = clip[11] - clip[10];
		m_planes[4][3] = clip[15] - clip[14];
		normalizePlane(m_planes[4]);

		m_planes[5][0] = clip[3] + clip[2];
		m_planes[5][1] = clip[7] + clip[6];
		m_planes[5][2] = clip[11] + clip[10];
		m_planes[5][3] = clip[15] + clip[14];
		normalizePlane(m_planes[5]);

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
	glm::vec4 m_planes[6] = { {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0} };


private:
	// Private Attributes
	bool m_enabled = false;
	GPUData m_localData;
};

#endif // CAMERA_H