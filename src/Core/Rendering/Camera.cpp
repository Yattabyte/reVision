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
	setNearPlane(near_plane);
	setFarPlane(far_plane);
	setDimensions(size);
	setHorizontalFOV(horizontal_FOV);
	ssboCameraID = 0;
	glGenBuffers(1, &ssboCameraID);
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera_Buffer), &ssboCameraBuffer, GL_DYNAMIC_COPY);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ssboCameraID, 0, sizeof(Camera_Buffer));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	Update();
}

Camera::Camera(Camera const & other)
{
	ssboCameraBuffer = other.getCameraBuffer();	
	ssboCameraID = 0;
	glGenBuffers(1, &ssboCameraID);
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Camera_Buffer), &ssboCameraBuffer, GL_DYNAMIC_COPY);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ssboCameraID, 0, sizeof(Camera_Buffer));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	Update();
}

void Camera::operator=(Camera const & other)
{
	ssboCameraBuffer = other.getCameraBuffer();
	Update();
}

void Camera::setPosition(const vec3 &p)
{
	ssboCameraBuffer.EyePosition = p;
}

void Camera::setDimensions(const vec2 &d)
{
	ssboCameraBuffer.Dimensions = d;
}

void Camera::setNearPlane(const float &d)
{
	ssboCameraBuffer.FarPlane = d;
}

void Camera::setFarPlane(const float &d)
{
	ssboCameraBuffer.FarPlane = d;
}

void Camera::setHorizontalFOV(const float &fov)
{
	ssboCameraBuffer.FOV = fov;
}

Camera_Buffer Camera::getCameraBuffer() const
{
	return ssboCameraBuffer;
}

void Camera::Update()
{
	// Update Perspective Matrix
	float ar(ssboCameraBuffer.Dimensions.x / ssboCameraBuffer.Dimensions.y);
	float verticalFOV = 2.0f * atanf(tanf(radians(ssboCameraBuffer.FOV) / 2.0f) / ar);
	ssboCameraBuffer.pMatrix = perspective(verticalFOV, ar, ssboCameraBuffer.NearPlane, ssboCameraBuffer.FarPlane);

	// Update Viewing Matrix
	ssboCameraBuffer.vMatrix = translate(mat4(1.0f), -ssboCameraBuffer.EyePosition);

	// Send data to GPU
	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Camera_Buffer), &ssboCameraBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Camera::Bind()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboCameraID);
}
