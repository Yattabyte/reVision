#include "Rendering\Camera.h"
#include "Managers\Visibility_Manager.h"
#include "glm\mat4x4.hpp"
#include "glm\gtc\matrix_transform.hpp"

Camera::~Camera()
{
	glDeleteBuffers(1, &ssboCameraID);
	Visibility_Manager::UnRegisterViewer(this);
}

Camera::Camera(const vec3 &position, const vec2 &size, const float &near_plane, const float &far_plane, const float &horizontal_FOV)
{
	setPosition(position);
	setDimensions(size);
	setNearPlane(near_plane);
	setFarPlane(far_plane);
	setHorizontalFOV(horizontal_FOV);
	setOrientation(quat(1, 0, 0, 0));
	enableRendering(true);
	ssboCameraID = 0;
	glGenBuffers(1, &ssboCameraID);
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera_Buffer), &m_cameraBuffer, GL_DYNAMIC_COPY);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ssboCameraID, 0, sizeof(Camera_Buffer));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	Update();

	Visibility_Manager::RegisterViewer(this);
}

Camera::Camera(Camera const & other)
{
	shared_lock<shared_mutex> rguard(other.data_mutex);
	m_cameraBuffer = other.getCameraBuffer();
	ssboCameraID = 0;
	glGenBuffers(1, &ssboCameraID);
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera_Buffer), &m_cameraBuffer, GL_DYNAMIC_COPY);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ssboCameraID, 0, sizeof(Camera_Buffer));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	m_frustum = Frustum(other.getFrustum());
	Update();

	Visibility_Manager::RegisterViewer(this);
}

void Camera::operator=(Camera const & other)
{
	shared_lock<shared_mutex> rguard(other.data_mutex);
	lock_guard<shared_mutex> wguard(data_mutex);
	m_cameraBuffer = other.getCameraBuffer();
	m_frustum = Frustum(other.getFrustum());
	data_mutex.unlock();
	Update();
}

void Camera::Update()
{
	shared_lock<shared_mutex> rguard(data_mutex);

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
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Camera_Buffer), &m_cameraBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::Bind()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboCameraID);
}
