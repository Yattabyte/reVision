#include "Modules/Graphics/Common/Camera.h"


void Camera::setEnabled(const bool& enabled) noexcept
{
	m_enabled = enabled;
}

bool Camera::getEnabled() const noexcept
{
	return m_enabled;
}

void Camera::updateFrustum()
{
	glm::vec4 posB = m_localData.vMatrixInverse * glm::vec4(0, 0, -m_localData.FarPlane / 2.0F, 1.0F);
	posB /= posB.w;
	m_frustumCenter = glm::vec3(posB) + m_localData.EyePosition;
}

glm::vec3 Camera::getFrustumCenter() const noexcept
{
	return m_frustumCenter;
}

const Camera::GPUData* Camera::operator-> () const noexcept
{
	return &m_localData;
}

Camera::GPUData* Camera::operator-> () noexcept
{
	return &m_localData;
}

const Camera::GPUData* Camera::get() const noexcept
{
	return &m_localData;
}

Camera::GPUData* Camera::get() noexcept
{
	return &m_localData;
}