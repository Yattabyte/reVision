#include "Modules/Graphics/Common/Viewport.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Engine.h"
#include <algorithm>


Viewport::~Viewport()
{
}

Viewport::Viewport(Engine * engine, const glm::ivec2 & screenPosition, const glm::ivec2 & dimensions, const CameraBuffer::BufferStructure & cameraData)
	: m_screenPosition(screenPosition), m_dimensions(dimensions)
{
	// Initialize Camera
	m_cameraBuffer = std::make_shared<CameraBuffer>();
	m_cameraBuffer->replace(cameraData);
	genPerspectiveMatrix();

	// Initialize FBO's
	m_gfxFBOS = std::make_shared<Graphics_Framebuffers>(dimensions);
	glNamedFramebufferTexture(m_gfxFBOS->getFboID("LIGHTING"), GL_DEPTH_STENCIL_ATTACHMENT, m_gfxFBOS->getTexID("GEOMETRY", 3), 0);
	glNamedFramebufferTexture(m_gfxFBOS->getFboID("REFLECTION"), GL_DEPTH_STENCIL_ATTACHMENT, m_gfxFBOS->getTexID("GEOMETRY", 3), 0);

	// Initialize radiance hints volume
	m_rhVolume = std::make_shared<RH_Volume>(engine);
}

void Viewport::resize(const glm::ivec2 & size)
{
	if (m_dimensions != size) {
		m_dimensions = size;
		(*m_cameraBuffer)->Dimensions = size;
		m_gfxFBOS->resize(size);
		genPerspectiveMatrix();
	}
}

void Viewport::setDrawDistance(const float & drawDistance)
{
	if ((*m_cameraBuffer)->FarPlane != drawDistance) {
		(*m_cameraBuffer)->FarPlane = drawDistance;
		genPerspectiveMatrix();
	}
}

float Viewport::getDrawDistance() const
{
	return (*m_cameraBuffer)->FarPlane;
}

void Viewport::setFOV(const float & fov)
{
	if ((*m_cameraBuffer)->FOV != fov) {
		(*m_cameraBuffer)->FOV = fov;
		genPerspectiveMatrix();
	}
}

float Viewport::getFOV() const
{
	return (*m_cameraBuffer)->FOV;
}

void Viewport::set3DPosition(const glm::vec3 & position)
{
	(*m_cameraBuffer)->EyePosition = position;
}

glm::vec3 Viewport::get3DPosition() const
{
	return (*m_cameraBuffer)->EyePosition;
}

void Viewport::setViewMatrix(const glm::mat4 & vMatrix)
{
	(*m_cameraBuffer)->vMatrix = vMatrix;
}

glm::mat4 Viewport::getViewMatrix() const
{
	return (*m_cameraBuffer)->vMatrix;
}

void Viewport::genPerspectiveMatrix()
{
	// Update Perspective Matrix
	const float ar = std::max(1.0f, (*m_cameraBuffer)->Dimensions.x) / std::max(1.0f, (*m_cameraBuffer)->Dimensions.y);
	const float horizontalRad = glm::radians((*m_cameraBuffer)->FOV);
	const float verticalRad = 2.0f * atanf(tanf(horizontalRad / 2.0f) / ar);
	(*m_cameraBuffer)->pMatrix = glm::perspective(verticalRad, ar, CameraBuffer::BufferStructure::ConstNearPlane, (*m_cameraBuffer)->FarPlane);
}

glm::mat4 Viewport::getPerspectiveMatrix() const
{
	return (*m_cameraBuffer)->pMatrix;
}

void Viewport::bind()
{
	glViewport(m_screenPosition.x, m_screenPosition.y, m_dimensions.x, m_dimensions.y);
	m_rhVolume->updateVolume(m_cameraBuffer);
	m_cameraBuffer->bind(2);
}

void Viewport::clear()
{
	m_gfxFBOS->clear();
	m_rhVolume->clear();
}
