#include "Modules\Graphics\Resources\Common\RH_Volume.h"
#include "Engine.h"
#include <algorithm>


RH_Volume::~RH_Volume() {
	// Preference Callbacks
	m_engine->removePrefCallback(PreferenceState::C_DRAW_DISTANCE, this);
	m_engine->removePrefCallback(PreferenceState::C_RH_BOUNCE_SIZE, this);
}

RH_Volume::RH_Volume(Engine * engine) : m_engine(engine) {
	// Preference Callbacks
	m_farPlane = m_engine->addPrefCallback<float>(PreferenceState::C_DRAW_DISTANCE, this, [&](const float &f) { m_farPlane = f; });
	m_resolution = m_engine->addPrefCallback<float>(PreferenceState::C_RH_BOUNCE_SIZE, this, [&](const float &f) { m_resolution = f; });
}

void RH_Volume::updateVolume(const VB_Element<Camera_Buffer>& cameraBuffer) {
	const glm::mat4 InverseView = glm::inverse(cameraBuffer.data->vMatrix);
	const glm::vec2 ViewDimensions = cameraBuffer.data->Dimensions;
	const float AspectRatio = ViewDimensions.x / ViewDimensions.y;
	const float tanHalfHFOV = glm::radians(cameraBuffer.data->FOV) / 2.0f;
	const float tanHalfVFOV = atanf(tanf(tanHalfHFOV) / AspectRatio);
	const float frustumSlice[2] = { m_nearPlane, (m_farPlane * 0.25f) };
	const float frustumPoints[4] = {
		frustumSlice[0] * tanHalfHFOV,
		frustumSlice[1] * tanHalfHFOV,
		frustumSlice[0] * tanHalfVFOV,
		frustumSlice[1] * tanHalfVFOV
	};
	float largestCoordinate = std::max(abs(frustumSlice[0]), abs(frustumSlice[1]));
	for (int x = 0; x < 4; ++x)
		largestCoordinate = std::max(largestCoordinate, abs(frustumPoints[x]));
	const glm::vec3 centerOfVolume(0, 0, ((frustumSlice[1] - frustumSlice[0]) / 2.0f) + frustumSlice[0]);
	const float radius = glm::distance(glm::vec3(largestCoordinate), centerOfVolume);
	const glm::vec3 aabb(radius);
	m_unitSize = (radius - -radius) / m_resolution;
	const glm::vec3 frustumpos = (InverseView * glm::vec4(centerOfVolume, 1.0f));
	// Snap volume position to grid
	m_center = glm::floor((frustumpos + (m_unitSize / 2.0f)) / m_unitSize) * m_unitSize;
	m_min = -aabb + m_center;
	m_max = aabb + m_center;
}