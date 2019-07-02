#include "Modules/Graphics/Common/Viewport.h"
#include "glm/gtc/matrix_transform.hpp"
#include "Engine.h"
#include <algorithm>


Viewport::~Viewport()
{
}

Viewport::Viewport(Engine * engine, const glm::ivec2 & screenPosition, const glm::ivec2 & dimensions)
	: m_screenPosition(screenPosition), m_dimensions(dimensions)
{
	// Initialize Camera

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
		m_gfxFBOS->resize(size);
	}
}

void Viewport::bind(const std::shared_ptr<CameraBuffer> & camera)
{
	glViewport(m_screenPosition.x, m_screenPosition.y, m_dimensions.x, m_dimensions.y);
	m_rhVolume->updateVolume(camera);
}

void Viewport::clear()
{
	m_gfxFBOS->clear();
	m_rhVolume->clear();
}
