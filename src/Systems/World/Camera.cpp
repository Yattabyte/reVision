#include "Systems\World\Camera.h"
#include "glm\mat4x4.hpp"
#include "glm\gtc\matrix_transform.hpp"


Camera::~Camera()
{
}

Camera::Camera(const vec3 & position, const vec2 & size, const float & near_plane, const float & far_plane, const float & horizontal_FOV)
{
	setPosition(position);
	setDimensions(size);
	setNearPlane(near_plane);
	setFarPlane(far_plane);
	setHorizontalFOV(horizontal_FOV);
	setOrientation(quat(1, 0, 0, 0));
	enableRendering(true);
	m_buffer = StaticBuffer(sizeof(Camera_Buffer), &m_cameraBuffer);
	m_buffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);
	update();
}

Camera::Camera(Camera const & other)
{
	shared_lock<shared_mutex> rguard(other.data_mutex);
	m_cameraBuffer = other.getCameraBuffer();
	m_buffer = StaticBuffer(sizeof(Camera_Buffer), &m_cameraBuffer);
	m_buffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);
	m_frustum = Frustum(other.getFrustum());
	update();
}

void Camera::operator=(Camera const & other)
{
	shared_lock<shared_mutex> rguard(other.data_mutex);
	unique_lock<shared_mutex> wguard(data_mutex);
	m_cameraBuffer = other.getCameraBuffer();
	m_frustum = Frustum(other.getFrustum());
	data_mutex.unlock();
	update();
}

void Camera::setMatrices(const mat4 & pMatrix, const mat4 & vMatrix)
{
	unique_lock<shared_mutex> wguard(data_mutex);
	m_cameraBuffer.pMatrix = pMatrix;
	m_cameraBuffer.vMatrix = vMatrix;
	m_cameraBuffer.pMatrix_Inverse = glm::inverse(pMatrix);
	m_cameraBuffer.vMatrix_Inverse = glm::inverse(vMatrix);
	m_frustum.setFrustum(pMatrix * vMatrix);

	// Send data to GPU
	m_buffer.write_immediate(0, sizeof(Camera_Buffer), &m_cameraBuffer);
}

void Camera::setVisibilityToken(const Visibility_Token & vis_token)
{
	unique_lock<shared_mutex> wguard(data_mutex);
	m_vistoken = vis_token;
}

void Camera::update()
{
	unique_lock<shared_mutex> wguard(data_mutex);

	// Update Perspective Matrix
	float ar(m_cameraBuffer.Dimensions.x / m_cameraBuffer.Dimensions.y);
	float verticalFOV = 2.0f * atanf(tanf(radians(m_cameraBuffer.FOV) / 2.0f) / ar);
	m_cameraBuffer.pMatrix = perspective(verticalFOV, ar, m_cameraBuffer.NearPlane, m_cameraBuffer.FarPlane);
	m_cameraBuffer.pMatrix_Inverse = glm::inverse(m_cameraBuffer.pMatrix);

	// Update Viewing Matrix
	m_cameraBuffer.vMatrix = glm::mat4_cast(m_orientation) * translate(mat4(1.0f), -m_cameraBuffer.EyePosition);
	m_cameraBuffer.vMatrix_Inverse = glm::inverse(m_cameraBuffer.vMatrix);

	m_frustum.setFrustum(m_cameraBuffer.pMatrix * m_cameraBuffer.vMatrix);

	// Send data to GPU
	m_buffer.write_immediate(0, sizeof(Camera_Buffer), &m_cameraBuffer);
}

void Camera::Bind()
{
	m_buffer.bindBufferBase(GL_SHADER_STORAGE_BUFFER, 1);
}
