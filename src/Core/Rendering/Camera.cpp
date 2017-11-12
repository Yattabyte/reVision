#include "Rendering\Camera.h"
#include "glm\mat4x4.hpp"
#include "glm\gtc\matrix_transform.hpp"

Camera::~Camera()
{
	glDeleteBuffers(1, &ssboCameraID);
}

Camera::Camera(const vec3 &position, const vec2 &size, const float &near_plane, const float &far_plane, const float &horizontal_FOV)
{
	setPosition(position);
	setDimensions(size);
	setNearPlane(near_plane);
	setFarPlane(far_plane);
	setHorizontalFOV(horizontal_FOV);
	setOrientation(quat(1, 0, 0, 0));
	ssboCameraID = 0;
	glGenBuffers(1, &ssboCameraID);
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera_Buffer), &m_cameraBuffer, GL_DYNAMIC_COPY);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ssboCameraID, 0, sizeof(Camera_Buffer));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	Update();
}

Camera::Camera(Camera const & other)
{
	m_cameraBuffer = other.getCameraBuffer();
	ssboCameraID = 0;
	glGenBuffers(1, &ssboCameraID);
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera_Buffer), &m_cameraBuffer, GL_DYNAMIC_COPY);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ssboCameraID, 0, sizeof(Camera_Buffer));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	Update();
}

void Camera::operator=(Camera const & other)
{
	m_cameraBuffer = other.getCameraBuffer();
	Update();
}

Camera_Buffer Camera::getCameraBuffer() const
{
	return m_cameraBuffer;
}

void Camera::Update()
{
	// Update Perspective Matrix
	float ar(m_cameraBuffer.Dimensions.x / m_cameraBuffer.Dimensions.y);
	float verticalFOV = 2.0f * atanf(tanf(radians(m_cameraBuffer.FOV) / 2.0f) / ar);
	m_cameraBuffer.pMatrix = perspective(verticalFOV, ar, m_cameraBuffer.NearPlane, m_cameraBuffer.FarPlane);
	m_cameraBuffer.pMatrix_Inverse = glm::inverse(m_cameraBuffer.pMatrix);

	// Update Viewing Matrix
	m_cameraBuffer.vMatrix = glm::mat4_cast(m_orientation) * translate(mat4(1.0f), -m_cameraBuffer.EyePosition);
	m_cameraBuffer.vMatrix_Inverse = glm::inverse(m_cameraBuffer.vMatrix);

	// Send data to GPU
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Camera_Buffer), &m_cameraBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::Bind()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboCameraID);
}
