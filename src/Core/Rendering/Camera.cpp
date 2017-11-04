#include "Rendering\Camera.h"
#include "glm\gtc\matrix_transform.hpp"

Camera::~Camera()
{
	glDeleteBuffers(1, &ssboCameraID);
}

Camera::Camera(const vec3 &position, const vec2 &size, const float &draw_distance, const float &horizontal_FOV)
{
	setPosition(position);
	setDrawDistance(draw_distance);
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
	ssboCameraBuffer.vMatrix = glm::translate(mat4(), p);
}

void Camera::setDimensions(const vec2 &d)
{
	ssboCameraBuffer.Dimensions = d;
}

void Camera::setDrawDistance(const float &d)
{
	ssboCameraBuffer.DrawDistance = d;
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
	float ar(ssboCameraBuffer.Dimensions.x/ ssboCameraBuffer.Dimensions.y);
	float verticalFOV = 2.0f * atanf(tanf(glm::radians(ssboCameraBuffer.FOV) / 2.0f) / ar);
	ssboCameraBuffer.pMatrix = glm::perspective(verticalFOV, ar, 0.01f, ssboCameraBuffer.DrawDistance);

	glBindBuffer(GL_UNIFORM_BUFFER, ssboCameraID);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Camera_Buffer), &ssboCameraBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}